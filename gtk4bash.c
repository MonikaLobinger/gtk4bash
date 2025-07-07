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

static void before_destroy (GtkWidget *, GtkCssProvider *);

#define APP_ID "com.github.MonikaLobinger.gtk4bash"
#define STRING_SIZE 128
#define OBJECT_TAG "<object class=\""
#define SIGNAL_TAG "<signal name=\""
#define ID_TAG "\" id=\""
#define HANDLER_TAG "\" handler=\""
#define MAXCHARS 1000000

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
    int         depth;
    const char *id;
    GtkWidget  *wid;
} _args;

void cbk_wrap_signal_handler(gpointer user_data, GObject *object){
    char *signal = (char *)user_data;
    fprintf(stdout, "%s\n", signal);
    fflush(stdout);
  }
static void wrap_cleanup(_args* pargs) {
    if(DEBUG) fprintf(stderr, "START wrap_cleanup...\n");
    RUNNING = 0;
    if(pargs->fpipeout)
        pthread_cancel(pargs->thread);
    if(VERBOSE)
        fprintf(stderr, "Cleaning...\n");
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
void *wrap_reader_loop(void* user_data){
    if(DEBUG) fprintf(stderr, "START wrap_reader_loop...\n");
    _args *pargs = (_args *)user_data;
    mkfifo(pargs->fpipeout, S_IRWXU);
    FILE *fileout = fopen(pargs->fpipeout, "a+");
    if(!fileout){
        fprintf(stderr, "Error opening pipe %s !\n", pargs->fpipeout);
        pthread_exit(NULL);
    }


    mkfifo(pargs->fpipein, S_IRWXU);
    FILE *filein = fopen(pargs->fpipein, "r+");    
    if(!filein){
        fprintf(stderr, "Error opening pipe %s !\n", pargs->fpipein);
        pthread_exit(NULL);
    }

    if(VERBOSE)
        fprintf(stderr, "Using pipes out:%s in:%s\n", pargs->fpipeout, pargs->fpipein);


    char input[1024]; 
    char *operanda = NULL;
    char *widget_id = NULL;
    char *command = NULL;


    while(RUNNING){
        fgets(input, 1024, filein);

        if(!RUNNING)
            break; 

        widget_id = input;
        command = input;
        operanda = input;

        while(*command && *command != ' ')
            command++;

        *command = '\0';
        operanda = ++command;

        while(*operanda && *operanda != ' ')
            operanda++;
        *operanda++ = '\0';

        if(VERBOSE)
            fprintf(stderr, "Command:> %s %s %s\n", widget_id, command, operanda);
  
        GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(pargs->builder, widget_id));
        
 
        //window set title
        if(!strcmp(command, "set_window_title")){
            gtk_window_set_title(GTK_WINDOW(widget), operanda);
        } else
        
        //window show
        if(!strcmp(command, "show")){
            gtk_widget_show(widget);
        } else
        
        //window hide
        if(!strcmp(command, "hide")){
            gtk_widget_hide(widget);
        } else

        //textview set text
        if(!strcmp(command, "set_textview_text")){
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget)), operanda, -1); 
        } else

        //textview get text 
        if(!strcmp(command, "get_textview_text")){
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
        if(!strcmp(command, "spinner_start")){
            gtk_spinner_start(GTK_SPINNER(widget)); 
        } else

        if(!strcmp(command, "spinner_stop")){
            gtk_spinner_stop(GTK_SPINNER(widget)); 
        } else

        //label set/get
        if(!strcmp(command, "set_label_text")){
            gtk_label_set_text(GTK_LABEL(widget), operanda);
        } else

        //set button label
        if(!strcmp(command, "set_button_label")){
            gtk_button_set_label(GTK_BUTTON(widget), operanda);
        } else

        //entrytext set/get
        if(!strcmp(command, "get_entry_text")){
            gchar* mtext = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, -1);
            fprintf(fileout, "%s\n", mtext);  
            fflush(fileout);
        } else
        
        //if(!strcmp(command, "set_entry_text")){
        //    gtk_entry_set_text(GTK_ENTRY(widget), operanda);
        //} else


        //combobox add options, get/set selected 
        if(!strcmp(command, "set_combobox_items")){
            //GtkTreeModel *tree_model;
            //gtk_combo_box_model_set(GTK_COMBO_BOX(widget), tree_model);

        } else
        if(!strcmp(command, "get_selected_combobox_item")){
            fprintf(fileout, "%d\n", gtk_combo_box_get_active(GTK_COMBO_BOX(widget)));  
            fflush(fileout);
        } else

        //image set image TODO doesn't work
        if(!strcmp(command, "set_image")){
            gtk_image_set_from_file(GTK_IMAGE(widget), operanda);
            gtk_widget_show(widget);
        } else

        //progressbar set, show/hide
        if(!strcmp(command, "set_progressbar")){

        } else

        //togglebutton istoggled //toggle, check, radio button 
        if(!strcmp(command, "get_button_state")){
            if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
                fprintf(fileout, "1\n");
            else
                fprintf(fileout, "0\n");
            fflush(fileout);
        } else

        if(!strcmp(command, "gtk_editable_get_text")){
            const char* mtext=gtk_editable_get_text(GTK_EDITABLE(widget));
            fprintf(fileout, "%s\n", mtext);  
            fflush(fileout);
        }
        

    }

    fclose(filein);
    fflush(fileout);
    fclose(fileout);
    pthread_exit(NULL);
  }
void wrap_add_signals(char *filename, _args* pargs){
    if(DEBUG) fprintf(stderr, "START wrap_add_signals...\n");
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
    if(!pargs->SIGNALS){
        if(VERBOSE) printf( "Error allocating memory: pargs->SIGNALS!\n");
        return;
    }    
    if(!file){
        if(VERBOSE) printf( "Couldn't open file %s, no signals will be auto-handled!\n", filename);
        return;
    }
    while(!feof(file)){
        fgets(line, STRING_SIZE, file);

        if((a = strstr(line, OBJECT_TAG)) != NULL){
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
        if ((a = strstr(line, SIGNAL_TAG)) != NULL){
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
static void add_styles(_args* pargs) {
    void start(void *user_data, const char *ele, const char **attr){
        _args *pargs = (_args*)user_data;
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
        _args *pargs = (_args*)user_data;
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
        XML_SetUserData(parser, pargs);
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
static void add_css(_args *pargs, GtkWidget *win) {
    GdkDisplay *display;
    GtkCssProvider *provider;
    if(pargs->css_file == NULL || access(pargs->css_file, F_OK) != 0) 
        return;
    if(DEBUG) fprintf(stderr, "Adding css file %s...\n", pargs->css_file);
    display = gdk_display_get_default ();
    provider = gtk_css_provider_new ();
    gtk_css_provider_load_from_path(provider, pargs->css_file);
    gtk_style_context_add_provider_for_display (display, 
                                GTK_STYLE_PROVIDER (provider), 
                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    add_styles(pargs);
    g_signal_connect (win, "destroy", G_CALLBACK (before_destroy), provider);
    g_object_unref (provider);
  }

static void app_query_end(GtkApplication *app, gpointer *user_data) { }
static void app_window_added(GtkApplication *app, GtkWindow *win, gpointer *user_data) { }
static void app_window_removed(GtkApplication *app, GtkWindow *win, gpointer *user_data) { }
static void app_command_line(GtkApplication *app, GApplicationCommandLine *cmd_line, gpointer *user_data) { }
static void app_handle_local_options(GtkApplication *app, GVariantDict *options, gpointer *user_data) { }
static void app_name_lost(GtkApplication *app, gpointer *user_data) { }
static void app_notify(GtkApplication *app, GParamSpec *pspec, gpointer *user_data) { }
static void app_action_added(GtkApplication *app, gchar* action_name, gpointer *user_data) { }
static void app_action_enabled_changed(GtkApplication *app, gchar* action_name, gboolean enabled, gpointer *user_data) { }
static void app_action_removed(GtkApplication *app, gchar* action_name, gpointer *user_data) { }
static void app_action_state_changed(GtkApplication *app, gchar* action_name, GVariant* value, gpointer *user_data) { }
static void app_startup(GtkApplication *app, gpointer *user_data) {
    if(DEBUG) fprintf(stderr, "START app_startup...\n");
    _args *pargs = (_args *)user_data;
    GtkWidget *win = NULL; // GtkDialog deprecated with GTK4.10, use GtkWindow

    adw_init();
    pargs->builder = gtk_builder_new();
    gtk_builder_add_from_file(pargs->builder, pargs->ui_file, NULL);
    win = GTK_WIDGET(gtk_builder_get_object(pargs->builder, pargs->win_id));
    gtk_window_set_application(GTK_WINDOW(win), GTK_APPLICATION(app));
    wrap_add_signals(pargs->ui_file, pargs);
    if(DEBUG) fprintf(stderr, "END  app_startup...\n");
  }
static void app_do(GtkApplication *app, gpointer *user_data) {
    if(DEBUG) fprintf(stderr, "START app_do...\n");
    _args *pargs = (_args *)user_data;
    GtkWidget *win;
    win = GTK_WIDGET(gtk_builder_get_object(pargs->builder, pargs->win_id));
    gtk_window_present(GTK_WINDOW(win));
    RUNNING = 1;
    if(pargs->fpipeout && !pargs->thread)
        pthread_create(&(pargs->thread), NULL, wrap_reader_loop, pargs);
    add_css(pargs, win);
    if(DEBUG) fprintf(stderr, "END  app_do...\n");
}
static void app_activate(GtkApplication *app, gpointer user_data) {
    /* Dieser Funktion endet mit dem Anzeigen des Dialogs. 
     * Danach befinden wir uns in g_application_run */
    app_do(app, user_data);
  }
static void app_open(GtkApplication *app, GFile ** files, gint n_files, gchar *hint, gpointer *user_data) {
    /* Dieser Funktion endet mit dem Anzeigen des Dialogs. 
     * Danach befinden wir uns in g_application_run */
    app_do(app, user_data);
  }
static void before_destroy (GtkWidget *win, GtkCssProvider *provider) {
    if(DEBUG) fprintf(stderr, "START before_destroy...\n");
    // Wenn die Applikation beendet wird, werden provider automatisch
    // entfernt. Dieser Code ist hier ist nicht unbedingt nötig.
    GdkDisplay *display = gdk_display_get_default ();
    gtk_style_context_remove_provider_for_display (display, GTK_STYLE_PROVIDER (provider));
    if(DEBUG) fprintf(stderr, "END  before_destroy...\n");
}
static void app_shutdown(GtkApplication *app, gpointer *user_data) {
    if(DEBUG) fprintf(stderr, "START app_end...\n");
    RUNNING = 0;
  }
static void help(char *appname){
    printf("\
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
                if(VERBOSE) printf("-%c\n"); 
                break; 
            case 'v': 
                VERBOSE = 1;
                if(VERBOSE) printf("-%c\n"); 
                break; 
            case 'h': 
                if(VERBOSE) printf("-%c\n"); 
                help(pargs->app_name);
                break; 
            case 'f': 
                if(VERBOSE) printf("-%c: %s\n", opt, optarg);
                if(optarg[0]=='-') printf("ACHTUNG: -%c: %s\n", opt, optarg);
                pargs->ui_file  = optarg;
                break; 
            case 's': 
                if(VERBOSE) printf("-%c: %s\n", opt, optarg);
                if(optarg[0]=='-') printf("ACHTUNG: -%c: %s\n", opt, optarg);
                pargs->css_file  = optarg;
                break; 
            case 'm': 
                if(VERBOSE) printf("-%c: %s\n", opt, optarg);
                if(optarg[0]=='-') printf("ACHTUNG: -%c: %s\n", opt, optarg);
                pargs->win_id  = optarg;
                break; 
            case 'o': 
                if(VERBOSE) printf("-%c: %s\n", opt, optarg);
                if(optarg[0]=='-') printf("ACHTUNG: -%c: %s\n", opt, optarg);
                pargs->fpipeout  = optarg;
                break; 
            case 'i': 
                if(VERBOSE) printf("-%c: %s\n", opt, optarg);
                if(optarg[0]=='-') printf("ACHTUNG: -%c: %s\n", opt, optarg);
                pargs->fpipein  = optarg;
                break; 
            case ':': 
                if(VERBOSE) printf("Die Option braucht einen Wert\n"); 
                break; 
            case '?': 
                if(VERBOSE) printf("Option %c wird nicht unterstützt\n", optopt);
                break; 
        } 
    } 
    for(pos=1; optind < argc; optind++){
        if(VERBOSE) printf("argv[%i]=%s\n", pos,argv[optind]); 
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
    args.depth     = 0;
    args.id        = NULL;
    args.wid       = NULL;

    read_opts(&args, &argc, &argv);
    if(!args.ui_file) 
        help(args.app_name);
    if((args.fpipeout && !args.fpipein) || (args.fpipein && !args.fpipeout))
        help(args.app_name);
    if(VERBOSE) for(idx=0; idx < argc; idx++)printf("%i: %s\n", idx, argv[idx]);
    if(VERBOSE) printf("UI-Datei: %s; TOP-WINDOW: %s\n", args.ui_file, args.win_id);

    app = gtk_application_new(APP_ID, G_APPLICATION_HANDLES_OPEN);
    // Empty handlers, perhabs to be used later
    g_object_set(app, "register-session", TRUE, NULL);
    g_signal_connect(app, "query-end", G_CALLBACK (app_query_end), &args);
    g_signal_connect(app, "window-added", G_CALLBACK (app_window_added), &args);
    g_signal_connect(app, "window-removed", G_CALLBACK (app_window_removed), &args);
    g_signal_connect(app, "command-line", G_CALLBACK (app_command_line), &args);
    g_signal_connect(app, "handle-local-options", G_CALLBACK (app_handle_local_options), &args);
    g_signal_connect(app, "name-lost", G_CALLBACK (app_name_lost), &args);
    g_signal_connect(app, "notify", G_CALLBACK (app_notify), &args);
    g_signal_connect(app, "action-added", G_CALLBACK (app_action_added), &args);
    g_signal_connect(app, "action-enabled-changed", G_CALLBACK (app_action_enabled_changed), &args);
    g_signal_connect(app, "action-removed", G_CALLBACK (app_action_removed), &args);
    g_signal_connect(app, "action-state-changed", G_CALLBACK (app_action_state_changed), &args);
    // Used handlers
    g_signal_connect(app, "startup", G_CALLBACK (app_startup), &args);
    g_signal_connect(app, "activate", G_CALLBACK (app_activate), &args);
    g_signal_connect(app, "open", G_CALLBACK (app_open), &args);
    g_signal_connect(app, "shutdown", G_CALLBACK (app_shutdown), &args);
    stat = g_application_run(G_APPLICATION(app), argc, argv);
    if(DEBUG) fprintf(stderr, "ENDE App läuft nicht mehr...\n");
    wrap_cleanup(&args);
    g_object_unref(app);
    return stat;
}

