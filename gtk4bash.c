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
#include "gtk4bash.h"

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
//
typedef void (* sigfu)(GtkBuilder*,char*, char*, char*, char*, char*, char*, char*);
//
typedef struct {char* name; void* wrapper_fup; void* gtk_fup;} FU;
typedef struct {int size;FU* fus;} FUS;
FUS cmds = {0,NULL};
void table_free() {
    free(cmds.fus);
  }
void table_add(char* name, void* wrapper_fup, void* gtk_fup) {
    if(!cmds.size) cmds.fus=malloc(sizeof(FU));
    else cmds.fus=realloc(cmds.fus, sizeof(FU)*(cmds.size+1));
    (cmds.fus + cmds.size)->name = malloc(strlen(name) + 1);
    strcpy((cmds.fus + cmds.size)->name, name);
    (cmds.fus + cmds.size)->wrapper_fup = wrapper_fup;
    (cmds.fus + cmds.size)->gtk_fup = gtk_fup;
    cmds.size++;
  }
void* table_get_gtk_fu( char* name) {
    for (int i = 0; i < cmds.size; i++)
        if (strcmp((cmds.fus + i)->name, name)==0)return(cmds.fus + i)->gtk_fup;
    return NULL;
  }
void* table_get_sig_fu( char* name) {
    for (int i = 0; i < cmds.size; i++)
        if (strcmp((cmds.fus + i)->name, name)==0)return(cmds.fus + i)->wrapper_fup;
    return NULL;
  }
//
void trim(char *s) {
    size_t len = strspn(s, " ");
    if (len > 0) memmove(s, s + len, strlen(s) - len + 1);
  }
const char*atoa(const char*s) {return s;}

typedef struct PTRS {void* ptr; struct PTRS*next;} PTRS;
PTRS* pfirst=NULL;
PTRS* plast=NULL;
void free_ptrs() {
    PTRS*run=pfirst;
    while(run) {
        pfirst=run->next;
        free(run);
        run=pfirst;
    }
    plast=NULL;
}
void *atop(const char*s) {
    //if(DEBUG) fprintf(stderr,"        START %s (%s)\n",__func__,s);
    if(plast==NULL) {
        plast=malloc(sizeof(PTRS));
        pfirst=plast;
    } else {
        plast->next=malloc(sizeof(PTRS));
        plast=plast->next;
    }
    plast->next=NULL;
    sscanf(s, "%p", &(plast->ptr));
    //if(DEBUG) fprintf(stderr,"        ENDE %s (%p)\n",__func__,plast->ptr);
    return plast->ptr;
}
void *addp(void* p) {
    if(plast==NULL) {
        plast=malloc(sizeof(PTRS));
        pfirst=plast;
    } else {
        plast->next=malloc(sizeof(PTRS));
        plast=plast->next;
    }
    plast->next=NULL;
    plast->ptr=p;
    return plast->ptr;
}

typedef struct {
    char       *app_name;
    char       *ui_file;
    char       *css_file;
    char       *name_out;
    char       *name_in;
    char       *win_id;
    GtkBuilder *builder;
    char*      *SIGNALS;
    pthread_t   thread;
    FILE       *fpout;
    FILE       *fpin;
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
//
void cbk_wrap_signal_handler(gpointer user_data, GObject *object) {
    char *signal = (char *)user_data;
    fprintf(stdout, "%s\n", signal);
    fflush(stdout);
}

void *wrap_reader_loop(void* user_data) {
    if(DEBUG) fprintf(stderr, "START %s()... - ENDE wird nie erreicht.\n", __func__);
    _args *pargs = (_args *)user_data;
    mkfifo(pargs->name_out, S_IRWXU);
    pargs->fpout = fopen(pargs->name_out, "a+");
    if(!pargs->fpout) {
        fprintf(stderr, "Error opening pipe %s !\n", pargs->name_out);
        pthread_exit(NULL);
    }
    mkfifo(pargs->name_in, S_IRWXU);
    pargs->fpin = fopen(pargs->name_in, "r+");
    if(!pargs->fpin) {
        fprintf(stderr, "Error opening pipe %s !\n", pargs->name_in);
        pthread_exit(NULL);
    }

    if(VERBOSE) fprintf(stderr, "Using pipes out:%s in:%s\n", pargs->name_out, pargs->name_in);

    char input[1024];
    char strend = ' ';
    char *command = NULL;
    char *arg1 = NULL; char *arg2 = NULL; char *arg3 = NULL; char *arg4 = NULL; char *arg5 = NULL; char *arg6 = NULL;
    void *sig_vu;
    //
    // DEFinitionen alphabetisch
      #pragma GCC diagnostic push
      #pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
      #pragma GCC diagnostic pop
      SIGFU_DEF_RET(
        GtkTextDirection,GtkTextDirection,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET(
        constcharP,const char*,atoa,"%s",NONO,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        constcairo_font_options_tP,const cairo_font_options_t *,atop,"%p",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        constcharP,const char*,atoa,"%s",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkLabelP,GtkLabel*,GTK_LABEL,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        constcharP,const char*,atoa,"%s",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        constcharP,const char*,atoa,"%s",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetClassP,GtkWidgetClass*,GTK_WIDGET_CLASS,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        constcharP,const char*,atoa,"%s",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWindowP,GtkWindow*,GTK_WINDOW,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      #pragma GCC diagnostic push
      #pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
      SIGFU_DEF_RETFREE_PAR2_PAR3_PAR4_PAR1(
        charP,char*,atoa,"%s",NONO,free,DUMMY7,SIGPAR_END,
        constGtkTextIterP,const GtkTextIter*,atop,"%p",NONO,NOARR,DUMMY7,SIGPAR_END,
        constGtkTextIterP,const GtkTextIter*,atop,"%p",NONO,NOARR,DUMMY7,SIGPAR_END,
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkTextBufferP,GtkTextBuffer*,atop,"%p",NONO,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      #pragma GCC diagnostic pop
      SIGFU_DEF_RETARR_PAR1(
        charPP,char**,atoa,"%s",NONO,g_strfreev,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        double,double,atof,"%d",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        float,float,atof,"%f",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkLabelP,GtkLabel*,GTK_LABEL,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkLabelP,GtkLabel*,GTK_LABEL,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkTextViewP,GtkTextView*,GTK_TEXT_VIEW,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWindowP,GtkWindow*,GTK_WINDOW,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        GtkJustification,GtkJustification,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkLabelP,GtkLabel*,GTK_LABEL,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        GtkJustification,GtkJustification,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkTextViewP,GtkTextView*,GTK_TEXT_VIEW,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        GtkNaturalWrapMode,GtkNaturalWrapMode,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkLabelP,GtkLabel*,GTK_LABEL,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        PangoEllipsizeMode,PangoEllipsizeMode,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkLabelP,GtkLabel*,GTK_LABEL,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        PangoWrapMode,PangoWrapMode,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkLabelP,GtkLabel*,GTK_LABEL,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        GtkStateFlags,GtkStateFlags,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        GtkTextDirection,GtkTextDirection,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        guint,guint,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkLabelP,GtkLabel*,GTK_LABEL,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        guint,guint,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetClassP,GtkWidgetClass*,GTK_WIDGET_CLASS,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        int,int,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkLabelP,GtkLabel*,GTK_LABEL,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        int,int,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkTextViewP,GtkTextView*,GTK_TEXT_VIEW,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        int,int,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        GtkTextBufferP,GtkTextBuffer*,atop,"%p",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkTextViewP,GtkTextView*,GTK_TEXT_VIEW,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      #pragma GCC diagnostic push
      #pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
      SIGFU_DEF_RET_PAR1(
        int,int,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkTextBufferP,GtkTextBuffer*,atop,"%p",NONO,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkTextBufferP,GtkTextBuffer*,atop,"%p",NONO,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR1(
        guint,guint,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkTextBufferP,GtkTextBuffer*,atop,"%p",NONO,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR2_PAR1(
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        constcharP,const char*,atoa,"%s",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR2_PAR1(
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR2_PAR1(
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkDirectionType,GtkDirectionType,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR2_PAR1(
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkOrientation,GtkOrientation,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR2_PAR1(
        int,int,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkOrientation,GtkOrientation,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR2_PAR3_PAR1(
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        double,double,atof,"%d",NONO,NOARR,DUMMY7,SIGPAR_END,
        double,double,atof,"%d",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_RET_PAR2_PAR3_PAR4_PAR5_PAR1(
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        int,int,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        int,int,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        int,int,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        int,int,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      #pragma GCC diagnostic pop
      #if FALSE
        gboolean FU(GtkWidgetClass*,guint,GType*,const char**,const GVariantType**,const char**);
        gboolean FU(GtkWidget*,const char*,const char*,...);
        gboolean FU(GtkWidget*,const char*,GVariant*);
        gboolean FU(GtkWidget*,GtkWidget*,const graphene_point_t*,graphene_point_t*);
        gboolean FU(GtkWidget*,GtkWidget*,double,double,double*,double*);
        gboolean FU(GtkWidget*,GtkWidget*,graphene_matrix_t*);
        gboolean FU(GtkWidget*,GtkWidget*);
        gboolean FU(GtkWidget*,GtkWidget*,graphene_rect_t*);
        GdkClipboard* FU(GtkWidget*);
        GdkCursor* FU(GtkWidget*);
        GdkDisplay* FU(GtkWidget*);
        GdkFrameClock* FU(GtkWidget*);
        GList* FU();
        GList* FU(GtkWidget*);
        GListModel* FU();
        GListModel* FU(GtkWidget*);
        GObject* FU(GtkWidget*,GType,const char*);
        GtkAccessibleRole FU(GtkWidgetClass*);
        GtkAlign FU(GtkWidget*);
        GtkApplication* FU(GtkWindow*);
        GtkLayoutManager* FU(GtkWidget*);
        GtkNative* FU(GtkWidget*);
        GtkOverflow FU(GtkWidget*);
        GtkRoot* FU(GtkWidget*);
        GtkSettings* FU(GtkWidget*);
        GtkSizeRequestMode FU(GtkWidget*);
        GtkStyleContext* FU(GtkWidget*);
        GtkWidget* FU(GtkWidget*);
        GtkWidget* FU(GtkWidget*,double,double,GtkPickFlags);
        GtkWidget* FU(GtkWidget*,GType);
        GtkWidget* FU(GtkWindow*);
        #if GTK_MINOR_VERSION >= 20
          GtkWindowGravity FU(GtkWindow*);
        #endif
        GtkWindowGroup  FU(GtkWindow*);
        GtkWindow*  FU(GtkWindow*);
        GType FU(GtkWidgetClass*);
        guint FU(GtkWidget*,GtkTickCallback,gpointer,GDestroyNotify);
        PangoContext* FU(GtkWidget*);
        PangoFontMap* FU(GtkWidget*);
        PangoLayout* FU(GtkWidget*,const char*);
      #endif // FALSE
      SIGFU_DEF_VOID_PAR1(
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR1(
        GtkWindowP,GtkWindow*,GTK_WINDOW,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      #pragma GCC diagnostic push
      #pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
      SIGFU_DEF_VOID_PAR1(
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR1(
        GtkTextDirection,GtkTextDirection,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR1(
        constcharP,const char*,atoa,"%s",NONO,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        constcharP,const char*,atoa,"%s",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkLabelP,GtkLabel*,GTK_LABEL,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        constcharP,const char*,atoa,"%s",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetClassP,GtkWidgetClass*,GTK_WIDGET_CLASS,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        constcharP,const char*,atoa,"%s",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        constcharP,const char*,atoa,"%s",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWindowP,GtkWindow*,GTK_WINDOW,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        double,double,atof,"%d",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        float,float,atof,"%f",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkLabelP,GtkLabel*,GTK_LABEL,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkLabelP,GtkLabel*,GTK_LABEL,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkTextViewP,GtkTextView*,GTK_TEXT_VIEW,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWindowP,GtkWindow*,GTK_WINDOW,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        GtkJustification,GtkJustification,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkLabelP,GtkLabel*,GTK_LABEL,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        GtkJustification,GtkJustification,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkTextViewP,GtkTextView*,GTK_TEXT_VIEW,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        GtkNaturalWrapMode,GtkNaturalWrapMode,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkLabelP,GtkLabel*,GTK_LABEL,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        guint,guint,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetClassP,GtkWidgetClass*,GTK_WIDGET_CLASS,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        guint,guint,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        guint32,guint32,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWindowP,GtkWindow*,GTK_WINDOW,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        int,int,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkLabelP,GtkLabel*,GTK_LABEL,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        int,int,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkTextViewP,GtkTextView*,GTK_TEXT_VIEW,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        int,int,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        PangoEllipsizeMode,PangoEllipsizeMode,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkLabelP,GtkLabel*,GTK_LABEL,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        PangoWrapMode,PangoWrapMode,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkLabelP,GtkLabel*,GTK_LABEL,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        GtkAccessibleRole,GtkAccessibleRole,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetClassP,GtkWidgetClass*,GTK_WIDGET_CLASS,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        GType,GType,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetClassP,GtkWidgetClass*,GTK_WIDGET_CLASS,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        GType,GType,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        GtkAlign,GtkAlign,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        GtkOverflow,GtkOverflow,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        GtkStateFlags,GtkStateFlags,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        GtkTextDirection,GtkTextDirection,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR1(
        GtkTextIterP,GtkTextIter*,atop,"%p",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkTextBufferP,GtkTextBuffer*,GTK_TEXT_BUFFER,"%p",OOBJECT,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR3_PAR1(
        GtkStateFlags,GtkStateFlags,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR3_PAR1(
        int,int,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        int,int,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR3_PAR1(
        int,int,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        int,int,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWindowP,GtkWindow*,GTK_WINDOW,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR3_PAR1(
        constcharP,const char*,atoa,"%s",NONO,NOARR,DUMMY7,SIGPAR_END,
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR3_PAR1(
        constcharP,const char*,atoa,"%s",NONO,NOARR,DUMMY7,SIGPAR_END,
        constcharP,const char*,atoa,"%s",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR3_PAR1(
        constcharP,const char*,atoa,"%s",NONO,NOARR,DUMMY7,SIGPAR_END,
        int,int,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkTextBufferP,GtkTextBuffer*,GTK_TEXT_BUFFER,"%p",OOBJECT,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      SIGFU_DEF_VOID_PAR2_PAR3_PAR4_PAR1(
        constcharP,const char*,atoa,"%s",NONO,NOARR,DUMMY7,SIGPAR_END,
        gboolean,gboolean,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        gssize,gssize,atoi,"%i",NONO,NOARR,DUMMY7,SIGPAR_END,
        GtkWidgetClassP,GtkWidgetClass*,GTK_WIDGET_CLASS,"%p",YESYES,NOARR,DUMMY7,SIGPAR_END,
        SIGDEF_END)
      #pragma GCC diagnostic pop
      #if FALSE
        FU(GtkWidgetClass*,const char*,const char*,GtkWidgetActionActivateFunc);
        FU(GtkWidgetClass*,const char*,GCallback);
        FU(GtkWidgetClass*,GBytes*);
        FU(GtkWidgetClass*,GtkBuilderScope*);
        FU(GtkWidgetClass*,GtkShortcut*);
        FU(GtkWidgetClass*,guint,GdkModifierType,const char*,const char*,...);
        FU(GtkWidgetClass*,guint,GdkModifierType,GtkShortcutFunc,const char*,...);
        FU(GtkWidget*,const cairo_font_options_t*);
        FU(GtkWidget*,const char*,GActionGroup*);
        FU(GtkWidget*,const char**);
        FU(GtkWidget*,const GtkAllocation*,int);
        FU(GtkWidget*,GdkCursor*);
        FU(GtkWidget*,GdkRGBA);
        FU(GtkWidget*,GtkAllocation*);
        FU(GtkWidget*,GtkEventController*);
        FU(GtkWidget*,GtkLayoutManager*);
        FU(GtkWidget*,GtkOrientation,int,int*,int*,int*,int*);
        FU(GtkWidget*,GtkRequisition*,GtkRequisition*);
        FU(GtkWidget*,GtkWidget*);
        FU(GtkWidget*,GtkWidget*,GtkSnapshot*);
        FU(GtkWidget*,GtkWidget*,GtkWidget*);
        FU(GtkWidget*,int,int,int,GskTransform*);
        FU(GtkWidget*,int*,int*);
        FU(GtkWidget*,PangoFontMap*);
        FU(GtkWindow*,GtkWindow*);
        FU(GtkWindow*,GdkDisplay*);
        FU(GtkWindow*,GdkMonitor*);
        FU(GtkWindow*,GtkApplication*);
        FU(GtkWindow*,GtkWidget);
        #if GTK_MINOR_VERSION >= 20
          FU(GtkWindow*, GtkWindowGravity);
        #endif
        FU(GtkWindow*, int*, int*);
      #endif // FALSE
    // CONnections (gtk Funktionen) in der Reihenfolge der doc
      /* GtkWidget ****************************************************001-199*/
        /* Functions */
          SIGFU_CON_RET(gtk_widget_get_default_direction,GtkTextDirection)
          SIGFU_CON_VOID_PAR1(gtk_widget_set_default_direction,GtkTextDirection)
        /* Instance methods */
          SIGFU_CON_VOID_PAR1_PAR2_PAR3(gtk_widget_action_set_enabled,GtkWidgetP,constcharP,gboolean)
          SIGFU_CON_RET_PAR1(gtk_widget_activate,gboolean,GtkWidgetP)
          /*table_gboolean_GtkWidgetP_constcharP_constcharP_Elipse*/ /*gtk_widget_activate_action*/
          /*table_gboolean_GtkWidgetP_constcharP_GVariantP*/ /*gtk_widget_activate_action_variant*/
          SIGFU_CON_VOID_PAR1(gtk_widget_activate_default,GtkWidgetP)
          /*table_void_GtkWidgetP_GtkEventControllerP*/ /*gtk_widget_add_controller*/
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_add_css_class,GtkWidgetP,constcharP)
          //xx SIGFU_CON_VOID_(gtk_widget_add_mnemonic_label, _GtkWidgetP_GtkWidgetP)
          /*table_guint_GtkWidgetP_GtkTickCallback_gpointer_GDestroyNotify*/ /*gtk_widget_add_tick_callback*/
          /*table_void_GtkWidgetP_int_int_int_GskTransformP*/ /*gtk_widget_allocate*/
          SIGFU_CON_RET_PAR1_PAR2(gtk_widget_child_focus,gboolean,GtkWidgetP,GtkDirectionType)
          /*table_gboolean_GtkWidgetP_GtkWidgetP_graphene_rect_tP*/ /*gtk_widget_compute_bounds*/
          /*table_gboolean_GtkWidgetP_GtkOrientation*/ /*gtk_widget_compute_expand*/
          /*table_gboolean_GtkWidgetP_GtkWidgetP_constgraphene_point_tP_graphene_point_tP*/ /*gtk_widget_compute_point*/
          /*table_gboolean_GtkWidgetP_GtkWidgetP_graphene_matrix_tP*/ /*gtk_widget_compute_transform*/
          SIGFU_CON_RET_PAR1_PAR2_PAR3(gtk_widget_contains,gboolean,GtkWidgetP,double,double)
          /*table_PangoContextP_GtkWidgetP*/ /*gtk_widget_create_pango_context*/
          /*table_PangoLayoutP_GtkWidgetP_constcharP*/ /*gtk_widget_create_pango_layout*/
          /*table_void_GtkWidgetP_GType*/ /*gtk_widget_dispose_template*/
          /*table_gboolean_GtkWidgetP_int_int_int_int*/ /*gtk_drag_check_threshold*/
          SIGFU_CON_VOID_PAR1(gtk_widget_error_bell,GtkWidgetP)
          #if GTK_MINOR_VERSION < 12
            SIGFU_CON_RET_PAR1(gtk_widget_get_allocated_baseline,int,GtkWidgetP)
            SIGFU_CON_RET_PAR1(gtk_widget_get_allocated_height,int,GtkWidgetP)
            SIGFU_CON_RET_PAR1(gtk_widget_get_allocated_width,int,GtkWidgetP)
            //xx SIGFU_CON_VOID_(gtk_widget_get_allocation, _GtkWidgetP_GtkAllocationP)
          #endif
          /*table_GtkWidgetP_GtkWidgetP_GType*/ /*gtk_widget_get_ancestor*/
          #if GTK_MINOR_VERSION >= 12
            SIGFU_CON_RET_PAR1(gtk_widget_get_baseline,int,GtkWidgetP)
          #endif
          SIGFU_CON_RET_PAR1(gtk_widget_get_can_focus,gboolean,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_get_can_target,gboolean,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_get_child_visible,gboolean,GtkWidgetP)
          /*table_GdkClipboardP_GtkWidgetP*/ /*gtk_widget_get_clipboard*/
          #if GTK_MINOR_VERSION >= 10
            table_add(table_void_GtkWidgetP_GdkRGBA, "gtk_widget_get_color",gtk_widget_get_color);
          #endif
          SIGFU_CON_RET_PAR1(gtk_widget_get_css_classes,charPP,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_get_css_name,constcharP,GtkWidgetP)
          /*table_GdkCursorP_GtkWidgetP*/ /*gtk_widget_get_cursor*/
          //xx table_add(gtk_widget_get_direction, _GtkTextDirection_GtkWidgetP);
          /*table_GdkDisplayP_GtkWidgetP*/ /*gtk_widget_get_display*/
          /*table_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_get_first_child*/
          /*table_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_get_focus_child*/
          SIGFU_CON_RET_PAR1(gtk_widget_get_focus_on_click,gboolean,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_get_focusable,gboolean,GtkWidgetP)
          /*table_PangoFontMapP_GtkWidgetP*/ /*gtk_widget_get_font_map*/
          #if GTK_MINOR_VERSION < 16
            SIGFU_CON_RET_PAR1(gtk_widget_get_font_options,constcairo_font_options_tP,GtkWidgetP)
          #endif
          /*table_GdkFrameClockP_GtkWidgetP*/ /*gtk_widget_get_frame_clock*/
          //xx table_add(gtk_widget_get_halign, _GtkAlign_GtkWidgetP);
          SIGFU_CON_RET_PAR1(gtk_widget_get_has_tooltip,gboolean,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_get_height,int,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_get_hexpand,gboolean,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_get_hexpand_set,gboolean,GtkWidgetP)
          /*table_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_get_last_child*/
          /*table_GtkLayoutManagerP_GtkWidgetP*/ /*gtk_widget_get_layout_manager*/
          #if GTK_MINOR_VERSION >= 18
            table_add(table_gboolean_GtkWidgetP, "gtk_widget_get_limit_events",gtk_widget_get_limit_events);
          #endif
          SIGFU_CON_RET_PAR1(gtk_widget_get_mapped,gboolean,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_get_margin_bottom,int,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_get_margin_end,int,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_get_margin_start,int,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_get_margin_top,int,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_get_name,constcharP,GtkWidgetP)
          /*table_GtkNativeP_GtkWidgetP*/ /*gtk_widget_get_native*/
          /*table_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_get_next_sibling*/
          SIGFU_CON_RET_PAR1(gtk_widget_get_opacity,double,GtkWidgetP)
          //xx table_add(gtk_widget_get_overflow, _GtkOverflow_GtkWidgetP);
          /*table_PangoContextP_GtkWidgetP*/ /*gtk_widget_get_pango_context*/
          /*table_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_get_parent*/
          /*table_void_GtkWidgetP_GtkRequisitionP_GtkRequisitionP*/ /*gtk_widget_get_preferred_size*/
          /*table_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_get_prev_sibling*/
          /*table_GdkClipboardP_GtkWidgetP*/ /*gtk_widget_get_primary_clipboard*/
          SIGFU_CON_RET_PAR1(gtk_widget_get_realized,gboolean,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_get_receives_default,gboolean,GtkWidgetP)
          //xx table_add(gtk_widget_get_request_mode, _GtkSizeRequestMode_GtkWidgetP);
          /*table_GtkRootP_GtkWidgetP*/ /*gtk_widget_get_root*/
          SIGFU_CON_RET_PAR1(gtk_widget_get_scale_factor,int,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_get_sensitive,gboolean,GtkWidgetP)
          /*table_GtkSettingsP_GtkWidgetP*/ /*gtk_widget_get_settings*/
          //xx table_add(gtk_widget_get_size, _int_GtkWidgetP_GtkOrientation);
          //xx SIGFU_CON_VOID_(gtk_widget_get_size_request, _GtkWidgetP_intP_intP)
          //xx table_add(gtk_widget_get_state_flags, _GtkStateFlags_GtkWidgetP);
          #if GTK_MINOR_VERSION < 10
            /*table_GtkStyleContextP_GtkWidgetP*/ /*gtk_widget_get_style_context*/
          #endif
          //xx table_add(gtk_widget_get_template_child, _GObjectP_GtkWidgetP_GType_constcharP);
          SIGFU_CON_RET_PAR1(gtk_widget_get_tooltip_markup,constcharP,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_get_tooltip_text,constcharP,GtkWidgetP)
          //xx table_add(gtk_widget_get_valign, _GtkAlign_GtkWidgetP);
          SIGFU_CON_RET_PAR1(gtk_widget_get_vexpand,gboolean,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_get_vexpand_set,gboolean,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_get_visible,gboolean,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_get_width,int,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_grab_focus,gboolean,GtkWidgetP)
          //xx table_add(gtk_widget_has_css_class, _gboolean_GtkWidgetP_constcharP);
          SIGFU_CON_RET_PAR1(gtk_widget_has_default,gboolean,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_has_focus,gboolean,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_has_visible_focus,gboolean,GtkWidgetP)
          #if GTK_MINOR_VERSION < 10
            SIGFU_CON_VOID_PAR1(gtk_widget_hide,GtkWidgetP)
          #endif
          SIGFU_CON_RET_PAR1(gtk_widget_in_destruction,gboolean,GtkWidgetP)
          SIGFU_CON_VOID_PAR1(gtk_widget_init_template,GtkWidgetP)
          /*table_void_GtkWidgetP_constcharP_GActionGroupP*/ /*gtk_widget_insert_action_group*/
          /*table_void_GtkWidgetP_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_insert_after*/
          /*table_void_GtkWidgetP_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_insert_before*/
          /*table_gboolean_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_is_ancestor*/
          SIGFU_CON_RET_PAR1(gtk_widget_is_drawable,gboolean,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_is_focus,gboolean,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_is_sensitive,gboolean,GtkWidgetP)
          SIGFU_CON_RET_PAR1(gtk_widget_is_visible,gboolean,GtkWidgetP)
          //xx table_add(gtk_widget_keynav_failed, _gboolean_GtkWidgetP_GtkDirectionType);
          /*table_GListP_GtkWidgetP*/ /*gtk_widget_list_mnemonic_labels*/
          SIGFU_CON_VOID_PAR1(gtk_widget_map,GtkWidgetP)
          /*table_void_GtkWidgetP_GtkOrientation_int_intP_intP_intP_intP*/ /*gtk_widget_measure*/
          //xx table_add(gtk_widget_mnemonic_activate, _gboolean_GtkWidgetP_gboolean);
          /*table_GListModelP_GtkWidgetP*/ /*gtk_widget_observe_children*/
          /*table_GListModelP_GtkWidgetP*/ /*gtk_widget_observe_controllers*/
          /*table_GtkWidgetP_GtkWidgetP_double_double_GtkPickFlags*/ /*gtk_widget_pick*/
          SIGFU_CON_VOID_PAR1(gtk_widget_queue_allocate,GtkWidgetP)
          SIGFU_CON_VOID_PAR1(gtk_widget_queue_draw,GtkWidgetP)
          SIGFU_CON_VOID_PAR1(gtk_widget_queue_resize,GtkWidgetP)
          SIGFU_CON_VOID_PAR1(gtk_widget_realize,GtkWidgetP)
          /*table_void_GtkWidgetP_GtkEventControllerP*/ /*gtk_widget_remove_controller*/
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_remove_css_class,GtkWidgetP,constcharP)
          /*table_void_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_remove_mnemonic_label*/
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_remove_tick_callback,GtkWidgetP,guint)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_can_focus, GtkWidgetP,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_can_target,GtkWidgetP,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_child_visible,GtkWidgetP,gboolean)
          // SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_css_classes,GtkWidgetP,constcharPP)
          /*table_void_GtkWidgetP_GdkCursorP*/ /*gtk_widget_set_cursor*/
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_cursor_from_name,GtkWidgetP,constcharP)
          /*table_void_GtkWidgetP_GtkTextDirection*/ /*gtk_widget_set_direction*/
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_focusable, GtkWidgetP,gboolean)
          /*table_void_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_set_focus_child*/
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_focus_on_click, GtkWidgetP,gboolean)
          /*table_void_GtkWidgetP_PangoFontMapP*/ /*gtk_widget_set_font_map*/
          #if GTK_MINOR_VERSION < 16
            /*table_void_GtkWidgetP_constcairo_font_options_tP*/ /*gtk_widget_set_font_options*/
          #endif
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_halign,GtkWidgetP,GtkAlign)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_has_tooltip,GtkWidgetP,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_hexpand,GtkWidgetP,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_hexpand_set,GtkWidgetP,gboolean)
          /*table_void_GtkWidgetP_GtkLayoutManagerP*/ /*gtk_widget_set_layout_manager*/
          #if GTK_MINOR_VERSION >= 18
            /*table_void_GtkWidgetP_gboolean*/ /*gtk_widget_set_limit_events*/
          #endif
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_margin_bottom,GtkWidgetP,int)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_margin_end,GtkWidgetP,int)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_margin_start,GtkWidgetP,int)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_margin_top,GtkWidgetP,int)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_name,GtkWidgetP,constcharP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_opacity,GtkWidgetP,double)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_overflow,GtkWidgetP,GtkOverflow)
          /*table_void_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_set_parent*/
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_receives_default,GtkWidgetP,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_sensitive,GtkWidgetP,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2_PAR3(gtk_widget_set_size_request,GtkWidgetP,int,int)
          SIGFU_CON_VOID_PAR1_PAR2_PAR3(gtk_widget_set_state_flags,GtkWidgetP,GtkStateFlags,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_tooltip_markup,GtkWidgetP,constcharP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_tooltip_text,GtkWidgetP,constcharP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_valign,GtkWidgetP,GtkAlign)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_vexpand,GtkWidgetP,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_vexpand_set,GtkWidgetP,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_set_visible,GtkWidgetP,gboolean)
          SIGFU_CON_RET_PAR1(gtk_widget_should_layout,gboolean,GtkWidgetP)
          #if GTK_MINOR_VERSION < 10
            SIGFU_CON_VOID_PAR1(gtk_widget_show,GtkWidgetP)
          #endif
          /*table_void_GtkWidgetP_constGtkAllocationP_int*/ /*gtk_widget_size_allocate*/
          /*table_void_GtkWidgetP_GtkWidgetP_GtkSnapshotP*/ /*gtk_widget_snapshot_child*/
          #if GTK_MINOR_VERSION < 12
            /*table_gboolean_GtkWidgetP_GtkWidgetP_double_double_doubleP_doubleP*/ /*gtk_widget_translate_coordinates*/
          #endif
          SIGFU_CON_VOID_PAR1(gtk_widget_trigger_tooltip_query,GtkWidgetP)
          SIGFU_CON_VOID_PAR1(gtk_widget_unmap,GtkWidgetP)
          SIGFU_CON_VOID_PAR1(gtk_widget_unparent,GtkWidgetP)
          SIGFU_CON_VOID_PAR1(gtk_widget_unrealize,GtkWidgetP)
          /*table_void_GtkWidgetP_GtkStateFlags*/ /*gtk_widget_unset_state_flags*/
        /* Class methods */
          /*table_void_GtkWidgetClassP_guint_GdkModifierType_GtkShortcutFunc_constcharP_Elipse*/ /*gtk_widget_class_add_binding*/
          /*table_void_GtkWidgetClassP_guint_GdkModifierType_constcharP_constcharP_Elipse*/ /*gtk_widget_class_add_binding_action*/
          /*table_void_GtkWidgetClassP_guint_GdkModifierType_constcharP_constcharP_Elipse*/ /*gtk_widget_class_add_binding_signal*/
          /*table_void_GtkWidgetClassP_GtkShortcutP*/ /*gtk_widget_class_add_shortcut*/
          /*table_void_GtkWidgetClassP_constcharP_GCallback*/ /*gtk_widget_class_bind_template_callback_full*/
          /*table_void_GtkWidgetClassP_constcharP_gboolean_gssize*/ /*gtk_widget_class_bind_template_child_full*/
          /*table_GtkAccessibleRole_GtkWidgetClassP*/ /*gtk_widget_class_get_accessible_role*/
          SIGFU_CON_RET_PAR1(gtk_widget_class_get_activate_signal,guint,GtkWidgetClassP)
          SIGFU_CON_RET_PAR1(gtk_widget_class_get_css_name,constcharP,GtkWidgetClassP)
          //xx table_add(gtk_widget_class_get_layout_manager_type, _GType_GtkWidgetClassP);
          /*table_void_GtkWidgetClassP_constcharP_constcharP_GtkWidgetActionActivateFunc*/ /*gtk_widget_class_install_action*/
          /*table_void_GtkWidgetClassP_constcharP_constcharP*/ /*gtk_widget_class_install_property_action*/
          /*table_gboolean_GtkWidgetClassP_guint_GTypeP_constcharPP_constGVariantTypePP_constcharPP*/ /*gtk_widget_class_query_action*/
          /*table_void_GtkWidgetClassP_GtkAccessibleRole*/ /*gtk_widget_class_set_accessible_role*/
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_class_set_activate_signal,GtkWidgetClassP,guint)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_class_set_activate_signal_from_name,GtkWidgetP,constcharP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_widget_class_set_css_name,GtkWidgetP,constcharP)
          /*table_void_GtkWidgetClassP_GType*/ /*gtk_widget_class_set_layout_manager_type*/
          //xx SIGFU_CON_VOID_(gtk_widget_class_set_template, _GtkWidgetClassP_GBytesP)
          /*table_void_GtkWidgetClassP_constcharP*/ /*gtk_widget_class_set_template_from_resource*/
          /*table_void_GtkWidgetClassP_GtkBuilderScopeP*/ /*gtk_widget_class_set_template_scope*/
        /* Virtual methods */
          // void(* compute_expand)(GtkWidget*,gboolean*,gboolean*);
          // gboolean(* contains)(GtkWidget*,double,double);
          // void(* css_changed)(GtkWidget*,GtkCssStyleChange*);
          // void(* direction_changed)(GtkWidget*,GtkTextDirection);
          // gboolean(* focus)(GtkWidget*,GtkDirectionType);
          // GtkSizeRequestMode(* get_request_mode)(GtkWidget*);
          // gboolean(* grab_focus)(GtkWidget*);
          #if GTK_MINOR_VERSION < 10
            // void(* hide)(GtkWidget*);
          #endif
          // gboolean(* keynav_failed)(GtkWidget*,GtkDirectionType);
          // void(* map)(GtkWidget*);
          // void(* measure)(GtkWidget*,GtkOrientation,int,int*,int*,int*,int*);
          // gboolean(* mnemonic_activate)(GtkWidget*,gboolean);
          // void(* move_focus)(GtkWidget*,GtkDirectionType);
          // gboolean(* query_tooltip)(GtkWidget*,int,int,gboolean,GtkTooltip*);
          // void(* realize)(GtkWidget*);
          // void(* root)(GtkWidget*);
          // void(* set_focus_child)(GtkWidget*,GtkWidget*);
          #if GTK_MINOR_VERSION < 10
            // void(* show)(GtkWidget*);
          #endif
          // void(* size_allocate)(GtkWidget*,int,int,int);
          // void(* snapshot)(GtkWidget*,GtkSnapshot*);
          // void(* state_flags_changed)(GtkWidget*,GtkStateFlags);
          // void(* system_setting_changed)(GtkWidget*,GtkSystemSetting);
          // void(* unmap)(GtkWidget*);
          // void(* unrealize)(GtkWidget*);
          // void(* unroot)(GtkWidget*);
      /* GtkWindow ****************************************************001-199*/
        /* Functions */
          SIGFU_CON_RET(gtk_window_get_default_icon_name,constcharP)
          /*table_GListModelP*/ /*gtk_window_get_toplevels*/ // GListModel* void
          /*table_GListP*/ /*gtk_window_list_toplevels*/ // GList* void
          SIGFU_CON_VOID_PAR1(gtk_window_set_auto_startup_notification,gboolean)
          SIGFU_CON_VOID_PAR1(gtk_window_set_default_icon_name, constcharP)
          SIGFU_CON_VOID_PAR1(gtk_window_set_interactive_debugging, gboolean)
        /* Instance methods */
          SIGFU_CON_VOID_PAR1(gtk_window_close,GtkWindowP)
          SIGFU_CON_VOID_PAR1(gtk_window_destroy,GtkWindowP)
          SIGFU_CON_VOID_PAR1(gtk_window_fullscreen,GtkWindowP)
          /*table_void_GtkWindowP_GdkMonitorP*/ /*gtk_window_fullscreen_on_monitor*/ // void GtkWindow* GdkMonitor*
          /*table_GtkApplication_GtkWindowP*/ /*gtk_window_get_application*/ // GtkApplication* GtkWindow*
          /*table_GtkWidgetP_GtkWindowP*/ /*gtk_window_get_child*/ // GtkWidget* GtkWindow*
          SIGFU_CON_RET_PAR1(gtk_window_get_decorated,gboolean,GtkWindowP)
          //xx SIGFU_CON_VOID_(gtk_window_get_default_size, _GtkWindowP_intP_intP)
          /*table_GtkWidgetP_GtkWindowP*/ /*gtk_window_get_default_widget*/ // GtkWidget* GtkWindow*
          SIGFU_CON_RET_PAR1(gtk_window_get_deletable,gboolean,GtkWindowP)
          SIGFU_CON_RET_PAR1(gtk_window_get_destroy_with_parent,gboolean,GtkWindowP)
          /*table_GtkWidgetP_GtkWindowP*/ /*gtk_window_get_focus*/ // GtkWidget* GtkWindow*
          SIGFU_CON_RET_PAR1(gtk_window_get_focus_visible,gboolean,GtkWindowP)
          #if GTK_MINOR_VERSION >= 20
            table_add(table_GtkWindowGravity_GtkWindowP,"gtk_window_get_gravity",gtk_window_get_gravity);
          #endif
          /*table_GtkWindowGroupP_GtkWindowP*/ /*gtk_window_get_group*/ // GtkWindowGroup* GtkWindow*
          SIGFU_CON_RET_PAR1(gtk_window_get_handle_menubar_accel,gboolean,GtkWindowP)
          SIGFU_CON_RET_PAR1(gtk_window_get_hide_on_close,gboolean,GtkWindowP)
          SIGFU_CON_RET_PAR1(gtk_window_get_icon_name,constcharP,GtkWindowP)
          SIGFU_CON_RET_PAR1(gtk_window_get_mnemonics_visible,gboolean,GtkWindowP)
          SIGFU_CON_RET_PAR1(gtk_window_get_modal,gboolean,GtkWindowP)
          SIGFU_CON_RET_PAR1(gtk_window_get_resizable,gboolean,GtkWindowP)
          SIGFU_CON_RET_PAR1(gtk_window_get_title,constcharP,GtkWindowP)
          /*table_GtkWidgetP_GtkWindowP*/ /*gtk_window_get_titlebar*/ // GtkWidget* GtkWindow*
          /*table_GtkWindowP_GtkWindowP*/ /*gtk_window_get_transient_for*/ // GtkWindow* GtkWindow*
          SIGFU_CON_RET_PAR1(gtk_window_has_group,gboolean,GtkWindowP)
          SIGFU_CON_RET_PAR1(gtk_window_is_active,gboolean,GtkWindowP)
          SIGFU_CON_RET_PAR1(gtk_window_is_fullscreen,gboolean,GtkWindowP)
          SIGFU_CON_RET_PAR1(gtk_window_is_maximized,gboolean,GtkWindowP)
          #if GTK_MINOR_VERSION >= 12
            SIGFU_CON_RET_PAR1(gtk_window_is_suspended,gboolean,GtkWindowP)
          #endif
          SIGFU_CON_VOID_PAR1(gtk_window_maximize,GtkWindowP)
          SIGFU_CON_VOID_PAR1(gtk_window_minimize,GtkWindowP)
          SIGFU_CON_VOID_PAR1(gtk_window_present,GtkWindowP)
          #if GTK_MINOR_VERSION < 14
            SIGFU_CON_VOID_PAR1_PAR2(gtk_window_present_with_time,GtkWindowP,guint32)
          #endif
          /*table_void_GtkWindowP_GtkApplicationP*/ /*gtk_window_set_application*/ // void GtkWindow* GtkApplication*
          /*table_void_GtkWindowP_GtkWidgetP*/ /*gtk_window_set_child*/ // void GtkWindow* GtkWidget*
          SIGFU_CON_VOID_PAR1_PAR2(gtk_window_set_decorated,GtkWindowP,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2_PAR3(gtk_window_set_default_size,GtkWindowP,int,int)
          /*table_void_GtkWindowP_GtkWidgetP*/ /*gtk_window_set_default_widget*/ // void GtkWindow* GtkWidget*
          SIGFU_CON_VOID_PAR1_PAR2(gtk_window_set_deletable,GtkWindowP,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_window_set_destroy_with_parent,GtkWindowP,gboolean)
          /*table_void_GtkWindowP_GdkDisplayP*/ /*gtk_window_set_display*/ // void GtkWindow* GdkDisplay*
          /*table_void_GtkWindowP_GtkWidgetP*/ /*gtk_window_set_focus*/ // void GtkWindow* GtkWidget*
          SIGFU_CON_VOID_PAR1_PAR2(gtk_window_set_focus_visible,GtkWindowP,gboolean)
          #if GTK_MINOR_VERSION >= 20
            table_add(table_void_GtkWindowP_GtkWindowGravity,"gtk_window_set_gravity",gtk_window_set_gravity);
          #endif
          SIGFU_CON_VOID_PAR1_PAR2(gtk_window_set_handle_menubar_accel,GtkWindowP,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_window_set_hide_on_close,GtkWindowP,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_window_set_icon_name,GtkWindowP,constcharP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_window_set_mnemonics_visible,GtkWindowP,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_window_set_modal,GtkWindowP,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_window_set_resizable,GtkWindowP,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_window_set_startup_id,GtkWindowP,constcharP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_window_set_title,GtkWindowP,constcharP)
          /*table_void_GtkWindowP_GtkWidgetP*/ /*gtk_window_set_titlebar*/ // void GtkWindow* GtkWidget*
          /*table_void_GtkWindowP_GtkWindowP*/ /*gtk_window_set_transient_for*/ // void GtkWindow* GtkWindow*
          SIGFU_CON_VOID_PAR1(gtk_window_unfullscreen,GtkWindowP)
          SIGFU_CON_VOID_PAR1(gtk_window_unmaximize,GtkWindowP)
          SIGFU_CON_VOID_PAR1(gtk_window_unminimize,GtkWindowP)
        /* Virtual methods */
          // void activate_default (GtkWindow* window)
          // void activate_focus (GtkWindow* window)
          // gboolean close_request (GtkWindow* window)
          // gboolean enable_debugging (GtkWindow* window, gboolean toggle)
          // void keys_changed (GtkWindow* window)
      /* GtkLabel ***************************************************0201-0299*/
        /* Instance methods */
          // PangoAttrList *gtk_label_get_attributes (GtkLabel *self);
          SIGFU_CON_RET_PAR1(gtk_label_get_current_uri,constcharP,GtkLabelP)
          SIGFU_CON_RET_PAR1(gtk_label_get_ellipsize,PangoEllipsizeMode,GtkLabelP)
          // GMenuModel * gtk_label_get_extra_menu (GtkLabel *self);
          SIGFU_CON_RET_PAR1(gtk_label_get_justify,GtkJustification,GtkLabelP)
          SIGFU_CON_RET_PAR1(gtk_label_get_label,constcharP,GtkLabelP)
          // PangoLayout *gtk_label_get_layout (GtkLabel *self);
          // void gtk_label_get_layout_offsets (GtkLabel *self, int *x, int *y);
          SIGFU_CON_RET_PAR1(gtk_label_get_lines,int,GtkLabelP)
          SIGFU_CON_RET_PAR1(gtk_label_get_max_width_chars,int,GtkLabelP)
          SIGFU_CON_RET_PAR1(gtk_label_get_mnemonic_keyval,guint,GtkLabelP)
          // GtkWidget *gtk_label_get_mnemonic_widget (GtkLabel *self);
          SIGFU_CON_RET_PAR1(gtk_label_get_natural_wrap_mode,GtkNaturalWrapMode,GtkLabelP)
          SIGFU_CON_RET_PAR1(gtk_label_get_selectable,gboolean,GtkLabelP)
          SIGFU_CON_RET_PAR1(gtk_label_get_selection_bounds,gboolean,GtkLabelP)
          SIGFU_CON_RET_PAR1(gtk_label_get_single_line_mode,gboolean,GtkLabelP)
          // PangoTabArray * gtk_label_get_tabs (GtkLabel *self);
          SIGFU_CON_RET_PAR1(gtk_label_get_text,constcharP,GtkLabelP)
          SIGFU_CON_RET_PAR1(gtk_label_get_use_markup,gboolean,GtkLabelP)
          SIGFU_CON_RET_PAR1(gtk_label_get_use_underline,gboolean,GtkLabelP)
          SIGFU_CON_RET_PAR1(gtk_label_get_width_chars,int,GtkLabelP)
          SIGFU_CON_RET_PAR1(gtk_label_get_wrap,gboolean,GtkLabelP)
          SIGFU_CON_RET_PAR1(gtk_label_get_wrap_mode,PangoWrapMode,GtkLabelP)
          SIGFU_CON_RET_PAR1(gtk_label_get_xalign,float,GtkLabelP)
          SIGFU_CON_RET_PAR1(gtk_label_get_yalign,float,GtkLabelP)
          // void gtk_label_select_region (GtkLabel *self, int start_offset, int end_offset);
          // void gtk_label_set_attributes (GtkLabel *self, PangoAttrList *attrs);
          SIGFU_CON_VOID_PAR1_PAR2(gtk_label_set_ellipsize,GtkLabelP,PangoEllipsizeMode)
          // void gtk_label_set_extra_menu (GtkLabel *self, GMenuModel *model);
          SIGFU_CON_VOID_PAR1_PAR2(gtk_label_set_justify,GtkLabelP,GtkJustification)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_label_set_label,GtkLabelP,constcharP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_label_set_lines,GtkLabelP,int)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_label_set_markup,GtkLabelP,constcharP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_label_set_markup_with_mnemonic,GtkLabelP,constcharP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_label_set_max_width_chars,GtkLabelP,int)
          // void gtk_label_set_mnemonic_widget (GtkLabel *self, GtkWidget *widget);
          SIGFU_CON_VOID_PAR1_PAR2(gtk_label_set_natural_wrap_mode,GtkLabelP,GtkNaturalWrapMode)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_label_set_selectable,GtkLabelP,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_label_set_single_line_mode,GtkLabelP,gboolean)
          // void gtk_label_set_tabs (GtkLabel *self, PangoTabArray *tabs);
          SIGFU_CON_VOID_PAR1_PAR2(gtk_label_set_text,GtkLabelP,constcharP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_label_set_text_with_mnemonic,GtkLabelP,constcharP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_label_set_use_markup,GtkLabelP,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_label_set_use_underline,GtkLabelP,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_label_set_width_chars,GtkLabelP,int)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_label_set_wrap,GtkLabelP,gboolean)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_label_set_wrap_mode,GtkLabelP,PangoWrapMode)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_label_set_xalign,GtkLabelP,float)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_label_set_yalign,GtkLabelP,float)
      /* GtkTextView ************************************************0301-0399*/
        /* Instance methods */
          // GType gtk_text_view_get_type (void) G_GNUC_CONST;
          // GtkWidget * gtk_text_view_new (void);
          // GtkWidget * gtk_text_view_new_with_buffer (GtkTextBuffer *buffer);
          // void gtk_text_view_set_buffer (GtkTextView *text_view,GtkTextBuffer *buffer);
          SIGFU_CON_RET_PAR1(gtk_text_view_get_buffer,GtkTextBufferP,GtkTextViewP)
          // gboolean gtk_text_view_scroll_to_iter (GtkTextView *text_view,GtkTextIter *iter,double within_margin,gboolean use_align,double xalign,double yalign);
          // void gtk_text_view_scroll_to_mark (GtkTextView *text_view,GtkTextMark *mark,double within_margin,gboolean use_align,double xalign,double yalign);
          // void gtk_text_view_scroll_mark_onscreen (GtkTextView *text_view,GtkTextMark *mark);
          // gboolean gtk_text_view_move_mark_onscreen (GtkTextView *text_view,GtkTextMark *mark);
          SIGFU_CON_RET_PAR1(gtk_text_view_place_cursor_onscreen,gboolean,GtkTextViewP)
          // void gtk_text_view_get_visible_rect (GtkTextView *text_view,GdkRectangle *visible_rect);
          SIGFU_CON_VOID_PAR1_PAR2(gtk_text_view_set_cursor_visible,GtkTextViewP,gboolean)
          SIGFU_CON_RET_PAR1(gtk_text_view_get_cursor_visible,gboolean,GtkTextViewP)
          // void gtk_text_view_reset_cursor_blink (GtkTextView *text_view);
          // void gtk_text_view_get_cursor_locations (GtkTextView *text_view,const GtkTextIter *iter,GdkRectangle *strong,GdkRectangle *weak);
          // void gtk_text_view_get_iter_location (GtkTextView *text_view,const GtkTextIter *iter,GdkRectangle *location);
          // gboolean gtk_text_view_get_iter_at_location (GtkTextView *text_view,GtkTextIter *iter,int x,int y);
          // gboolean gtk_text_view_get_iter_at_position (GtkTextView *text_view,GtkTextIter *iter,int *trailing,int x,int y);
          // void gtk_text_view_get_line_yrange (GtkTextView *text_view,const GtkTextIter *iter,int *y,int *height);
          // void gtk_text_view_get_line_at_y (GtkTextView *text_view,GtkTextIter *target_iter,int y,int *line_top);
          // void gtk_text_view_buffer_to_window_coords (GtkTextView *text_view,GtkTextWindowType win,int buffer_x,int buffer_y,int *window_x,int *window_y);
          // void gtk_text_view_window_to_buffer_coords (GtkTextView *text_view,GtkTextWindowType win,int window_x,int window_y,int *buffer_x,int *buffer_y);
          // gboolean gtk_text_view_forward_display_line (GtkTextView *text_view,GtkTextIter *iter);
          // gboolean gtk_text_view_backward_display_line (GtkTextView *text_view,GtkTextIter *iter);
          // gboolean gtk_text_view_forward_display_line_end (GtkTextView *text_view,GtkTextIter *iter);
          // gboolean gtk_text_view_backward_display_line_start (GtkTextView *text_view,GtkTextIter *iter);
          // gboolean gtk_text_view_starts_display_line (GtkTextView *text_view,const GtkTextIter *iter);
          // gboolean gtk_text_view_move_visually (GtkTextView *text_view,GtkTextIter *iter,int count);
          // gboolean gtk_text_view_im_context_filter_keypress (GtkTextView *text_view,GdkEvent *event);
          // void gtk_text_view_reset_im_context (GtkTextView *text_view);
          // GtkWidget *gtk_text_view_get_gutter (GtkTextView *text_view,GtkTextWindowType win);
          // void gtk_text_view_set_gutter (GtkTextView *text_view,GtkTextWindowType win,GtkWidget *widget);
          // void gtk_text_view_add_child_at_anchor (GtkTextView *text_view,GtkWidget *child,GtkTextChildAnchor *anchor);
          // void gtk_text_view_add_overlay (GtkTextView *text_view,GtkWidget *child,int xpos,int ypos);
          // void gtk_text_view_move_overlay (GtkTextView *text_view,GtkWidget *child,int xpos,int ypos);
          // void gtk_text_view_remove (GtkTextView *text_view,GtkWidget *child);
          // void gtk_text_view_set_wrap_mode (GtkTextView *text_view,GtkWrapMode wrap_mode);
          // GtkWrapMode gtk_text_view_get_wrap_mode (GtkTextView *text_view);
          SIGFU_CON_VOID_PAR1_PAR2(gtk_text_view_set_editable,GtkTextViewP,gboolean)
          SIGFU_CON_RET_PAR1(gtk_text_view_get_editable,gboolean,GtkTextViewP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_text_view_set_overwrite,GtkTextViewP,gboolean)
          SIGFU_CON_RET_PAR1(gtk_text_view_get_overwrite,gboolean,GtkTextViewP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_text_view_set_accepts_tab,GtkTextViewP,gboolean)
          SIGFU_CON_RET_PAR1(gtk_text_view_get_accepts_tab,gboolean,GtkTextViewP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_text_view_set_pixels_above_lines,GtkTextViewP,int)
          SIGFU_CON_RET_PAR1(gtk_text_view_get_pixels_above_lines,int,GtkTextViewP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_text_view_set_pixels_below_lines,GtkTextViewP,int)
          SIGFU_CON_RET_PAR1(gtk_text_view_get_pixels_below_lines,int,GtkTextViewP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_text_view_set_pixels_inside_wrap,GtkTextViewP,int)
          SIGFU_CON_RET_PAR1(gtk_text_view_get_pixels_inside_wrap,int,GtkTextViewP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_text_view_set_justification,GtkTextViewP,GtkJustification)
          SIGFU_CON_RET_PAR1(gtk_text_view_get_justification,GtkJustification,GtkTextViewP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_text_view_set_left_margin,GtkTextViewP,int)
          SIGFU_CON_RET_PAR1(gtk_text_view_get_left_margin,int,GtkTextViewP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_text_view_set_right_margin,GtkTextViewP,int)
          SIGFU_CON_RET_PAR1(gtk_text_view_get_right_margin,int,GtkTextViewP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_text_view_set_top_margin,GtkTextViewP,int)
          SIGFU_CON_RET_PAR1(gtk_text_view_get_top_margin,int,GtkTextViewP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_text_view_set_bottom_margin,GtkTextViewP,int)
          SIGFU_CON_RET_PAR1(gtk_text_view_get_bottom_margin,int,GtkTextViewP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_text_view_set_indent,GtkTextViewP,int)
          SIGFU_CON_RET_PAR1(gtk_text_view_get_indent,int,GtkTextViewP)
          // void gtk_text_view_set_tabs (GtkTextView *text_view,PangoTabArray *tabs);
          // PangoTabArray* gtk_text_view_get_tabs (GtkTextView *text_view);
          // void gtk_text_view_set_input_purpose (GtkTextView *text_view,GtkInputPurpose purpose);
          // GtkInputPurpose gtk_text_view_get_input_purpose (GtkTextView *text_view);
          // void gtk_text_view_set_input_hints (GtkTextView *text_view,GtkInputHints hints);
          // GtkInputHints gtk_text_view_get_input_hints (GtkTextView *text_view);
          SIGFU_CON_VOID_PAR1_PAR2(gtk_text_view_set_monospace,GtkTextViewP,gboolean)
          SIGFU_CON_RET_PAR1(gtk_text_view_get_monospace,gboolean,GtkTextViewP)
          // void gtk_text_view_set_extra_menu (GtkTextView *text_view,GMenuModel *model);
          // GMenuModel * gtk_text_view_get_extra_menu (GtkTextView *text_view);
          // PangoContext *gtk_text_view_get_rtl_context (GtkTextView *text_view);
          // PangoContext *gtk_text_view_get_ltr_context (GtkTextView *text_view);
      /* GtkTextBuffer **********************************************0401-0499*/
        /* Instance methods */
          // GType gtk_text_buffer_get_type(void) G_GNUC_CONST;
          // GtkTextBuffer*gtk_text_buffer_new(GtkTextTagTable*table);
          SIGFU_CON_RET_PAR1(gtk_text_buffer_get_line_count,int,GtkTextBufferP)
          SIGFU_CON_RET_PAR1(gtk_text_buffer_get_char_count,int,GtkTextBufferP)
          // GtkTextTagTable* gtk_text_buffer_get_tag_table(GtkTextBuffer*buffer);
          SIGFU_CON_VOID_PAR1_PAR2_PAR3(gtk_text_buffer_set_text,GtkTextBufferP,constcharP,int)
          // void gtk_text_buffer_insert(GtkTextBuffer*buffer,GtkTextIter*iter,const char*text,int len);
          // void gtk_text_buffer_insert_at_cursor(GtkTextBuffer*buffer,const char*text,int len);
          // gboolean gtk_text_buffer_insert_interactive(GtkTextBuffer*buffer,GtkTextIter*iter,const char*text,int len,gboolean default_editable);
          // gboolean gtk_text_buffer_insert_interactive_at_cursor(GtkTextBuffer*buffer,const char*text,int len,gboolean default_editable);
          // void gtk_text_buffer_insert_range(GtkTextBuffer*buffer,GtkTextIter*iter,const GtkTextIter*start,const GtkTextIter*end);
          // gboolean gtk_text_buffer_insert_range_interactive(GtkTextBuffer*buffer,GtkTextIter*iter,const GtkTextIter*start,const GtkTextIter*end,gboolean default_editable);
          // void gtk_text_buffer_insert_with_tags(GtkTextBuffer*buffer,GtkTextIter*iter,const char*text,int len,GtkTextTag*first_tag,...) G_GNUC_NULL_TERMINATED;
          // void gtk_text_buffer_insert_with_tags_by_name(GtkTextBuffer*buffer,GtkTextIter*iter,const char*text,int len,const char*first_tag_name,...) G_GNUC_NULL_TERMINATED;
          // void gtk_text_buffer_insert_markup(GtkTextBuffer*buffer,GtkTextIter*iter,const char*markup,int len);
          // void gtk_text_buffer_delete(GtkTextBuffer*buffer,GtkTextIter*start,GtkTextIter*end);
          // gboolean gtk_text_buffer_delete_interactive(GtkTextBuffer*buffer,GtkTextIter*start_iter,GtkTextIter*end_iter,gboolean default_editable);
          // gboolean gtk_text_buffer_backspace(GtkTextBuffer*buffer,GtkTextIter*iter,gboolean interactive,gboolean default_editable);
          SIGFU_CON_VOID_PAR1_PAR2_PAR3(gtk_text_buffer_set_text,GtkTextBufferP,constcharP,int)
          SIGFU_CON_RET_PAR1_PAR2_PAR3_PAR4(gtk_text_buffer_get_text,charP,GtkTextBufferP,constGtkTextIterP,constGtkTextIterP,gboolean)
          // char*gtk_text_buffer_get_slice(GtkTextBuffer*buffer,const GtkTextIter*start,const GtkTextIter*end,gboolean include_hidden_chars);
          // void gtk_text_buffer_insert_paintable(GtkTextBuffer*buffer,GtkTextIter*iter,GdkPaintable*paintable);
          // void gtk_text_buffer_insert_child_anchor(GtkTextBuffer*buffer,GtkTextIter*iter,GtkTextChildAnchor*anchor);
          // GtkTextChildAnchor*gtk_text_buffer_create_child_anchor(GtkTextBuffer*buffer,GtkTextIter*iter);
          #if GTK_MINOR_VERSION >= 16
            // void gtk_text_buffer_add_mark(GtkTextBuffer*buffer,GtkTextMark*mark,const GtkTextIter*where);
          #endif
          // GtkTextMark*gtk_text_buffer_create_mark(GtkTextBuffer*buffer,const char*mark_name,const GtkTextIter*where,gboolean left_gravity);
          // void gtk_text_buffer_move_mark(GtkTextBuffer*buffer,GtkTextMark*mark,const GtkTextIter*where);
          // void gtk_text_buffer_delete_mark(GtkTextBuffer*buffer,GtkTextMark*mark);
          // GtkTextMark* gtk_text_buffer_get_mark(GtkTextBuffer*buffer,const char*name);
          // void gtk_text_buffer_move_mark_by_name(GtkTextBuffer*buffer,const char*name,const GtkTextIter*where);
          // void gtk_text_buffer_delete_mark_by_name(GtkTextBuffer*buffer,const char*name);
          // GtkTextMark* gtk_text_buffer_get_insert(GtkTextBuffer*buffer);
          // GtkTextMark* gtk_text_buffer_get_selection_bound(GtkTextBuffer*buffer);
          // void gtk_text_buffer_place_cursor(GtkTextBuffer*buffer,const GtkTextIter*where);
          // void gtk_text_buffer_select_range(GtkTextBuffer*buffer,const GtkTextIter*ins,const GtkTextIter*bound);
          // void gtk_text_buffer_apply_tag(GtkTextBuffer*buffer,GtkTextTag*tag,const GtkTextIter*start,const GtkTextIter*end);
          // void gtk_text_buffer_remove_tag(GtkTextBuffer*buffer,GtkTextTag*tag,const GtkTextIter*start,const GtkTextIter*end);
          // void gtk_text_buffer_apply_tag_by_name(GtkTextBuffer*buffer,const char*name,const GtkTextIter*start,const GtkTextIter*end);
          // void gtk_text_buffer_remove_tag_by_name(GtkTextBuffer*buffer,const char*name,const GtkTextIter*start,const GtkTextIter*end);
          // void gtk_text_buffer_remove_all_tags(GtkTextBuffer*buffer,const GtkTextIter*start,const GtkTextIter*end);
          // GtkTextTag*gtk_text_buffer_create_tag(GtkTextBuffer*buffer,const char*tag_name,const char*first_property_name,...);
          // gboolean gtk_text_buffer_get_iter_at_line_offset(GtkTextBuffer*buffer,GtkTextIter*iter,int line_number,int char_offset);
          // gboolean gtk_text_buffer_get_iter_at_line_index(GtkTextBuffer*buffer,GtkTextIter*iter,int line_number,int byte_index);
          // void gtk_text_buffer_get_iter_at_offset(GtkTextBuffer*buffer,GtkTextIter*iter,int char_offset);
          // gboolean gtk_text_buffer_get_iter_at_line(GtkTextBuffer*buffer,GtkTextIter*iter,int line_number);
          SIGFU_CON_VOID_PAR1_PAR2(gtk_text_buffer_get_start_iter,GtkTextBufferP,GtkTextIterP)
          SIGFU_CON_VOID_PAR1_PAR2(gtk_text_buffer_get_end_iter,GtkTextBufferP,GtkTextIterP)
          // void gtk_text_buffer_get_bounds(GtkTextBuffer*buffer,GtkTextIter*start,GtkTextIter*end);
          // void gtk_text_buffer_get_iter_at_mark(GtkTextBuffer*buffer,GtkTextIter*iter,GtkTextMark*mark);
          // void gtk_text_buffer_get_iter_at_child_anchor(GtkTextBuffer*buffer,GtkTextIter*iter,GtkTextChildAnchor*anchor);
          SIGFU_CON_RET_PAR1(gtk_text_buffer_get_modified,gboolean,GtkTextBufferP)
          // void gtk_text_buffer_set_modified(GtkTextBuffer*buffer,gboolean setting);
          SIGFU_CON_RET_PAR1(gtk_text_buffer_get_has_selection,gboolean,GtkTextBufferP)
          // void gtk_text_buffer_add_selection_clipboard(GtkTextBuffer*buffer,GdkClipboard*clipboard);
          // void gtk_text_buffer_remove_selection_clipboard(GtkTextBuffer*buffer,GdkClipboard*clipboard);
          // void gtk_text_buffer_cut_clipboard(GtkTextBuffer*buffer,GdkClipboard*clipboard,gboolean default_editable);
          // void gtk_text_buffer_copy_clipboard(GtkTextBuffer*buffer,GdkClipboard*clipboard);
          // void gtk_text_buffer_paste_clipboard(GtkTextBuffer*buffer,GdkClipboard*clipboard,GtkTextIter*override_location,gboolean default_editable);
          // gboolean gtk_text_buffer_get_selection_bounds(GtkTextBuffer*buffer,GtkTextIter*start,GtkTextIter*end);
          // gboolean gtk_text_buffer_delete_selection(GtkTextBuffer*buffer,gboolean interactive,gboolean default_editable);
          // GdkContentProvider*gtk_text_buffer_get_selection_content(GtkTextBuffer*buffer);
          SIGFU_CON_RET_PAR1(gtk_text_buffer_get_can_undo,gboolean,GtkTextBufferP)
          SIGFU_CON_RET_PAR1(gtk_text_buffer_get_can_redo,gboolean,GtkTextBufferP)
          SIGFU_CON_RET_PAR1(gtk_text_buffer_get_enable_undo,gboolean,GtkTextBufferP)
          // void gtk_text_buffer_set_enable_undo(GtkTextBuffer*buffer,gboolean enable_undo);
          SIGFU_CON_RET_PAR1(gtk_text_buffer_get_max_undo_levels,guint,GtkTextBufferP)
          // void gtk_text_buffer_set_max_undo_levels(GtkTextBuffer*buffer,guint max_undo_levels);
          // void gtk_text_buffer_undo(GtkTextBuffer*buffer);
          // void gtk_text_buffer_redo(GtkTextBuffer*buffer);
          // void gtk_text_buffer_begin_irreversible_action(GtkTextBuffer*buffer);
          // void gtk_text_buffer_end_irreversible_action(GtkTextBuffer*buffer);
          // void gtk_text_buffer_begin_user_action(GtkTextBuffer*buffer);
          // void gtk_text_buffer_end_user_action(GtkTextBuffer*buffer);
    //
    while(TRUE) {
        fgets(input, 1024, pargs->fpin);
        input[strlen(input)-1]='\0';
        command = input;
        if(command[0]=='|') {strend='|';command++;} else strend=' ';
        trim(command);
        arg1 = strchr(command, strend);
        if(arg1 != NULL) { *arg1++ = '\0';trim(arg1); arg2 = strchr(arg1, strend);
        } else { arg2 = NULL; }
        if(arg2 != NULL) {*arg2++ = '\0';trim(arg2); arg3 = strchr(arg2, strend);
        } else { arg3 = NULL; }
        if(arg3 != NULL) {*arg3++ = '\0';trim(arg3); arg4 = strchr(arg3, strend);
        } else { arg4 = NULL; }
        if(arg4 != NULL) {*arg4++ = '\0';trim(arg4);}
        if(VERBOSE) fprintf(stderr, "\n\nCALLBACK:> %s %s %s %s %s %s %s\n", command, arg1 == NULL ? "NULL" : arg1, arg2 == NULL ? "NULL" : arg2, arg3 == NULL ? "NULL" : arg3, arg4 == NULL ? "NULL" : arg4, arg5 == NULL ? "NULL" : arg5, arg6 == NULL ? "NULL" : arg6);
        
        // Kommando in Tabelle suchen
          if(NULL != (sig_vu=table_get_sig_fu(command))) {
              ((sigfu)sig_vu)(pargs->builder, command, arg1,arg2,arg3,arg4,arg5,arg6);
          } else
        //
        // Konstruktoren und Memory
          if(!strcmp(command, "newGtkTextIter")) {
              //if(DEBUG) fprintf(stderr,"CONSTRUCT newGtkTextIter => ");
              GtkWidget *iter = malloc(sizeof(GtkTextIter));
              addp(iter);
              //if(DEBUG) fprintf(stderr," %p (%s)\n",iter,__func__);
              fprintf(pargs->fpout, "%p\n", iter);
              fflush(pargs->fpout);
          } else
          if(!strcmp(command, "free")) {
              //if(DEBUG) fprintf(stderr,"FREE  (%s)\n",__func__);
              free_ptrs();
          } else
        //
        // Legacy Aufrufe
          if(!strcmp(command, "gtk_editable_get_text")) {
              GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(pargs->builder, arg1));
              const char* mtext=gtk_editable_get_text(GTK_EDITABLE(widget));
              fprintf(pargs->fpout, "%s\n", mtext);
              fflush(pargs->fpout);
          } else {
              fprintf(pargs->fpout, "NICHT_IMPLEMENTIERT:%s\n", command);
              fflush(pargs->fpout);
          }

    }

    fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ENDE %s()... Ich dachte, da kommen wir nie hin \n", __func__);
    pthread_exit(NULL);
}
void wrap_add_signals(char *filename, _args* pargs) {
    // wrap_add_signals von: https://github.com/abecadel/gtkwrap
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
    if(DEBUG) fprintf(stderr, "ENDE %s()...\n", __func__);
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
    if(VERBOSE) fprintf(stderr, "Adding css file %s ...\n", pargs->css_file);
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
static void app_do/*app_activate,app_open*/(GtkApplication *app, gpointer *user_data) {
    if(DEBUG) fprintf(stderr, "START %s()...\n", __func__);
    _args *pargs = (_args *)user_data;
    GtkWidget *win;
    win = GTK_WIDGET(gtk_builder_get_object(pargs->builder, pargs->win_id));
    gtk_window_present(GTK_WINDOW(win));
    if(pargs->name_out && !pargs->thread)
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
    if(DEBUG) fprintf(stderr, "HANDLER %s()... ruft nur app_do auf\n", __func__);
    /* Dieser Funktion endet mit dem Anzeigen des Dialogs.
     * Danach befinden wir uns in g_application_run */
    app_do(app, user_data);
  }
static void app_open(GtkApplication *app, GFile ** files, gint n_files, gchar *hint, gpointer *user_data) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()... ruft nur app_do auf\n", __func__);
    /* Dieser Funktion endet mit dem Anzeigen des Dialogs.
     * Danach befinden wir uns in g_application_run */
    app_do(app, user_data);
  }
static void app_shutdown(GtkApplication *app, gpointer *user_data) {
    if(DEBUG) fprintf(stderr, "HANDLER %s()...\n", __func__);
    if(VERBOSE) fprintf(stderr, "Cleaning ...\n");
    _args *pargs = (_args *)user_data;
    if(pargs->name_out) {
        void* res;
        int s = pthread_cancel(pargs->thread);

        s = pthread_join(pargs->thread, &res);
        if(s != 0) fprintf(stderr, "!!!!ERROR pthread_join ...\n");
        if(res == PTHREAD_CANCELED) {
            if(DEBUG) fprintf(stderr, "    %s(): thread was canceled\n", __func__);
            pargs->thread=0;
        } else
            fprintf(stderr, "!!!!ERROR %s(): thread wasn't canceled (shouldn't happen!)\n", __func__);
    }
    g_object_unref(pargs->builder);pargs->builder=NULL;
    char **tmp = pargs->SIGNALS;
    while(*tmp)
        free(*tmp++);
    free(pargs->SIGNALS);pargs->SIGNALS=NULL;
    table_free();
    free_ptrs();
    fclose(pargs->fpin);pargs->fpin=NULL;
    fflush(pargs->fpout);
    fclose(pargs->fpout);pargs->fpout=NULL;
    unlink(pargs->name_out);
    unlink(pargs->name_in);
    if(DEBUG) fprintf(stderr, "ENDE %s()...\n", __func__);
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
                if(VERBOSE) fprintf(stderr, "-%c\n",opt);
                break;
            case 'v':
                VERBOSE = 1;
                if(VERBOSE) fprintf(stderr, "-%c\n",opt);
                break;
            case 'h':
                if(VERBOSE) fprintf(stderr, "-%c\n",opt);
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
                pargs->name_out  = optarg;
                break;
            case 'i':
                if(VERBOSE) fprintf(stderr, "-%c: %s\n", opt, optarg);
                if(optarg[0]=='-') fprintf(stderr, "ACHTUNG: -%c: %s\n", opt, optarg);
                pargs->name_in  = optarg;
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
//
int main(int argc, char **argv) {
    int stat;
    int idx;
    //GtkApplication *app;
  	g_autoptr(AdwApplication) app = NULL;
    _args args;
    args.app_name  = argv[0];
    args.ui_file   = NULL;
    if (access("styles.css", F_OK) == 0) {
        args.css_file  = "styles.css";
    } else {
        args.css_file  = NULL;
    }
    args.name_out  = NULL;
    args.name_in   = NULL;
    args.win_id    = "window1";
    args.builder   = NULL;
    args.SIGNALS   = NULL;
    args.thread    = 0;
    args.fpout     = NULL;
    args.fpin      = NULL;

    read_opts(&args, &argc, &argv);
    if(DEBUG) fprintf(stderr, "MAIN Optionen eingelesen ...\n");
    if(!args.ui_file)
        help(args.app_name);
    if((args.name_out && !args.name_in) || (args.name_in && !args.name_out))
        help(args.app_name);
    if(VERBOSE) for(idx=0; idx < argc; idx++)fprintf(stderr, "%i: %s\n", idx, argv[idx]);
    if(VERBOSE) fprintf(stderr, "UI-Datei: %s; TOP-WINDOW: %s\n", args.ui_file, args.win_id);

    //app = gtk_application_new(APP_ID, G_APPLICATION_HANDLES_OPEN);
	  app = adw_application_new (APP_ID, /*G_APPLICATION_HANDLES_COMMAND_LINE |*/ G_APPLICATION_NON_UNIQUE);
	  //g_signal_connect (app, "handle-local-options", G_CALLBACK(local_options_cb), results);
	  //g_signal_connect (app, "command-line", G_CALLBACK(command_line_cb), results);


    if(DEBUG) fprintf(stderr, "MAIN Neue Applikation ...\n");
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
    if(DEBUG) fprintf(stderr, "MAIN Alle Application Handler verbunden ...\n");

    stat = g_application_run(G_APPLICATION(app), argc, argv);

    if(DEBUG) fprintf(stderr, "ENDE App läuft nicht mehr ...\n");
    g_object_unref(app);
    return stat;
}

