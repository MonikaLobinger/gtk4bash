// wrap_add_signals von: https://github.com/abecadel/gtkwrap
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <adwaita.h>
#include <unistd.h>
#include <libintl.h>
#include <expat.h>

#define APP_ID "com.github.MonikaLobinger.gtk4bash"
#define STRING_SIZE 128
#define OBJECT_TAG "<object class=\""
#define SIGNAL_TAG "<signal name=\""
#define ID_TAG "\" id=\""
#define HANDLER_TAG "\" handler=\""
#define MAXCHARS 1000000
#define ACTIVATE_EMPTY_HANDLERS
#undef ACTIVATE_EMPTY_HANDLERS

short DEBUG   = 0;
short VERBOSE = 0;
short RUNNING = 0;

typedef struct {
    char       *app_name;
    char       *ui_file;
    char       *css_file;
    char       *win_id;
    GtkBuilder *builder;
    char*      *SIGNALS;
    char       *fpipeout;
    char       *fpipein;
    pthread_t   thread;
} _args;

static void appwin_activate_default(GtkApplicationWindow *appwin, _args *pargs);
static void appwin_activate_focus(GtkApplicationWindow *appwin, _args *pargs);
static gboolean appwin_close_request(GtkApplicationWindow *appwin, _args *pargs);
static gboolean appwin_enable_debugging(GtkApplicationWindow *appwin, gboolean toggle, _args *pargs);
static void appwin_destroy(GtkApplicationWindow *appwin, _args *pargs);
static void appwin_direction_changed(GtkApplicationWindow *appwin, GtkTextDirection previous_direction, _args *pargs);
static void appwin_hide(GtkApplicationWindow *appwin, _args *pargs);
static gboolean appwin_keynav_failed(GtkApplicationWindow *appwin, GtkDirectionType direction, _args *pargs);
static void appwin_map(GtkApplicationWindow *appwin, _args *pargs);
static gboolean appwin_mnemonic_activate(GtkApplicationWindow *appwin, gboolean group_cycling, _args *pargs);
static void appwin_move_focus(GtkApplicationWindow *appwin, GtkDirectionType direction, _args *pargs);
static gboolean appwin_query_tooltip(GtkApplicationWindow *appwin, gint x, gint y, gboolean keyboard_mode, GtkTooltip* tooltip,_args *pargs);
static void appwin_realize(GtkApplicationWindow *appwin, _args *pargs);
static void appwin_show(GtkApplicationWindow *appwin, _args *pargs);
static void appwin_state_flags_changed(GtkApplicationWindow *appwin, GtkStateFlags flags, _args *pargs);
static void appwin_unmap(GtkApplicationWindow *appwin, _args *pargs);
static void appwin_unrealize(GtkApplicationWindow *appwin, _args *pargs);
static void appwin_notify(GtkApplicationWindow *appwin, GParamSpec * pspec, _args *pargs);

void cbk_wrap_signal_handler(gpointer user_data, GObject *object) {
    char *signal = (char *)user_data;
    fprintf(stdout, "%s\n", signal);
    fflush(stdout);
  }
static void wrap_cleanup(_args* pargs) {
    if(DEBUG) fprintf(stderr, "START %s()...\n", __func__);
    RUNNING = 0;
    if(pargs->fpipeout) {
        void* res;
        if(DEBUG) fprintf(stderr, "!!!!VOR pthread_cancel ...\n");
        int s = pthread_cancel(pargs->thread);
        if(s != 0) fprintf(stderr, "!!!!ERROR pthread_cancel ...\n");

        s = pthread_join(pargs->thread, &res);
        if(s != 0) fprintf(stderr, "!!!!ERROR pthread_join ...\n");
        if(res == PTHREAD_CANCELED)
            if(DEBUG) fprintf(stderr, "%s(): thread was canceled\n", __func__);
        else
            fprintf(stderr, "!!!!ERROR %s(): thread wasn't canceled (shouldn't happen!)\n", __func__);
    }
    if(VERBOSE)
        fprintf(stderr, "Cleaning ...\n");
    g_object_unref(pargs->builder);
    char **tmp = pargs->SIGNALS;
    while(*tmp)
        free(*tmp++);
    free(pargs->SIGNALS);
    unlink(pargs->fpipeout);
    unlink(pargs->fpipein);
    pargs->SIGNALS=NULL;
    pargs->builder=NULL;
    pargs->fpipeout=NULL;
    pargs->fpipein=NULL;
    pargs->thread=0;
  }
void *wrap_reader_loop(void* user_data) {
    if(DEBUG) fprintf(stderr, "START %s()...\n", __func__);
    _args *pargs = (_args *)user_data;
    mkfifo(pargs->fpipeout, S_IRWXU);
    FILE *fileout = fopen(pargs->fpipeout, "a+");
    if(!fileout) {
        fprintf(stderr, "Error opening pipe %s !\n", pargs->fpipeout);
        pthread_exit(NULL);
    }


    mkfifo(pargs->fpipein, S_IRWXU);
    FILE *filein = fopen(pargs->fpipein, "r+");    
    if(!filein) {
        fprintf(stderr, "Error opening pipe %s !\n", pargs->fpipein);
        pthread_exit(NULL);
    }

    if(VERBOSE)
        fprintf(stderr, "Using pipes out:%s in:%s\n", pargs->fpipeout, pargs->fpipein);


    char input[1024]; 
    char strend = ' ';
    char *command = NULL;
    char *widget_id = NULL;
    char *operanda = NULL;
    char *operandb = NULL;
    void *vu;

    void trim(char *s) {
        size_t len = strspn(s, " ");
        if (len > 0) memmove(s, s + len, strlen(s) - len + 1);
    }

    typedef struct {char* name; void* fup; } FU;
    typedef struct {int size; FU* fus;} FUS;
    FUS* new_function_table() {
        FUS* t = malloc(sizeof(FUS));t->size = 0;t->fus = NULL;return t;
    }
    void free_function_table(FUS* t) {
        for(int i = 0; i < t->size; i++)free((t->fus + i)->name);
        free(t->fus); free(t);
    }
    void add_function(FUS* t, char* name, void* fup) {
        if(!t->size) t->fus=malloc(sizeof(FU));
        else t->fus=realloc(t->fus, sizeof(FU)*(t->size+1));
        (t->fus + t->size)->name = malloc(strlen(name) + 1);
        strcpy((t->fus + t->size)->name, name);
        (t->fus + t->size)->fup = fup;
        t->size++;
    }
    void* get_function(FUS* t, char* name) {
        for (int i = 0; i < t->size; i++)
            if (strcmp((t->fus + i)->name, name)==0)return(t->fus + i)->fup;
        return NULL;
    }
    FUS* table_constcharP=new_function_table();                      // const char* void
    typedef const char*(*sig_constcharP)();          
    FUS* table_GListModelP=new_function_table();                     // GListModel* void
    typedef GListModel*(*sig_GListModelP)();         
    FUS* table_GListP=new_function_table();                          // GList* void
    typedef GList*(*sig_GListP)();                   
    FUS* table_void_gboolean=new_function_table();                   // void gboolean
    typedef void(*sig_void_gboolean)(gboolean);      
    FUS* table_void_constcharP=new_function_table();                 // void const char * 
    typedef void(*sig_void_constcharP)(const char*); 
    FUS* table_void_GtkWindowP=new_function_table();                 // void GtkWindow*
    typedef void(*sig_void_GtkWindowP)(GtkWindow*);  
    FUS* table_void_GtkWindowP_GdkMonitorP=new_function_table();     // void GtkWindow* GdkMonitor*
    typedef void(*sig_void_GtkWindowP_GdkMonitorP)(GtkWindow*, GdkMonitor*);  
    FUS* table_GtkApplication_GtkWindowP=new_function_table();       // GtkApplication* GtkWindow*
    typedef GtkApplication*(*sig_GtkApplication_GtkWindowP)(GtkWindow*);  
    FUS* table_GtkWidgetP_GtkWindowP=new_function_table();           // GtkWidget* GtkWindow*
    typedef GtkWidget*(*sig_GtkWidgetP_GtkWindowP)(GtkWindow*);  
    FUS* table_gboolean_GtkWindowP=new_function_table();             // gboolean GtkWindow*
    typedef gboolean(*sig_gboolean_GtkWindowP)(GtkWindow*);  
    FUS* table_void_GtkWindowP_intP_intP=new_function_table();            // void GtkWindow* int* int*
    typedef void(*sig_void_GtkWindowP_intP_intP)(GtkWindow*, int*, int*);  
    #if GTK_MINOR_VERSION >= 20
      FUS* table_GtkWindowGravity_GtkWindowP=new_function_table();     // GtkWindowGravity GtkWindow*
      typedef GtkWindowGravity (*sig_GtkWindowGravity_GtkWindowP)(GtkWindow*);
    #endif
    FUS* table_GtkWindowGroupP_GtkWindowP=new_function_table();      // GtkWindowGroup* GtkWindow*
    typedef GtkWindowGroup (*sig_GtkWindowGroupP_GtkWindowP)(GtkWindow*);
    FUS* table_constcharP_GtkWindowP=new_function_table();           // const char* GtkWindow*
    typedef const char* (*sig_constcharP_GtkWindowP)(GtkWindow*);
    FUS* table_GtkWindowP_GtkWindowP=new_function_table();           // GtkWindow* GtkWindow*
    typedef GtkWindow* (*sig_GtkWindowP_GtkWindowP)(GtkWindow*);
    FUS* table_void_GtkWindowP_guint32=new_function_table();         // void GtkWindow* guint32
    typedef void (*sig_void_GtkWindowP_guint32)(GtkWindow*, guint32);
    FUS* table_void_GtkWindowP_GtkApplicationP=new_function_table(); // void GtkWindow* GtkApplication*
    typedef void (*sig_void_GtkWindowP_GtkApplicationP)(GtkWindow*, GtkApplication*);
    FUS* table_void_GtkWindowP_GtkWidgetP=new_function_table();      // void GtkWindow* GtkWidget*
    typedef void (*sig_void_GtkWindowP_GtkWidgetP)(GtkWindow*,GtkWidget);
    FUS* table_void_GtkWindowP_gboolean=new_function_table();        // void GtkWindow* gboolean
    typedef void (*sig_void_GtkWindowP_gboolean)(GtkWindow*, gboolean);
    FUS* table_void_GtkWindowP_int_int=new_function_table();         // void GtkWindow* int int
    typedef void (*sig_void_GtkWindowP_int_int)(GtkWindow*, int, int);
    FUS* table_void_GtkWindowP_GdkDisplayP=new_function_table();     // void GtkWindow* GdkDisplay*
    typedef void (*sig_void_GtkWindowP_GdkDisplayP)(GtkWindow*, GdkDisplay*);
    #if GTK_MINOR_VERSION >= 20
      FUS* table_void_GtkWindowP_GtkWindowGravity=new_function_table();// void GtkWindow* GtkWindowGravity
      typedef void (*sig_void_GtkWindowP_GtkWindowGravity)(GtkWindow*, GtkWindowGravity);
    #endif
    FUS* table_void_GtkWindowP_constcharP=new_function_table();      // void GtkWindow* const char*
    typedef void (*sig_void_GtkWindowP_constcharP)(GtkWindow*, const char *);
    FUS* table_void_GtkWindowP_GtkWindowP=new_function_table();      // void GtkWindow* GtkWindow*

    /* GtkWindow **********************************************************/
    /* Functions */
    add_function(table_constcharP,"gtk_window_get_default_icon_name",gtk_window_get_default_icon_name); // const char* void
    /*table_GListModelP*/ /*gtk_window_get_toplevels*/ // GListModel* void
    /*table_GListP*/ /*gtk_window_list_toplevels*/ // GList* void
    add_function(table_void_gboolean,"gtk_window_set_auto_startup_notification",gtk_window_set_auto_startup_notification); // void gboolean
    add_function(table_void_constcharP,"gtk_window_set_default_icon_name",gtk_window_set_default_icon_name); // void const char *
    add_function(table_void_gboolean,"gtk_window_set_interactive_debugging",gtk_window_set_interactive_debugging); // void gboolean
    /* Instance methods */
    add_function(table_void_GtkWindowP,"gtk_window_close",gtk_window_close); // void GtkWindow*
    add_function(table_void_GtkWindowP,"gtk_window_destroy", gtk_window_destroy); // void GtkWindow*
    add_function(table_void_GtkWindowP,"gtk_window_fullscreen", gtk_window_fullscreen); // void GtkWindow*
    /*table_void_GtkWindowP_GdkMonitorP*/ /*gtk_window_fullscreen_on_monitor*/ // void GtkWindow* GdkMonitor*
    /*table_GtkApplication_GtkWindowP*/ /*gtk_window_get_application*/ // GtkApplication* GtkWindow*
    /*table_GtkWidgetP_GtkWindowP*/ /*gtk_window_get_child*/ // GtkWidget* GtkWindow*
    add_function(table_gboolean_GtkWindowP,"gtk_window_get_decorated",gtk_window_get_decorated); // gboolean GtkWindow*
    add_function(table_void_GtkWindowP_intP_intP,"gtk_window_get_default_size",gtk_window_get_default_size); // void GtkWindow* int* int*
    /*table_GtkWidgetP_GtkWindowP*/ /*gtk_window_get_default_widget*/ // GtkWidget* GtkWindow*
    add_function(table_gboolean_GtkWindowP,"gtk_window_get_deletable",gtk_window_get_deletable); // gboolean GtkWindow*
    add_function(table_gboolean_GtkWindowP,"gtk_window_get_destroy_with_parent",gtk_window_get_destroy_with_parent); // gboolean GtkWindow*
    /*table_GtkWidgetP_GtkWindowP*/ /*gtk_window_get_focus*/ // GtkWidget* GtkWindow*
    add_function(table_gboolean_GtkWindowP,"gtk_window_get_focus_visible",gtk_window_get_focus_visible); // gboolean GtkWindow*
    #if GTK_MINOR_VERSION >= 20
      add_function(table_GtkWindowGravity_GtkWindowP,"gtk_window_get_gravity",gtk_window_get_gravity); // GtkWindowGravity GtkWindow*
    #endif
    /*table_GtkWindowGroupP_GtkWindowP*/ /*gtk_window_get_group*/ // GtkWindowGroup* GtkWindow*
    add_function(table_gboolean_GtkWindowP,"gtk_window_get_handle_menubar_accel",gtk_window_get_handle_menubar_accel); // gboolean GtkWindow*
    add_function(table_gboolean_GtkWindowP,"gtk_window_get_hide_on_close",gtk_window_get_hide_on_close); // gboolean GtkWindow*
    add_function(table_constcharP_GtkWindowP,"gtk_window_get_icon_name",gtk_window_get_icon_name); // const char* GtkWindow*
    add_function(table_gboolean_GtkWindowP,"gtk_window_get_mnemonics_visible",gtk_window_get_mnemonics_visible); // gboolean GtkWindow*
    add_function(table_gboolean_GtkWindowP,"gtk_window_get_modal",gtk_window_get_modal); // gboolean GtkWindow*
    add_function(table_gboolean_GtkWindowP,"gtk_window_get_resizable",gtk_window_get_resizable); // gboolean GtkWindow*
    add_function(table_constcharP_GtkWindowP,"gtk_window_get_title",gtk_window_get_title); // const char* GtkWindow*
    /*table_GtkWidgetP_GtkWindowP*/ /*gtk_window_get_titlebar*/ // GtkWidget* GtkWindow*
    /*table_GtkWindowP_GtkWindowP*/ /*gtk_window_get_transient_for*/ // GtkWindow* GtkWindow*
    add_function(table_gboolean_GtkWindowP,"gtk_window_has_group",gtk_window_has_group); // gboolean GtkWindow*
    add_function(table_gboolean_GtkWindowP,"gtk_window_is_active",gtk_window_is_active); // gboolean GtkWindow*
    add_function(table_gboolean_GtkWindowP,"gtk_window_is_fullscreen",gtk_window_is_fullscreen); // gboolean GtkWindow*
    add_function(table_gboolean_GtkWindowP,"gtk_window_is_maximized",gtk_window_is_maximized); // gboolean GtkWindow*
    #if GTK_MINOR_VERSION >= 12
      add_function(table_gboolean_GtkWindowP,"gtk_window_is_suspended",gtk_window_is_suspended); // gboolean GtkWindow*
    #endif
    add_function(table_void_GtkWindowP,"gtk_window_maximize", gtk_window_maximize); // void GtkWindow*
    add_function(table_void_GtkWindowP,"gtk_window_minimize", gtk_window_minimize); // void GtkWindow*
    add_function(table_void_GtkWindowP,"gtk_window_present", gtk_window_present); // void GtkWindow*
    #if GTK_MINOR_VERSION < 14
      add_function(table_void_GtkWindowP_guint32,"gtk_window_present_with_time",gtk_window_present_with_time); // void GtkWindow* guint32
    #endif
    /*table_void_GtkWindowP_GtkApplicationP*/ /*gtk_window_set_application*/ // void GtkWindow* GtkApplication*
    /*table_void_GtkWindowP_GtkWidgetP*/ /*gtk_window_set_child*/ // void GtkWindow* GtkWidget*
    add_function(table_void_GtkWindowP_gboolean,"gtk_window_set_decorated",gtk_window_set_decorated); // void GtkWindow* gboolean
    add_function(table_void_GtkWindowP_int_int,"gtk_window_set_default_size",gtk_window_set_default_size); // void GtkWindow* int int
    /*table_void_GtkWindowP_GtkWidgetP*/ /*gtk_window_set_default_widget*/ // void GtkWindow* GtkWidget*
    add_function(table_void_GtkWindowP_gboolean,"gtk_window_set_deletable",gtk_window_set_deletable); // void GtkWindow* gboolean
    add_function(table_void_GtkWindowP_gboolean,"gtk_window_set_destroy_with_parent",gtk_window_set_destroy_with_parent); // void GtkWindow* gboolean
    /*table_void_GtkWindowP_GdkDisplayP*/ /*gtk_window_set_display*/ // void GtkWindow* GdkDisplay*
    /*table_void_GtkWindowP_GtkWidgetP*/ /*gtk_window_set_focus*/ // void GtkWindow* GtkWidget*
    add_function(table_void_GtkWindowP_gboolean,"gtk_window_set_focus_visible",gtk_window_set_focus_visible); // void GtkWindow* gboolean
    #if GTK_MINOR_VERSION >= 20
      add_function(table_void_GtkWindowP_GtkWindowGravity,"gtk_window_set_gravity",gtk_window_set_gravity); // void GtkWindow* GtkWindowGravity
    #endif
    add_function(table_void_GtkWindowP_gboolean,"gtk_window_set_handle_menubar_accel",gtk_window_set_handle_menubar_accel); // void GtkWindow* gboolean
    add_function(table_void_GtkWindowP_gboolean,"gtk_window_set_hide_on_close",gtk_window_set_hide_on_close); // void GtkWindow* gboolean
    add_function(table_void_GtkWindowP_constcharP,"gtk_window_set_icon_name",gtk_window_set_icon_name); // void GtkWindow* const char*
    add_function(table_void_GtkWindowP_gboolean,"gtk_window_set_mnemonics_visible",gtk_window_set_mnemonics_visible); // void GtkWindow* gboolean
    add_function(table_void_GtkWindowP_gboolean,"gtk_window_set_modal",gtk_window_set_modal); // void GtkWindow* gboolean
    add_function(table_void_GtkWindowP_gboolean,"gtk_window_set_resizable",gtk_window_set_resizable); // void GtkWindow* gboolean
    add_function(table_void_GtkWindowP_constcharP,"gtk_window_set_startup_id",gtk_window_set_startup_id); // void GtkWindow* const char*
    add_function(table_void_GtkWindowP_constcharP,"gtk_window_set_title",gtk_window_set_title); // void GtkWindow* const char*
    /*table_void_GtkWindowP_GtkWidgetP*/ /*gtk_window_set_titlebar*/ // void GtkWindow* GtkWidget*
    /*table_void_GtkWindowP_GtkWindowP*/ /*gtk_window_set_transient_for*/ // void GtkWindow* GtkWindow*
    add_function(table_void_GtkWindowP,"gtk_window_unfullscreen", gtk_window_unfullscreen); // void GtkWindow*
    add_function(table_void_GtkWindowP,"gtk_window_unmaximize", gtk_window_unmaximize); // void GtkWindow*
    add_function(table_void_GtkWindowP,"gtk_window_unminimize", gtk_window_unminimize); // void GtkWindow*
    /* Virtual methods */
    // void activate_default (GtkWindow* window)
    // void activate_focus (GtkWindow* window)
    // gboolean close_request (GtkWindow* window)
    // gboolean enable_debugging (GtkWindow* window, gboolean toggle)
    // void keys_changed (GtkWindow* window)

    while(RUNNING) {
        fgets(input, 1024, filein);
        if(!RUNNING) break; 
        input[strlen(input)-1]='\0';
        command = input;
        if(command[0]=='|') {strend='|';command++;} else strend=' ';
        trim(command);
        widget_id = strchr(command, strend);
        if(widget_id != NULL) { *widget_id++ = '\0';trim(widget_id); operanda = strchr(widget_id, strend);
        } else { operanda = NULL; }
        if(operanda != NULL) {*operanda++ = '\0';trim(operanda); operandb = strchr(operanda, strend);
        } else { operandb = NULL; }
        if(operandb != NULL) {*operandb++ = '\0';trim(operandb);}
        if(get_function(table_void_gboolean, command) || get_function(table_void_constcharP, command)) {
            operanda = widget_id;
            widget_id = NULL;
        }
        if(VERBOSE) fprintf(stderr, "CALLBACK:> %s %s %s %s\n", command, widget_id == NULL ? "NULL" : widget_id, operanda == NULL ? "NULL" : operanda, operandb == NULL ? "NULL" : operandb);
        GtkWidget *widget = widget_id == NULL ? NULL : GTK_WIDGET(gtk_builder_get_object(pargs->builder, widget_id));

        if(NULL != (vu=get_function(table_constcharP,command))) {
            if(VERBOSE) fprintf(stderr,"CALLBACK01 %s",command);
            const char* value = ((sig_constcharP)vu)();
            if(VERBOSE) fprintf(stderr," %s\n",value);
            fprintf(fileout, "%s\n", value);
            fflush(fileout);
        } else
        if(NULL != (vu=get_function(table_GListModelP,command))) {} else
        if(NULL != (vu=get_function(table_GListP,command))) {} else
        if(NULL != (vu=get_function(table_void_gboolean,command))) {
            if(VERBOSE) fprintf(stderr,"CALLBACK02 %s, argument: %s\n",command,operanda);
            gboolean b = (int) strtol(operanda, (char **)NULL, 10) == 0 ? FALSE : TRUE;
            ((sig_void_gboolean)vu)(b);
        } else
        if(NULL != (vu=get_function(table_void_constcharP,command))) {
            if(VERBOSE) fprintf(stderr,"CALLBACK03 %s, argument: %s\n",command,operanda);
            ((sig_void_constcharP)vu)(operanda);
        } else
        if(NULL != (vu=get_function(table_void_GtkWindowP,command))) {
            if(VERBOSE) fprintf(stderr,"CALLBACK04 %s, widget_id: %s\n",command,widget_id);
            ((sig_void_GtkWindowP)vu)(GTK_WINDOW(widget));
        } else
        if(NULL != (vu=get_function(table_void_GtkWindowP_GdkMonitorP,command))) { } else
        if(NULL != (vu=get_function(table_GtkApplication_GtkWindowP,command))) { } else
        if(NULL != (vu=get_function(table_GtkWidgetP_GtkWindowP,command))) { } else
        if(NULL != (vu=get_function(table_gboolean_GtkWindowP,command))) {
            if(VERBOSE) fprintf(stderr,"CALLBACK05 %s, widget_id: %s\n",command,widget_id);
            gboolean b = ((sig_gboolean_GtkWindowP)vu)(GTK_WINDOW(widget));
            fprintf(fileout, "%i\n", b);
            fflush(fileout);
        } else
        if(NULL != (vu=get_function(table_void_GtkWindowP_intP_intP,command))) {
            if(VERBOSE) fprintf(stderr,"CALLBACK06 %s, widget_id: %s\n",command,widget_id);
            int x, y;
            ((sig_void_GtkWindowP_intP_intP)vu)(GTK_WINDOW(widget),&x, &y);
            fprintf(fileout, "%i\n", x);
            fprintf(fileout, "%i\n", y);
            fflush(fileout);
        } else
        #if GTK_MINOR_VERSION >= 20
          if(NULL != (vu=get_function(table_GtkWindowGravity_GtkWindowP,command))) {
            if(VERBOSE) fprintf(stderr,"CALLBACK07 %s, widget_id: %s\n",command,widget_id);
            GtkWindowGravity g = ((sig_GtkWindowGravity_GtkWindowP)vu)(GTK_WINDOW(widget));
            fprintf(fileout, "%i\n", g);
            fflush(fileout);
          } else
        #endif
        if(NULL != (vu=get_function(table_GtkWindowGroupP_GtkWindowP,command))) { } else
        if(NULL != (vu=get_function(table_constcharP_GtkWindowP,command))) {
            if(VERBOSE) fprintf(stderr,"CALLBACK08 %s, widget_id: %s\n",command,widget_id);
            const char*value = ((sig_constcharP_GtkWindowP)vu)(GTK_WINDOW(widget));
            fprintf(fileout, "%s\n", value);
            fflush(fileout);
        } else
        if(NULL != (vu=get_function(table_GtkWindowP_GtkWindowP,command))) { } else
        if(NULL != (vu=get_function(table_void_GtkWindowP_guint32,command))) {
            if(VERBOSE) fprintf(stderr,"CALLBACK09 %s, widget_id: %s, argument: %s\n",command,widget_id,operanda);
            guint32 i = (int) strtol(operanda, (char **)NULL, 10);
            ((sig_void_GtkWindowP_guint32)vu)(GTK_WINDOW(widget),i);
        } else
        if(NULL != (vu=get_function(table_void_GtkWindowP_GtkApplicationP,command))) { } else
        if(NULL != (vu=get_function(table_void_GtkWindowP_GtkWidgetP,command))) { } else
        if(NULL != (vu=get_function(table_void_GtkWindowP_gboolean,command))) {
            if(VERBOSE) fprintf(stderr,"CALLBACK10 %s, widget_id: %s, argument: %s\n",command,widget_id,operanda);
            gboolean b = (int) strtol(operanda, (char **)NULL, 10) == 0 ? FALSE : TRUE;
            ((sig_void_GtkWindowP_gboolean)vu)(GTK_WINDOW(widget),b);
        } else
        if(NULL != (vu=get_function(table_void_GtkWindowP_int_int,command))) {
            if(VERBOSE) fprintf(stderr,"CALLBACK11 %s, widget_id: %s, argument1: %s, argument2: %s\n",command,widget_id,operanda, operandb);
            int x = (int) strtol(operanda, (char **)NULL, 10);
            int y = (int) strtol(operandb, (char **)NULL, 10);
            ((sig_void_GtkWindowP_int_int)vu)(GTK_WINDOW(widget),x,y);
        } else
        if(NULL != (vu=get_function(table_void_GtkWindowP_GdkDisplayP,command))) { } else
        #if GTK_MINOR_VERSION >= 20
          if(NULL != (vu=get_function(table_void_GtkWindowP_GtkWindowGravity,command))) {
            if(VERBOSE) fprintf(stderr,"CALLBACK12 %s, widget_id: %s, argument: %s\n",command,widget_id,operanda);
            GtkWindowGravity g = (int) strtol(operanda, (char **)NULL, 10);
            ((sig_void_GtkWindowP_GtkWindowGravity)vu)(GTK_WINDOW(widget),g);
          } else
        #endif
        if(NULL != (vu=get_function(table_void_GtkWindowP_constcharP,command))) {
            if(VERBOSE) fprintf(stderr,"CALLBACK13 %s, widget_id: %s, argument: %s\n",command,widget_id,operanda);
            ((sig_void_GtkWindowP_constcharP)vu)(GTK_WINDOW(widget),operanda);
        } else
        if(NULL != (vu=get_function(table_void_GtkWindowP_GtkWindowP,command))) { } else
       
        //window show
        if(!strcmp(command, "show")) {
            gtk_widget_show(widget);
        } else
        
        //window hide
        if(!strcmp(command, "hide")) {
            gtk_widget_hide(widget);
        } else

        //textview set text
        if(!strcmp(command, "set_textview_text")) {
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget)), operanda, -1); 
        } else

        //textview get text 
        if(!strcmp(command, "get_textview_text")) {
            GtkTextIter a, b;
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget)); 
            gtk_text_buffer_get_iter_at_offset(buffer, &a, 0);
            gtk_text_buffer_get_iter_at_offset(buffer, &b, -1);
            gchar* mtext = gtk_text_buffer_get_text(buffer, &a, &b, FALSE);
            fprintf(fileout, "%s\n", mtext);  
            fflush(fileout);
        } else
        //

        //spinner activate/deactivate
        if(!strcmp(command, "spinner_start")) {
            gtk_spinner_start(GTK_SPINNER(widget)); 
        } else

        if(!strcmp(command, "spinner_stop")) {
            gtk_spinner_stop(GTK_SPINNER(widget)); 
        } else

        //label set/get
        if(!strcmp(command, "set_label_text")) {
            gtk_label_set_text(GTK_LABEL(widget), operanda);
        } else

        //set button label
        if(!strcmp(command, "set_button_label")) {
            gtk_button_set_label(GTK_BUTTON(widget), operanda);
        } else

        //entrytext set/get
        if(!strcmp(command, "get_entry_text")) {
            gchar* mtext = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, -1);
            fprintf(fileout, "%s\n", mtext);  
            fflush(fileout);
        } else
        
        //if(!strcmp(command, "set_entry_text")) {
        //    gtk_entry_set_text(GTK_ENTRY(widget), operanda);
        //} else


        //combobox add options, get/set selected 
        if(!strcmp(command, "set_combobox_items")) {
            //GtkTreeModel *tree_model;
            //gtk_combo_box_model_set(GTK_COMBO_BOX(widget), tree_model);

        } else
        if(!strcmp(command, "get_selected_combobox_item")) {
            fprintf(fileout, "%d\n", gtk_combo_box_get_active(GTK_COMBO_BOX(widget)));  
            fflush(fileout);
        } else

        //image set image TODO doesn't work
        if(!strcmp(command, "set_image")) {
            gtk_image_set_from_file(GTK_IMAGE(widget), operanda);
            gtk_widget_show(widget);
        } else

        //progressbar set, show/hide
        if(!strcmp(command, "set_progressbar")) {

        } else

        //togglebutton istoggled //toggle, check, radio button 
        if(!strcmp(command, "get_button_state")) {
            if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
                fprintf(fileout, "1\n");
            else
                fprintf(fileout, "0\n");
            fflush(fileout);
        } else

        if(!strcmp(command, "gtk_editable_get_text")) {
            const char* mtext=gtk_editable_get_text(GTK_EDITABLE(widget));
            fprintf(fileout, "%s\n", mtext);  
            fflush(fileout);
        }
        

    }

    free_function_table(table_constcharP);table_constcharP=NULL;
    free_function_table(table_GListModelP);table_GListModelP=NULL;
    free_function_table(table_GListP);table_GListP=NULL;
    free_function_table(table_void_gboolean);table_void_gboolean=NULL;
    free_function_table(table_void_constcharP);table_void_constcharP=NULL;
    free_function_table(table_void_GtkWindowP);table_void_GtkWindowP=NULL;
    free_function_table(table_void_GtkWindowP_GdkMonitorP);table_void_GtkWindowP_GdkMonitorP=NULL;
    free_function_table(table_GtkApplication_GtkWindowP);table_GtkApplication_GtkWindowP=NULL;
    free_function_table(table_GtkWidgetP_GtkWindowP);table_GtkWidgetP_GtkWindowP=NULL;
    free_function_table(table_gboolean_GtkWindowP);table_gboolean_GtkWindowP=NULL;
    free_function_table(table_void_GtkWindowP_intP_intP);table_void_GtkWindowP_intP_intP=NULL;
    #if GTK_MINOR_VERSION >= 20
      free_function_table(table_GtkWindowGravity_GtkWindowP);table_GtkWindowGravity_GtkWindowP=NULL;
    #endif
    free_function_table(table_GtkWindowGroupP_GtkWindowP);table_GtkWindowGroupP_GtkWindowP=NULL;
    free_function_table(table_constcharP_GtkWindowP);table_constcharP_GtkWindowP=NULL;
    free_function_table(table_GtkWindowP_GtkWindowP);table_GtkWindowP_GtkWindowP=NULL;
    free_function_table(table_void_GtkWindowP_guint32);table_void_GtkWindowP_guint32=NULL;
    free_function_table(table_void_GtkWindowP_GtkApplicationP);table_void_GtkWindowP_GtkApplicationP=NULL;
    free_function_table(table_void_GtkWindowP_GtkWidgetP);table_void_GtkWindowP_GtkWidgetP=NULL;
    free_function_table(table_void_GtkWindowP_gboolean);table_void_GtkWindowP_gboolean=NULL;
    free_function_table(table_void_GtkWindowP_int_int);table_void_GtkWindowP_int_int=NULL;
    free_function_table(table_void_GtkWindowP_GdkDisplayP);table_void_GtkWindowP_GdkDisplayP=NULL;
    #if GTK_MINOR_VERSION >= 20
      free_function_table(table_void_GtkWindowP_GtkWindowGravity);table_void_GtkWindowP_GtkWindowGravity=NULL;
    #endif
    free_function_table(table_void_GtkWindowP_constcharP);table_void_GtkWindowP_constcharP=NULL;
    free_function_table(table_void_GtkWindowP_GtkWindowP);table_void_GtkWindowP_GtkWindowP=NULL;
    

    fclose(filein);
    fflush(fileout);
    fclose(fileout);
    fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! %s() Ich dachte, da kommen wir nie hin \n", __func__);
    pthread_exit(NULL);
  }
void wrap_add_signals(char *filename, _args* pargs) {
    if(DEBUG) fprintf(stderr, "START %s()...\n", __func__);
    //Adding signals handled in glade file
    //TODO: make less dumb, replace by real xml parser

    FILE *file = fopen(filename, "r");
    char line[STRING_SIZE];
    char objclass[STRING_SIZE];
    char objname[STRING_SIZE];
    char signame[STRING_SIZE];
    char sighandler[STRING_SIZE];
    char *a;
    int hand_count = 0;
    
    pargs->SIGNALS = (char**)calloc(50, sizeof(char*));
    if(!pargs->SIGNALS) {
        if(VERBOSE) fprintf(stderr,  "Error allocating memory: pargs->SIGNALS!\n");
        return;
    }    
    if(!file) {
        if(VERBOSE) fprintf(stderr,  "Couldn't open file %s, no signals will be auto-handled!\n", filename);
        return;
    }
    while(!feof(file)) {
        fgets(line, STRING_SIZE, file);

        if((a = strstr(line, OBJECT_TAG)) != NULL) {
            a += strlen(OBJECT_TAG);  
            int i = 0;
            for (; i < STRING_SIZE - 1 && *a != '\"'; i++)
                objclass[i] = *a++;
            objclass[i] = '\0';
            a += strlen(ID_TAG);
            for ( i = 0; i < STRING_SIZE - 1 && *a != '\"'; i++)
                objname[i] = *a++;
            objname[i] = '\0';
            continue;        
        }            
        if ((a = strstr(line, SIGNAL_TAG)) != NULL) {
            a += strlen(SIGNAL_TAG);
            int i = 0;
            for (; i < STRING_SIZE - 1 && *a != '\"'; i++)
                signame[i] = *a++;
            signame[i] = '\0';
            a += strlen(HANDLER_TAG);
            for(i = 0; i < STRING_SIZE - 1 && *a != '\"'; i++)
                sighandler[i] = *a++;
            sighandler[i] = '\0';
            if(VERBOSE) fprintf(stderr,  "Found signal \"%s\", handled by \"%s\" in object \"%s\" (%s)\n", signame, sighandler, objname, objclass);
            pargs->SIGNALS[hand_count] = (char*)calloc(strlen(sighandler) + 1, sizeof(char));
            strncpy(pargs->SIGNALS[hand_count], sighandler, strlen(sighandler));
            GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object( pargs->builder, objname));
            g_signal_connect_swapped(widget, signame, G_CALLBACK(cbk_wrap_signal_handler), pargs->SIGNALS[hand_count]);
            hand_count++;
            continue;
        }
    }
    pargs->SIGNALS[hand_count] = NULL;

    fclose(file); 
  }
static void add_css(_args *pargs, GtkApplicationWindow *appwin) {
    void add_styles(_args* pargs) {
        typedef struct {
            GtkBuilder *builder;
            int         depth;
            const char *id;
            GtkWidget  *wid;
        } xml_args;
        xml_args args = {pargs->builder,0,NULL,NULL};
        void start(void *user_data, const char *ele, const char **attr) {
            xml_args *pargs = (xml_args*)user_data;
            int             i;
            if(0 == strncmp(ele,"object",6)) {
                pargs->id="";
                pargs->wid=NULL;
                for (i = 0; attr[i]; i += 2) {
                    if(0 == strcmp(attr[i],"id")) {
                        pargs->id=attr[i+1];
                        pargs->wid = GTK_WIDGET(gtk_builder_get_object(pargs->builder, pargs->id));
                        break;
                    }
                }
            } else if(0 == strncmp(ele,"style",5)) {
                ;
            } else if(0 == strncmp(ele,"class",5)) {
                if(pargs->wid != NULL)
                    gtk_widget_add_css_class(pargs->wid,attr[1]);
            }
            pargs->depth++;
          }
        void end(void *user_data, const char *ele) {
            xml_args *pargs = (xml_args*)user_data;
            pargs->depth--;
          }
        char           *filename;
        FILE           *fp;
        size_t          size;
        char           *filecontent;
        XML_Parser      parser;

        filename = pargs->ui_file;
        parser = XML_ParserCreate(NULL);
        if (parser == NULL) {
            fprintf(stderr, "Kein Parser; Css Klassen nicht angebunden\n");
        } else {
            XML_SetUserData(parser, &args);
            XML_SetElementHandler(parser, start, end);
            fp = fopen(filename, "r");
            filecontent = malloc(MAXCHARS);
            size = fread(filecontent, sizeof(char), MAXCHARS, fp);
            if (XML_Parse(parser, filecontent, strlen(filecontent), XML_TRUE) == XML_STATUS_ERROR)
                fprintf(stderr, "Fehler in %s; Css Klassen nicht angebunden\n", filename);
            fclose(fp);
            XML_ParserFree(parser);
        }
      }
    GdkDisplay *display;
    GtkCssProvider *provider;
    if(pargs->css_file == NULL || access(pargs->css_file, F_OK) != 0) 
        return;
    if(DEBUG) fprintf(stderr, "Adding css file %s ...\n", pargs->css_file);
    display = gdk_display_get_default ();
    provider = gtk_css_provider_new ();
    gtk_css_provider_load_from_path(provider, pargs->css_file);
    gtk_style_context_add_provider_for_display (display, 
                                GTK_STYLE_PROVIDER (provider), 
                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    add_styles(pargs);
    g_object_unref(provider);
  }

static void app_query_end(GtkApplication *app, gpointer *user_data) { 
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void app_window_added(GtkApplication *app, GtkWindow *win, gpointer *user_data) { 
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void app_window_removed(GtkApplication *app, GtkWindow *win, gpointer *user_data) { 
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void app_command_line(GtkApplication *app, GApplicationCommandLine *cmd_line, gpointer *user_data) { 
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void app_handle_local_options(GtkApplication *app, GVariantDict *options, gpointer *user_data) { 
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void app_name_lost(GtkApplication *app, gpointer *user_data) { 
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void app_notify(GtkApplication *app, GParamSpec *pspec, gpointer *user_data) { 
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void app_action_added(GtkApplication *app, gchar* action_name, gpointer *user_data) { 
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void app_action_enabled_changed(GtkApplication *app, gchar* action_name, gboolean enabled, gpointer *user_data) { 
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void app_action_removed(GtkApplication *app, gchar* action_name, gpointer *user_data) { 
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void app_action_state_changed(GtkApplication *app, gchar* action_name, GVariant* value, gpointer *user_data) { 
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void app_startup(GtkApplication *app, gpointer *user_data) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
    _args *pargs = (_args *)user_data;
    GtkWidget *win = NULL; // GtkDialog deprecated with GTK4.10, use GtkWindow

    adw_init();
    pargs->builder = gtk_builder_new();
    gtk_builder_add_from_file(pargs->builder, pargs->ui_file, NULL);
    win = GTK_WIDGET(gtk_builder_get_object(pargs->builder, pargs->win_id));
    gtk_window_set_application(GTK_WINDOW(win), GTK_APPLICATION(app));
    wrap_add_signals(pargs->ui_file, pargs);
    if(DEBUG) fprintf(stderr, "ENDE %s()...\n", __func__);
  }
static void app_do(GtkApplication *app, gpointer *user_data) {
    if(DEBUG) fprintf(stderr, "START %s()...\n", __func__);
    _args *pargs = (_args *)user_data;
    GtkWidget *win;
    win = GTK_WIDGET(gtk_builder_get_object(pargs->builder, pargs->win_id));
    gtk_window_present(GTK_WINDOW(win));
    RUNNING = 1;
    if(pargs->fpipeout && !pargs->thread)
        pthread_create(&(pargs->thread), NULL, wrap_reader_loop, pargs);
    add_css(pargs, GTK_APPLICATION_WINDOW(win));

  #ifdef ACTIVATE_EMPTY_HANDLERS
    // Empty handlers, perhabs to be used later
    g_signal_connect (appwin, "activate-default", G_CALLBACK (appwin_activate_default), pargs);
    g_signal_connect (appwin, "activate-focus", G_CALLBACK (appwin_activate_focus), pargs);
    g_signal_connect (appwin, "close-request", G_CALLBACK (appwin_close_request), pargs);
    g_signal_connect (appwin, "enable-debugging", G_CALLBACK (appwin_enable_debugging), pargs);
    g_signal_connect (appwin, "destroy", G_CALLBACK (appwin_destroy), pargs);
    g_signal_connect (appwin, "direction-changed", G_CALLBACK (appwin_direction_changed), pargs);
    g_signal_connect (appwin, "hide", G_CALLBACK (appwin_hide), pargs);
    g_signal_connect (appwin, "keynav-failed", G_CALLBACK (appwin_keynav_failed), pargs);
    g_signal_connect (appwin, "map", G_CALLBACK (appwin_map), pargs);
    g_signal_connect (appwin, "mnemonic-activate", G_CALLBACK (appwin_mnemonic_activate), pargs);
    g_signal_connect (appwin, "move-focus", G_CALLBACK (appwin_move_focus), pargs);
    g_signal_connect (appwin, "query-tooltip", G_CALLBACK (appwin_query_tooltip), pargs);
    g_signal_connect (appwin, "realize", G_CALLBACK (appwin_realize), pargs);
    g_signal_connect (appwin, "show", G_CALLBACK (appwin_show), pargs);
    g_signal_connect (appwin, "state-flags-changed", G_CALLBACK (appwin_state_flags_changed), pargs);
    g_signal_connect (appwin, "unmap", G_CALLBACK (appwin_unmap), pargs);
    g_signal_connect (appwin, "unrealize", G_CALLBACK (appwin_unrealize), pargs);
    g_signal_connect (appwin, "notify", G_CALLBACK (appwin_notify), pargs);
  #endif
    if(DEBUG) fprintf(stderr, "ENDE %s()...\n", __func__);
}
static void appwin_activate_default(GtkApplicationWindow *appwin, _args *pargs) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void appwin_activate_focus(GtkApplicationWindow *appwin, _args *pargs) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static gboolean appwin_close_request(GtkApplicationWindow *appwin, _args *pargs) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
    return FALSE;
  }
static gboolean appwin_enable_debugging(GtkApplicationWindow *appwin, gboolean toggle, _args *pargs) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void appwin_destroy(GtkApplicationWindow *appwin, _args *pargs) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void appwin_direction_changed(GtkApplicationWindow *appwin, GtkTextDirection previous_direction, _args *pargs) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void appwin_hide(GtkApplicationWindow *appwin, _args *pargs) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static gboolean appwin_keynav_failed(GtkApplicationWindow *appwin, GtkDirectionType direction, _args *pargs) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void appwin_map(GtkApplicationWindow *appwin, _args *pargs) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static gboolean appwin_mnemonic_activate(GtkApplicationWindow *appwin, gboolean group_cycling, _args *pargs) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void appwin_move_focus(GtkApplicationWindow *appwin, GtkDirectionType direction, _args *pargs) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static gboolean appwin_query_tooltip(GtkApplicationWindow *appwin, gint x, gint y, gboolean keyboard_mode, GtkTooltip* tooltip,_args *pargs) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void appwin_realize(GtkApplicationWindow *appwin, _args *pargs) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void appwin_show(GtkApplicationWindow *appwin, _args *pargs) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void appwin_state_flags_changed(GtkApplicationWindow *appwin, GtkStateFlags flags, _args *pargs) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void appwin_unmap(GtkApplicationWindow *appwin, _args *pargs) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void appwin_unrealize(GtkApplicationWindow *appwin, _args *pargs) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void appwin_notify(GtkApplicationWindow *appwin, GParamSpec * pspec, _args *pargs) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
  }
static void app_activate(GtkApplication *app, gpointer *user_data) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
    /* Dieser Funktion endet mit dem Anzeigen des Dialogs. 
     * Danach befinden wir uns in g_application_run */
    app_do(app, user_data);
  }
static void app_open(GtkApplication *app, GFile ** files, gint n_files, gchar *hint, gpointer *user_data) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
    /* Dieser Funktion endet mit dem Anzeigen des Dialogs. 
     * Danach befinden wir uns in g_application_run */
    app_do(app, user_data);
  }
static void app_shutdown(GtkApplication *app, gpointer *user_data) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
    RUNNING = 0;
  }
static void help(char *appname) {
    fprintf(stderr, "\
    Aufruf:\n\
    %s -f string [-m string][-o string][-i string][-d][-v]\n\
    -f DLGFILE\t\t Die Dialogdatei\n\
    -s CSSFILE\t\t Stil Datei. Vorgabe ist \"styles.css\".\n\
    -m OBJECTNAME \t OBJECTNAME fürs main window. Vorgabe ist \"window1\".\n\
    -o OUTPIPE\t\t z.B. \"/tmp/${0}.${$}.out\", dann auch -i nötig\n\
    -i INPIPE\t\t z.B. \"/tmp/${0}.${$}.in\", dann auch -o nötig\n\
    -d \t\t\t Debug Output.\n\
    -v \t\t\t Verbose.\n"
    , appname);
    
    exit(1);
  }
static void read_opts(_args *pargs, int *pargc, char*** pargv) {
    char **argv;
    int argc;
    int opt;
    int pos;

    argc = *pargc;
    argv = *pargv;

    while((opt = getopt(argc, argv, ":dvhf:s:m:o:i:")) != -1) { 
        switch(opt) { 
            case 'd': 
                DEBUG = 1;
                if(VERBOSE) fprintf(stderr, "-%c\n"); 
                break; 
            case 'v': 
                VERBOSE = 1;
                if(VERBOSE) fprintf(stderr, "-%c\n"); 
                break; 
            case 'h': 
                if(VERBOSE) fprintf(stderr, "-%c\n"); 
                help(pargs->app_name);
                break; 
            case 'f': 
                if(VERBOSE) fprintf(stderr, "-%c: %s\n", opt, optarg);
                if(optarg[0]=='-') fprintf(stderr, "ACHTUNG: -%c: %s\n", opt, optarg);
                pargs->ui_file  = optarg;
                break; 
            case 's': 
                if(VERBOSE) fprintf(stderr, "-%c: %s\n", opt, optarg);
                if(optarg[0]=='-') fprintf(stderr, "ACHTUNG: -%c: %s\n", opt, optarg);
                pargs->css_file  = optarg;
                break; 
            case 'm': 
                if(VERBOSE) fprintf(stderr, "-%c: %s\n", opt, optarg);
                if(optarg[0]=='-') fprintf(stderr, "ACHTUNG: -%c: %s\n", opt, optarg);
                pargs->win_id  = optarg;
                break; 
            case 'o': 
                if(VERBOSE) fprintf(stderr, "-%c: %s\n", opt, optarg);
                if(optarg[0]=='-') fprintf(stderr, "ACHTUNG: -%c: %s\n", opt, optarg);
                pargs->fpipeout  = optarg;
                break; 
            case 'i': 
                if(VERBOSE) fprintf(stderr, "-%c: %s\n", opt, optarg);
                if(optarg[0]=='-') fprintf(stderr, "ACHTUNG: -%c: %s\n", opt, optarg);
                pargs->fpipein  = optarg;
                break; 
            case ':': 
                if(VERBOSE) fprintf(stderr, "Die Option braucht einen Wert\n"); 
                break; 
            case '?': 
                if(VERBOSE) fprintf(stderr, "Option %c wird nicht unterstützt\n", optopt);
                break; 
        } 
    } 
    for(pos=1; optind < argc; optind++) {
        if(VERBOSE) fprintf(stderr, "argv[%i]=%s\n", pos,argv[optind]); 
        argv[pos++]=argv[optind];
    }  
    *pargc=pos;
  }
int main(int argc, char **argv) {
    int stat;
    int idx;
    GtkApplication *app;
    _args args;

    args.app_name  = argv[0];
    args.ui_file   = NULL;
    if (access("styles.css", F_OK) == 0) {    
        args.css_file  = "styles.css";
    } else {
        args.css_file  = NULL;
    }
    args.win_id    = "window1";
    args.builder   = NULL;
    args.SIGNALS   = NULL;
    args.fpipeout  = NULL;
    args.fpipein   = NULL;
    args.thread    = 0;

    read_opts(&args, &argc, &argv);
    if(DEBUG) fprintf(stderr, "MAIN:Optionen eingelesen ...\n");
    if(!args.ui_file) 
        help(args.app_name);
    if((args.fpipeout && !args.fpipein) || (args.fpipein && !args.fpipeout))
        help(args.app_name);
    if(VERBOSE) for(idx=0; idx < argc; idx++)fprintf(stderr, "%i: %s\n", idx, argv[idx]);
    if(VERBOSE) fprintf(stderr, "UI-Datei: %s; TOP-WINDOW: %s\n", args.ui_file, args.win_id);

    app = gtk_application_new(APP_ID, G_APPLICATION_HANDLES_OPEN);
    if(DEBUG) fprintf(stderr, "MAIN:Neue Applikation ...\n");
  #ifdef ACTIVATE_EMPTY_HANDLERS
    // Empty handlers, perhabs to be used later
    g_object_set(app, "register-session", TRUE, NULL);
    g_signal_connect(app, "query-end", G_CALLBACK (app_query_end), &args);
    g_signal_connect(app, "window-added", G_CALLBACK (app_window_added), &args);
    g_signal_connect(app, "window-removed", G_CALLBACK (app_window_removed), &args);
    g_signal_connect(app, "command-line", G_CALLBACK (app_command_line), &args);
    //g_signal_connect(app, "handle-local-options", G_CALLBACK (app_handle_local_options), &args);
    g_signal_connect(app, "name-lost", G_CALLBACK (app_name_lost), &args);
    g_signal_connect(app, "notify", G_CALLBACK (app_notify), &args);
    g_signal_connect(app, "action-added", G_CALLBACK (app_action_added), &args);
    g_signal_connect(app, "action-enabled-changed", G_CALLBACK (app_action_enabled_changed), &args);
    g_signal_connect(app, "action-removed", G_CALLBACK (app_action_removed), &args);
    g_signal_connect(app, "action-state-changed", G_CALLBACK (app_action_state_changed), &args);
  #endif
    // Used handlers
    g_signal_connect(app, "startup", G_CALLBACK (app_startup), &args);
    g_signal_connect(app, "activate", G_CALLBACK (app_activate), &args);
    g_signal_connect(app, "open", G_CALLBACK (app_open), &args);
    g_signal_connect(app, "shutdown", G_CALLBACK (app_shutdown), &args);
    if(DEBUG) fprintf(stderr, "MAIN:Alle Application Handler verbunden ...\n");
    stat = g_application_run(G_APPLICATION(app), argc, argv);
    if(DEBUG) fprintf(stderr, "ENDE App läuft nicht mehr ...\n");
    wrap_cleanup(&args);
    g_object_unref(app);
    return stat;
}

