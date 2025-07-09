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
//
typedef struct {char* name; void* fup;} FU;
struct FUS {int size; FU* fus; struct FUS* next;};
typedef struct FUS FUS;
FUS* first_table=NULL;
FUS* table_new() {
    static FUS* last=NULL;
    FUS* t = malloc(sizeof(FUS));
    t->size = 0;t->fus = NULL;t->next=NULL;
    if(!first_table) first_table=t;
    if(last) last->next=t;
    last=t;
    //if(DEBUG) fprintf(stderr, "%s() !\n", __func__);
    return t;
  }
void table_free(FUS* t) {
    //if(DEBUG) fprintf(stderr, "%s() !\n", __func__);
    for(int i = 0; i < t->size; i++)free((t->fus + i)->name);
    free(t->fus); free(t);
  }
void table_add(FUS* t, char* name, void* fup) {
    if(!t->size) t->fus=malloc(sizeof(FU));
    else t->fus=realloc(t->fus, sizeof(FU)*(t->size+1));
    (t->fus + t->size)->name = malloc(strlen(name) + 1);
    strcpy((t->fus + t->size)->name, name);
    (t->fus + t->size)->fup = fup;
    t->size++;
  }
void* table_get(FUS* t, char* name) {
    for (int i = 0; i < t->size; i++)
        if (strcmp((t->fus + i)->name, name)==0)return(t->fus + i)->fup;
    return NULL;
  }
void tables_free() {
    FUS* curr=NULL;
    while(first_table) {
        curr=first_table;
        first_table=first_table->next;
        table_free(curr);curr=NULL;
    }
  }
//
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
    char *widget_id = NULL;
    char *operanda = NULL;
    char *operandb = NULL;
    void *vu;

    void trim(char *s) {
        size_t len = strspn(s, " ");
        if (len > 0) memmove(s, s + len, strlen(s) - len + 1);
    }
    // tables alphabetisch
      FUS* table_charPP_GtkWidgetP=table_new();
      typedef char**(*sig_charPP_GtkWidgetP)(GtkWidget*);
      FUS* table_constcairo_font_options_tP_GtkWidgetP=table_new();
      typedef const cairo_font_options_t*(*sig_constcairo_font_options_tP_GtkWidgetP)(GtkWidget*);
      FUS* table_constcharP=table_new();                      // const char* void
      typedef const char*(*sig_constcharP)();          
      FUS* table_constcharP_GtkWidgetClassP=table_new();
      typedef const char*(*sig_constcharP_GtkWidgetClassP)(GtkWidgetClass*);
      FUS* table_constcharP_GtkWidgetP=table_new();
      typedef const char*(*sig_constcharP_GtkWidgetP)(GtkWidget*);
      FUS* table_constcharP_GtkWindowP=table_new();           // const char* GtkWindow*
      typedef const char* (*sig_constcharP_GtkWindowP)(GtkWindow*);
      FUS* table_double_GtkWidgetP=table_new();
      typedef double(*sig_double_GtkWidgetP)(GtkWidget*);
      FUS* table_gboolean_GtkWidgetClassP_guint_GTypeP_constcharPP_constGVariantTypePP_constcharPP=table_new();
      typedef gboolean(*sig_gboolean_GtkWidgetClassP_guint_GTypeP_constcharPP_constGVariantTypePP_constcharPP)(GtkWidgetClass*,guint,GType*,const char**,const GVariantType**,const char**);
      FUS* table_gboolean_GtkWidgetP_constcharP=table_new();
      typedef gboolean(*sig_gboolean_GtkWidgetP_constcharP)(GtkWidget*,const char*);
      FUS* table_gboolean_GtkWidgetP_constcharP_constcharP_Elipse=table_new();
      typedef gboolean(*sig_gboolean_GtkWidgetP_constcharP_constcharP_Elipse)(GtkWidget*,const char*,const char*,...);
      FUS* table_gboolean_GtkWidgetP_constcharP_GVariantP=table_new();
      typedef gboolean(*sig_gboolean_GtkWidgetP_constcharP_GVariantP)(GtkWidget*,const char*,GVariant*);
      FUS* table_gboolean_GtkWidgetP_double_double=table_new();
      typedef gboolean(*_gboolean_GtkWidgetP_double_double)(GtkWidget*,double,double);
      FUS* table_gboolean_GtkWidgetP_gboolean=table_new();
      typedef gboolean(*sig_gboolean_GtkWidgetP_gboolean)(GtkWidget*,gboolean);
      FUS* table_gboolean_GtkWidgetP_GtkDirectionType=table_new();
      typedef gboolean(*sig_gboolean_GtkWidgetP_GtkDirectionType)(GtkWidget*,GtkDirectionType);
      FUS* table_gboolean_GtkWidgetP_GtkOrientation=table_new();
      typedef gboolean(*sig_gboolean_GtkWidgetP_GtkOrientation)(GtkWidget*,GtkOrientation);
      FUS* table_gboolean_GtkWidgetP_GtkWidgetP_constgraphene_point_tP_graphene_point_tP=table_new();
      typedef gboolean(*sig_gboolean_GtkWidgetP_GtkWidgetP_constgraphene_point_tP_graphene_point_tP)(GtkWidget*,GtkWidget*,const graphene_point_t*,graphene_point_t*);
      FUS* table_gboolean_GtkWidgetP_GtkWidgetP_double_double_doubleP_doubleP=table_new();
      typedef gboolean(*sig_gboolean_GtkWidgetP_GtkWidgetP_double_double_doubleP_doubleP)(GtkWidget*,GtkWidget*,double,double,double*,double*);
      FUS* table_gboolean_GtkWidgetP_GtkWidgetP_graphene_matrix_tP=table_new();
      typedef gboolean(*sig_gboolean_GtkWidgetP_GtkWidgetP_graphene_matrix_tP)(GtkWidget*,GtkWidget*,graphene_matrix_t*);
      FUS* table_gboolean_GtkWidgetP_GtkWidgetP=table_new();
      typedef gboolean(*sig_gboolean_GtkWidgetP_GtkWidgetP)(GtkWidget*,GtkWidget*);
      FUS* table_gboolean_GtkWidgetP_GtkWidgetP_graphene_rect_tP=table_new();
      typedef gboolean(*sig_gboolean_GtkWidgetP_GtkWidgetP_graphene_rect_tP)(GtkWidget*,GtkWidget*,graphene_rect_t*);
      FUS* table_gboolean_GtkWidgetP_int_int_int_int=table_new();
      typedef gboolean(*sig_gboolean_GtkWidgetP_int_int_int_int)(GtkWidget*,int,int,int,int);
      FUS* table_gboolean_GtkWidgetP=table_new();
      typedef gboolean(*sig_gboolean_GtkWidgetP)(GtkWidget*);
      FUS* table_gboolean_GtkWindowP=table_new();             // gboolean GtkWindow*
      typedef gboolean(*sig_gboolean_GtkWindowP)(GtkWindow*);  
      FUS* table_GdkClipboardP_GtkWidgetP=table_new();
      typedef GdkClipboard*(*sig_GdkClipboardP_GtkWidgetP)(GtkWidget*);
      FUS* table_GdkCursorP_GtkWidgetP=table_new();
      typedef GdkCursor*(*sig_GdkCursorP_GtkWidgetP)(GtkWidget*);
      FUS* table_GdkDisplayP_GtkWidgetP=table_new();
      typedef GdkDisplay*(*sig_GdkDisplayP_GtkWidgetP)(GtkWidget*);
      FUS* table_GdkFrameClockP_GtkWidgetP=table_new();
      typedef GdkFrameClock*(*sig_GdkFrameClockP_GtkWidgetP)(GtkWidget*);
      FUS* table_GListP=table_new();                          // GList* void
      typedef GList*(*sig_GListP)();                   
      FUS* table_GListP_GtkWidgetP=table_new();
      typedef GList*(*sig_GListP_GtkWidgetP)(GtkWidget*);
      FUS* table_GListModelP=table_new();                     // GListModel* void
      typedef GListModel*(*sig_GListModelP)();         
      FUS* table_GListModelP_GtkWidgetP=table_new();
      typedef GListModel*(*sig_GListModelP_GtkWidgetP)(GtkWidget*);
      FUS* table_GObjectP_GtkWidgetP_GType_constcharP=table_new();
      typedef GObject*(*sig_GObjectP_GtkWidgetP_GType_constcharP)(GtkWidget*,GType,const char*);
      FUS* table_GtkAccessibleRole_GtkWidgetClassP=table_new();
      typedef GtkAccessibleRole(*sig_GtkAccessibleRole_GtkWidgetClassP)(GtkWidgetClass*);
      FUS* table_GtkAlign_GtkWidgetP=table_new();
      typedef GtkAlign(*sig_GtkAlign_GtkWidgetP)(GtkWidget*);
      FUS* table_GtkApplication_GtkWindowP=table_new();       // GtkApplication* GtkWindow*
      typedef GtkApplication*(*sig_GtkApplication_GtkWindowP)(GtkWindow*);  
      FUS* table_GtkLayoutManagerP_GtkWidgetP=table_new();
      typedef GtkLayoutManager*(*sig_GtkLayoutManagerP_GtkWidgetP)(GtkWidget*);
      FUS* table_GtkNativeP_GtkWidgetP=table_new();
      typedef GtkNative*(*sig_GtkNativeP_GtkWidgetP)(GtkWidget*);
      FUS* table_GtkOverflow_GtkWidgetP=table_new();
      typedef GtkOverflow(*sig_GtkOverflow_GtkWidgetP)(GtkWidget*);
      FUS* table_GtkRootP_GtkWidgetP=table_new();
      typedef GtkRoot*(*sig_GtkRootP_GtkWidgetP)(GtkWidget*);
      FUS* table_GtkSettingsP_GtkWidgetP=table_new();
      typedef GtkSettings*(*sig_GtkSettingsP_GtkWidgetP)(GtkWidget*);
      FUS* table_GtkSizeRequestMode_GtkWidgetP=table_new();
      typedef GtkSizeRequestMode(*sig_GtkSizeRequestMode_GtkWidgetP)(GtkWidget*);
      FUS* table_GtkStateFlags_GtkWidgetP=table_new();
      typedef GtkStateFlags(*sig_GtkStateFlags_GtkWidgetP)(GtkWidget*);
      FUS* table_GtkStyleContextP_GtkWidgetP=table_new();
      typedef GtkStyleContext*(*sig_GtkStyleContextP_GtkWidgetP)(GtkWidget*);
      FUS* table_GtkTextDirection_GtkWidgetP=table_new();
      typedef GtkTextDirection(*sig_GtkTextDirection_GtkWidgetP)(GtkWidget*);
      FUS* table_GtkTextDirection_void=table_new();
      typedef GtkTextDirection(*sig_GtkTextDirection_void)(void);
      FUS* table_GtkWidgetP_GtkWidgetP=table_new();
      typedef GtkWidget*(*sig_GtkWidgetP_GtkWidgetP)(GtkWidget*);
      FUS* table_GtkWidgetP_GtkWidgetP_double_double_GtkPickFlags=table_new();
      typedef GtkWidget*(*sig_GtkWidgetP_GtkWidgetP_double_double_GtkPickFlags)(GtkWidget*,double,double,GtkPickFlags);
      FUS* table_GtkWidgetP_GtkWidgetP_GType=table_new();
      typedef GtkWidget*(*sig_GtkWidgetP_GtkWidgetP_GType)(GtkWidget*,GType);
      FUS* table_GtkWidgetP_GtkWindowP=table_new();           // GtkWidget* GtkWindow*
      typedef GtkWidget*(*sig_GtkWidgetP_GtkWindowP)(GtkWindow*);  
      #if GTK_MINOR_VERSION >= 20
        FUS* table_GtkWindowGravity_GtkWindowP=table_new();     // GtkWindowGravity GtkWindow*
        typedef GtkWindowGravity (*sig_GtkWindowGravity_GtkWindowP)(GtkWindow*);
      #endif
      FUS* table_GtkWindowGroupP_GtkWindowP=table_new();      // GtkWindowGroup* GtkWindow*
      typedef GtkWindowGroup (*sig_GtkWindowGroupP_GtkWindowP)(GtkWindow*);
      FUS* table_GtkWindowP_GtkWindowP=table_new();           // GtkWindow* GtkWindow*
      typedef GtkWindow* (*sig_GtkWindowP_GtkWindowP)(GtkWindow*);
      FUS* table_GType_GtkWidgetClassP=table_new();
      typedef GType(*sig_GType_GtkWidgetClassP)(GtkWidgetClass*);
      FUS* table_guint_GtkWidgetClassP=table_new();
      typedef guint(*sig_guint_GtkWidgetClassP)(GtkWidgetClass*);
      FUS* table_guint_GtkWidgetP_GtkTickCallback_gpointer_GDestroyNotify=table_new();
      typedef guint(*sig_guint_GtkWidgetP_GtkTickCallback_gpointer_GDestroyNotify)(GtkWidget*,GtkTickCallback,gpointer,GDestroyNotify);
      FUS* table_int_GtkWidgetP=table_new();
      typedef int(*sig_int_GtkWidgetP)(GtkWidget*);
      FUS* table_int_GtkWidgetP_GtkOrientation=table_new();
      typedef int(*sig_int_GtkWidgetP_GtkOrientation)(GtkWidget*,GtkOrientation);
      FUS* table_PangoContextP_GtkWidgetP=table_new();
      typedef PangoContext*(*sig_PangoContextP_GtkWidgetP)(GtkWidget*);
      FUS* table_PangoFontMapP_GtkWidgetP=table_new();
      typedef PangoFontMap*(*sig_PangoFontMapP_GtkWidgetP)(GtkWidget*);
      FUS* table_PangoLayoutP_GtkWidgetP_constcharP=table_new();
      typedef PangoLayout*(*sig_PangoLayoutP_GtkWidgetP_constcharP)(GtkWidget*,const char*);
      FUS* table_void_constcharP=table_new();                 // void const char * 
      typedef void(*sig_void_constcharP)(const char*); 
      FUS* table_void_gboolean=table_new();                   // void gboolean
      typedef void(*sig_void_gboolean)(gboolean);      
      FUS* table_void_GtkTextDirection=table_new();
      typedef void(*sig_void_GtkTextDirection)(GtkTextDirection);
      FUS* table_void_GtkWidgetClassP_constcharP=table_new();
      typedef void(*sig_void_GtkWidgetClassP_constcharP)(GtkWidgetClass*,const char*);
      FUS* table_void_GtkWidgetClassP_constcharP_constcharP=table_new();
      typedef void(*sig_void_GtkWidgetClassP_constcharP_constcharP)(GtkWidgetClass*,const char*,const char*);
      FUS* table_void_GtkWidgetClassP_constcharP_constcharP_GtkWidgetActionActivateFunc=table_new();
      typedef void(*sig_void_GtkWidgetClassP_constcharP_constcharP_GtkWidgetActionActivateFunc)(GtkWidgetClass*,const char*,const char*,GtkWidgetActionActivateFunc);
      FUS* table_void_GtkWidgetClassP_constcharP_gboolean_gssize=table_new();
      typedef void(*sig_void_GtkWidgetClassP_constcharP_gboolean_gssize)(GtkWidgetClass*,const char*,gboolean,gssize);
      FUS* table_void_GtkWidgetClassP_constcharP_GCallback=table_new();
      typedef void(*sig_void_GtkWidgetClassP_constcharP_GCallback)(GtkWidgetClass*,const char*,GCallback);
      FUS* table_void_GtkWidgetClassP_GBytesP=table_new();
      typedef void(*sig_void_GtkWidgetClassP_GBytesP)(GtkWidgetClass*,GBytes*);
      FUS* table_void_GtkWidgetClassP_GtkAccessibleRole=table_new();
      typedef void(*sig_void_GtkWidgetClassP_GtkAccessibleRole)(GtkWidgetClass*,GtkAccessibleRole);
      FUS* table_void_GtkWidgetClassP_GtkBuilderScopeP=table_new();
      typedef void(*sig_void_GtkWidgetClassP_GtkBuilderScopeP)(GtkWidgetClass*,GtkBuilderScope*);
      FUS* table_void_GtkWidgetClassP_GtkShortcutP=table_new();
      typedef void(*sig_void_GtkWidgetClassP_GtkShortcutP)(GtkWidgetClass*,GtkShortcut*);
      FUS* table_void_GtkWidgetClassP_GType=table_new();
      typedef void(*sig_void_GtkWidgetClassP_GType)(GtkWidgetClass*,GType);
      FUS* table_void_GtkWidgetClassP_guint=table_new();
      typedef void(*sig_void_GtkWidgetClassP_guint)(GtkWidgetClass*,guint);
      FUS* table_void_GtkWidgetClassP_guint_GdkModifierType_constcharP_constcharP_Elipse=table_new();
      typedef void(*sig_void_GtkWidgetClassP_guint_GdkModifierType_constcharP_constcharP_Elipse)(GtkWidgetClass*,guint,GdkModifierType,const char*,const char*,...);
      FUS* table_void_GtkWidgetClassP_guint_GdkModifierType_GtkShortcutFunc_constcharP_Elipse=table_new();
      typedef void(*sig_void_GtkWidgetClassP_guint_GdkModifierType_GtkShortcutFunc_constcharP_Elipse)(GtkWidgetClass*,guint,GdkModifierType,GtkShortcutFunc,const char*,...);
      FUS* table_void_GtkWidgetP=table_new();
      typedef void(*sig_void_GtkWidgetP)(GtkWidget*);
      FUS* table_void_GtkWidgetP_constcairo_font_options_tP=table_new();
      typedef void(*sig_void_GtkWidgetP_constcairo_font_options_tP)(GtkWidget*,const cairo_font_options_t*);
      FUS* table_void_GtkWidgetP_constcharP=table_new();
      typedef void(*sig_void_GtkWidgetP_constcharP)(GtkWidget*,const char*);
      FUS* table_void_GtkWidgetP_constcharP_GActionGroupP=table_new();
      typedef void(*sig_void_GtkWidgetP_constcharP_GActionGroupP)(GtkWidget*,const char*,GActionGroup*);
      FUS* table_void_GtkWidgetP_constcharP_gboolean=table_new();
      typedef void(*sig_void_GtkWidgetP_constcharP_gboolean)(GtkWidget*,const char*,gboolean);
      FUS* table_void_GtkWidgetP_constcharPP=table_new();
      typedef void(*sig_void_GtkWidgetP_constcharPP)(GtkWidget*,const char**);
      FUS* table_void_GtkWidgetP_constGtkAllocationP_int=table_new();
      typedef void(*sig_void_GtkWidgetP_constGtkAllocationP_int)(GtkWidget*,const GtkAllocation*,int);
      FUS* table_void_GtkWidgetP_double=table_new();
      typedef void(*sig_void_GtkWidgetP_double)(GtkWidget*,double);
      FUS* table_void_GtkWidgetP_gboolean=table_new();
      typedef void(*sig_void_GtkWidgetP_gboolean)(GtkWidget*,gboolean);
      FUS* table_void_GtkWidgetP_GdkCursorP=table_new();
      typedef void(*sig_void_GtkWidgetP_GdkCursorP)(GtkWidget*,GdkCursor*);
      FUS* table_void_GtkWidgetP_GdkRGBA=table_new();
      typedef void(*sig_void_GtkWidgetP_GdkRGBA)(GtkWidget*,GdkRGBA);
      FUS* table_void_GtkWidgetP_GtkAlign=table_new();
      typedef void(*sig_void_GtkWidgetP_GtkAlign)(GtkWidget*,GtkAlign);
      FUS* table_void_GtkWidgetP_GtkAllocationP=table_new();
      typedef void(*sig_void_GtkWidgetP_GtkAllocationP)(GtkWidget*,GtkAllocation*);
      FUS* table_void_GtkWidgetP_GtkEventControllerP=table_new();
      typedef void(*sig_void_GtkWidgetP_GtkEventControllerP)(GtkWidget*,GtkEventController*);
      FUS* table_void_GtkWidgetP_GtkLayoutManagerP=table_new();
      typedef void(*sig_void_GtkWidgetP_GtkLayoutManagerP)(GtkWidget*,GtkLayoutManager*);
      FUS* table_void_GtkWidgetP_GtkOrientation_int_intP_intP_intP_intP=table_new();
      typedef void(*sig_void_GtkWidgetP_GtkOrientation_int_intP_intP_intP_intP)(GtkWidget*,GtkOrientation,int,int*,int*,int*,int*);
      FUS* table_void_GtkWidgetP_GtkOverflow=table_new();
      typedef void(*sig_void_GtkWidgetP_GtkOverflow)(GtkWidget*,GtkOverflow);
      FUS* table_void_GtkWidgetP_GtkRequisitionP_GtkRequisitionP=table_new();
      typedef void(*sig_void_GtkWidgetP_GtkRequisitionP_GtkRequisitionP)(GtkWidget*,GtkRequisition*,GtkRequisition*);
      FUS* table_void_GtkWidgetP_GtkStateFlags_gboolean=table_new();
      typedef void(*sig_void_GtkWidgetP_GtkStateFlags_gboolean)(GtkWidget*,GtkStateFlags,gboolean);
      FUS* table_void_GtkWidgetP_GtkStateFlags=table_new();
      typedef void(*sig_void_GtkWidgetP_GtkStateFlags)(GtkWidget*,GtkStateFlags);
      FUS* table_void_GtkWidgetP_GtkTextDirection=table_new();
      typedef void(*sig_void_GtkWidgetP_GtkTextDirection)(GtkWidget*,GtkTextDirection);
      FUS* table_void_GtkWidgetP_GtkWidgetP=table_new();
      typedef void(*sig_void_GtkWidgetP_GtkWidgetP)(GtkWidget*,GtkWidget*);
      FUS* table_void_GtkWidgetP_GtkWidgetP_GtkSnapshotP=table_new();
      typedef void(*sig_void_GtkWidgetP_GtkWidgetP_GtkSnapshotP)(GtkWidget*,GtkWidget*,GtkSnapshot*);
      FUS* table_void_GtkWidgetP_GtkWidgetP_GtkWidgetP=table_new();
      typedef void(*sig_void_GtkWidgetP_GtkWidgetP_GtkWidgetP)(GtkWidget*,GtkWidget*,GtkWidget*);
      FUS* table_void_GtkWidgetP_GType=table_new();
      typedef void(*sig_void_GtkWidgetP_GType)(GtkWidget*,GType);
      FUS* table_void_GtkWidgetP_guint=table_new();
      typedef void(*sig_void_GtkWidgetP_guint)(GtkWidget*,guint);
      FUS* table_void_GtkWidgetP_int=table_new();
      typedef void(*sig_void_GtkWidgetP_int)(GtkWidget*,int);
      FUS* table_void_GtkWidgetP_int_int=table_new();
      typedef void(*sig_void_GtkWidgetP_int_int)(GtkWidget*,int,int);
      FUS* table_void_GtkWidgetP_int_int_int_GskTransformP=table_new();
      typedef void(*sig_void_GtkWidgetP_int_int_int_GskTransformP)(GtkWidget*,int,int,int,GskTransform*);
      FUS* table_void_GtkWidgetP_intP_intP=table_new();
      typedef void(*sig_void_GtkWidgetP_intP_intP)(GtkWidget*,int*,int*);
      FUS* table_void_GtkWidgetP_PangoFontMapP=table_new();
      typedef void(*sig_void_GtkWidgetP_PangoFontMapP)(GtkWidget*,PangoFontMap*);
      FUS* table_void_GtkWindowP=table_new();                 // void GtkWindow*
      typedef void(*sig_void_GtkWindowP)(GtkWindow*);  
      FUS* table_void_GtkWindowP_constcharP=table_new();      // void GtkWindow* const char*
      typedef void (*sig_void_GtkWindowP_constcharP)(GtkWindow*, const char *);
      FUS* table_void_GtkWindowP_gboolean=table_new();        // void GtkWindow* gboolean
      typedef void (*sig_void_GtkWindowP_gboolean)(GtkWindow*, gboolean);
      FUS* table_void_GtkWindowP_GdkDisplayP=table_new();     // void GtkWindow* GdkDisplay*
      typedef void (*sig_void_GtkWindowP_GdkDisplayP)(GtkWindow*, GdkDisplay*);
      FUS* table_void_GtkWindowP_GdkMonitorP=table_new();     // void GtkWindow* GdkMonitor*
      typedef void(*sig_void_GtkWindowP_GdkMonitorP)(GtkWindow*, GdkMonitor*);  
      FUS* table_void_GtkWindowP_GtkApplicationP=table_new(); // void GtkWindow* GtkApplication*
      typedef void (*sig_void_GtkWindowP_GtkApplicationP)(GtkWindow*, GtkApplication*);
      FUS* table_void_GtkWindowP_GtkWidgetP=table_new();      // void GtkWindow* GtkWidget*
      typedef void (*sig_void_GtkWindowP_GtkWidgetP)(GtkWindow*,GtkWidget);
      #if GTK_MINOR_VERSION >= 20
        FUS* table_void_GtkWindowP_GtkWindowGravity=table_new();// void GtkWindow* GtkWindowGravity
        typedef void (*sig_void_GtkWindowP_GtkWindowGravity)(GtkWindow*, GtkWindowGravity);
      #endif
      FUS* table_void_GtkWindowP_GtkWindowP=table_new();      // void GtkWindow* GtkWindow*
      typedef void(*sig_void_GtkWindowP_GtkWindowP)(GtkWindow*,GtkWindow*);
      FUS* table_void_GtkWindowP_guint32=table_new();         // void GtkWindow* guint32
      typedef void (*sig_void_GtkWindowP_guint32)(GtkWindow*, guint32);
      FUS* table_void_GtkWindowP_int_int=table_new();         // void GtkWindow* int int
      typedef void (*sig_void_GtkWindowP_int_int)(GtkWindow*, int, int);
      FUS* table_void_GtkWindowP_intP_intP=table_new();       // void GtkWindow* int* int*
      typedef void(*sig_void_GtkWindowP_intP_intP)(GtkWindow*, int*, int*);  
     // gtk Funktionen in der Reihenfolge der doc
      /* GtkWidget ***********************************************************/
        /* Functions */
          table_add(table_GtkTextDirection_void, "gtk_widget_get_default_direction",gtk_widget_get_default_direction);
          table_add(table_void_GtkTextDirection, "gtk_widget_set_default_direction",gtk_widget_set_default_direction);
        /* Instance methods */
          table_add(table_void_GtkWidgetP_constcharP_gboolean, "gtk_widget_action_set_enabled",gtk_widget_action_set_enabled);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_activate",gtk_widget_activate);
          /*table_gboolean_GtkWidgetP_constcharP_constcharP_Elipse*/ /*gtk_widget_activate_action*/
          /*table_gboolean_GtkWidgetP_constcharP_GVariantP*/ /*gtk_widget_activate_action_variant*/
          table_add(table_void_GtkWidgetP, "gtk_widget_activate_default",gtk_widget_activate_default);
          /*table_void_GtkWidgetP_GtkEventControllerP*/ /*gtk_widget_add_controller*/
          table_add(table_void_GtkWidgetP_constcharP, "gtk_widget_add_css_class",gtk_widget_add_css_class);
          table_add(table_void_GtkWidgetP_GtkWidgetP, "gtk_widget_add_mnemonic_label",gtk_widget_add_mnemonic_label);
          /*table_guint_GtkWidgetP_GtkTickCallback_gpointer_GDestroyNotify*/ /*gtk_widget_add_tick_callback*/
          /*table_void_GtkWidgetP_int_int_int_GskTransformP*/ /*gtk_widget_allocate*/
          table_add(table_gboolean_GtkWidgetP_GtkDirectionType, "gtk_widget_child_focus",gtk_widget_child_focus);
          /*table_gboolean_GtkWidgetP_GtkWidgetP_graphene_rect_tP*/ /*gtk_widget_compute_bounds*/
          /*table_gboolean_GtkWidgetP_GtkOrientation*/ /*gtk_widget_compute_expand*/
          /*table_gboolean_GtkWidgetP_GtkWidgetP_constgraphene_point_tP_graphene_point_tP*/ /*gtk_widget_compute_point*/
          /*table_gboolean_GtkWidgetP_GtkWidgetP_graphene_matrix_tP*/ /*gtk_widget_compute_transform*/
          table_add(table_gboolean_GtkWidgetP_double_double, "gtk_widget_contains",gtk_widget_contains);
          /*table_PangoContextP_GtkWidgetP*/ /*gtk_widget_create_pango_context*/
          /*table_PangoLayoutP_GtkWidgetP_constcharP*/ /*gtk_widget_create_pango_layout*/
          /*table_void_GtkWidgetP_GType*/ /*gtk_widget_dispose_template*/
          /*table_gboolean_GtkWidgetP_int_int_int_int*/ /*gtk_drag_check_threshold*/
          table_add(table_void_GtkWidgetP, "gtk_widget_error_bell",gtk_widget_error_bell);
          #if GTK_MINOR_VERSION < 12
            table_add(table_int_GtkWidgetP, "gtk_widget_get_allocated_baseline",gtk_widget_get_allocated_baseline);
            table_add(table_int_GtkWidgetP, "gtk_widget_get_allocated_height",gtk_widget_get_allocated_height);
            table_add(table_int_GtkWidgetP, "gtk_widget_get_allocated_width",gtk_widget_get_allocated_width);
            table_add(table_void_GtkWidgetP_GtkAllocationP, "gtk_widget_get_allocation",gtk_widget_get_allocation);
          #endif
          /*table_GtkWidgetP_GtkWidgetP_GType*/ /*gtk_widget_get_ancestor*/
          #if GTK_MINOR_VERSION >= 12
            table_add(table_int_GtkWidgetP, "gtk_widget_get_baseline",gtk_widget_get_baseline);
          #endif
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_get_can_focus",gtk_widget_get_can_focus);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_get_can_target",gtk_widget_get_can_target);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_get_child_visible",gtk_widget_get_child_visible);
          /*table_GdkClipboardP_GtkWidgetP*/ /*gtk_widget_get_clipboard*/
          #if GTK_MINOR_VERSION >= 10
            table_add(table_void_GtkWidgetP_GdkRGBA, "gtk_widget_get_color",gtk_widget_get_color);
          #endif
          table_add(table_charPP_GtkWidgetP, "gtk_widget_get_css_classes",gtk_widget_get_css_classes);
          table_add(table_constcharP_GtkWidgetP, "gtk_widget_get_css_name",gtk_widget_get_css_name);
          /*table_GdkCursorP_GtkWidgetP*/ /*gtk_widget_get_cursor*/
          /*table_GtkTextDirection_GtkWidgetP*/ /*gtk_widget_get_direction*/
          /*table_GdkDisplayP_GtkWidgetP*/ /*gtk_widget_get_display*/
          /*table_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_get_first_child*/
          /*table_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_get_focus_child*/
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_get_focus_on_click",gtk_widget_get_focus_on_click);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_get_focusable",gtk_widget_get_focusable);
          /*table_PangoFontMapP_GtkWidgetP*/ /*gtk_widget_get_font_map*/
          #if GTK_MINOR_VERSION < 16
            /*table_constcairo_font_options_tP_GtkWidgetP*/ /*gtk_widget_get_font_options*/
          #endif
          /*table_GdkFrameClockP_GtkWidgetP*/ /*gtk_widget_get_frame_clock*/
          table_add(table_GtkAlign_GtkWidgetP, "gtk_widget_get_halign",gtk_widget_get_halign);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_get_has_tooltip",gtk_widget_get_has_tooltip);
          table_add(table_int_GtkWidgetP, "gtk_widget_get_height",gtk_widget_get_height);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_get_hexpand",gtk_widget_get_hexpand);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_get_hexpand_set",gtk_widget_get_hexpand_set);
          /*table_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_get_last_child*/
          /*table_GtkLayoutManagerP_GtkWidgetP*/ /*gtk_widget_get_layout_manager*/
          #if GTK_MINOR_VERSION >= 18
            /*table_gboolean_GtkWidgetP*/ /*gtk_widget_get_limit_events*/
          #endif
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_get_mapped",gtk_widget_get_mapped);
          table_add(table_int_GtkWidgetP, "gtk_widget_get_margin_bottom",gtk_widget_get_margin_bottom);
          table_add(table_int_GtkWidgetP, "gtk_widget_get_margin_end",gtk_widget_get_margin_end);
          table_add(table_int_GtkWidgetP, "gtk_widget_get_margin_start",gtk_widget_get_margin_start);
          table_add(table_int_GtkWidgetP, "gtk_widget_get_margin_top",gtk_widget_get_margin_top);
          table_add(table_constcharP_GtkWidgetP, "gtk_widget_get_name",gtk_widget_get_name);
          /*table_GtkNativeP_GtkWidgetP*/ /*gtk_widget_get_native*/
          /*table_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_get_next_sibling*/
          table_add(table_double_GtkWidgetP, "gtk_widget_get_opacity",gtk_widget_get_opacity);
          /*table_GtkOverflow_GtkWidgetP*/ /*gtk_widget_get_overflow*/
          /*table_PangoContextP_GtkWidgetP*/ /*gtk_widget_get_pango_context*/
          /*table_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_get_parent*/
          /*table_void_GtkWidgetP_GtkRequisitionP_GtkRequisitionP*/ /*gtk_widget_get_preferred_size*/
          /*table_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_get_prev_sibling*/
          /*table_GdkClipboardP_GtkWidgetP*/ /*gtk_widget_get_primary_clipboard*/
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_get_realized",gtk_widget_get_realized);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_get_receives_default",gtk_widget_get_receives_default);
          /*table_GtkSizeRequestMode_GtkWidgetP*/ /*gtk_widget_get_request_mode*/
          /*table_GtkRootP_GtkWidgetP*/ /*gtk_widget_get_root*/
          table_add(table_int_GtkWidgetP, "gtk_widget_get_scale_factor",gtk_widget_get_scale_factor);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_get_sensitive",gtk_widget_get_sensitive);
          /*table_GtkSettingsP_GtkWidgetP*/ /*gtk_widget_get_settings*/
          /*table_int_GtkWidgetP_GtkOrientation*/ /*gtk_widget_get_size*/
          table_add(table_void_GtkWidgetP_intP_intP, "gtk_widget_get_size_request",gtk_widget_get_size_request);
          table_add(table_GtkStateFlags_GtkWidgetP, "gtk_widget_get_state_flags",gtk_widget_get_state_flags);
          #if GTK_MINOR_VERSION < 10
            /*table_GtkStyleContextP_GtkWidgetP*/ /*gtk_widget_get_style_context*/
          #endif
          table_add(table_GObjectP_GtkWidgetP_GType_constcharP, "gtk_widget_get_template_child",gtk_widget_get_template_child);
          table_add(table_constcharP_GtkWidgetP, "gtk_widget_get_tooltip_markup",gtk_widget_get_tooltip_markup);
          table_add(table_constcharP_GtkWidgetP, "gtk_widget_get_tooltip_text",gtk_widget_get_tooltip_text);
          table_add(table_GtkAlign_GtkWidgetP, "gtk_widget_get_valign",gtk_widget_get_valign);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_get_vexpand",gtk_widget_get_vexpand);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_get_vexpand_set",gtk_widget_get_vexpand_set);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_get_visible",gtk_widget_get_visible);
          table_add(table_int_GtkWidgetP, "gtk_widget_get_width",gtk_widget_get_width);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_grab_focus",gtk_widget_grab_focus);
          table_add(table_gboolean_GtkWidgetP_constcharP, "gtk_widget_has_css_class",gtk_widget_has_css_class);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_has_default",gtk_widget_has_default);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_has_focus",gtk_widget_has_focus);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_has_visible_focus",gtk_widget_has_visible_focus);
          #if GTK_MINOR_VERSION < 10
            table_add(table_void_GtkWidgetP, "gtk_widget_hide",gtk_widget_hide);
          #endif
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_in_destruction",gtk_widget_in_destruction);
          table_add(table_void_GtkWidgetP, "gtk_widget_init_template",gtk_widget_init_template);
          /*table_void_GtkWidgetP_constcharP_GActionGroupP*/ /*gtk_widget_insert_action_group*/
          /*table_void_GtkWidgetP_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_insert_after*/
          /*table_void_GtkWidgetP_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_insert_before*/
          /*table_gboolean_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_is_ancestor*/
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_is_drawable",gtk_widget_is_drawable);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_is_focus",gtk_widget_is_focus);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_is_sensitive",gtk_widget_is_sensitive);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_is_visible",gtk_widget_is_visible);
          table_add(table_gboolean_GtkWidgetP_GtkDirectionType, "gtk_widget_keynav_failed",gtk_widget_keynav_failed);
          /*table_GListP_GtkWidgetP*/ /*gtk_widget_list_mnemonic_labels*/
          table_add(table_void_GtkWidgetP, "gtk_widget_map",gtk_widget_map);
          /*table_void_GtkWidgetP_GtkOrientation_int_intP_intP_intP_intP*/ /*gtk_widget_measure*/
          /*table_gboolean_GtkWidgetP_gboolean*/ /*gtk_widget_mnemonic_activate*/
          /*table_GListModelP_GtkWidgetP*/ /*gtk_widget_observe_children*/
          /*table_GListModelP_GtkWidgetP*/ /*gtk_widget_observe_controllers*/
          /*table_GtkWidgetP_GtkWidgetP_double_double_GtkPickFlags*/ /*gtk_widget_pick*/
          table_add(table_void_GtkWidgetP, "gtk_widget_queue_allocate",gtk_widget_queue_allocate);
          table_add(table_void_GtkWidgetP, "gtk_widget_queue_draw",gtk_widget_queue_draw);
          table_add(table_void_GtkWidgetP, "gtk_widget_queue_resize",gtk_widget_queue_resize);
          table_add(table_void_GtkWidgetP, "gtk_widget_realize",gtk_widget_realize);
          /*table_void_GtkWidgetP_GtkEventControllerP*/ /*gtk_widget_remove_controller*/
          table_add(table_void_GtkWidgetP_constcharP, "gtk_widget_remove_css_class",gtk_widget_remove_css_class);
          /*table_void_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_remove_mnemonic_label*/
          table_add(table_void_GtkWidgetP_guint, "gtk_widget_remove_tick_callback",gtk_widget_remove_tick_callback);
          table_add(table_void_GtkWidgetP_gboolean, "gtk_widget_set_can_focus",gtk_widget_set_can_focus);
          table_add(table_void_GtkWidgetP_gboolean, "gtk_widget_set_can_target",gtk_widget_set_can_target);
          table_add(table_void_GtkWidgetP_gboolean, "gtk_widget_set_child_visible",gtk_widget_set_child_visible);
          table_add(table_void_GtkWidgetP_constcharPP, "gtk_widget_set_css_classes",gtk_widget_set_css_classes);
          /*table_void_GtkWidgetP_GdkCursorP*/ /*gtk_widget_set_cursor*/
          table_add(table_void_GtkWidgetP_constcharP, "gtk_widget_set_cursor_from_name",gtk_widget_set_cursor_from_name);
          /*table_void_GtkWidgetP_GtkTextDirection*/ /*gtk_widget_set_direction*/
          table_add(table_void_GtkWidgetP_gboolean, "gtk_widget_set_focusable",gtk_widget_set_focusable);
          /*table_void_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_set_focus_child*/
          table_add(table_void_GtkWidgetP_gboolean, "gtk_widget_set_focus_on_click",gtk_widget_set_focus_on_click);
          /*table_void_GtkWidgetP_PangoFontMapP*/ /*gtk_widget_set_font_map*/
          #if GTK_MINOR_VERSION < 16
            /*table_void_GtkWidgetP_constcairo_font_options_tP*/ /*gtk_widget_set_font_options*/
          #endif
          table_add(table_void_GtkWidgetP_GtkAlign, "gtk_widget_set_halign",gtk_widget_set_halign);
          table_add(table_void_GtkWidgetP_gboolean, "gtk_widget_set_has_tooltip",gtk_widget_set_has_tooltip);
          table_add(table_void_GtkWidgetP_gboolean, "gtk_widget_set_hexpand",gtk_widget_set_hexpand);
          table_add(table_void_GtkWidgetP_gboolean, "gtk_widget_set_hexpand_set",gtk_widget_set_hexpand_set);
          /*table_void_GtkWidgetP_GtkLayoutManagerP*/ /*gtk_widget_set_layout_manager*/
          #if GTK_MINOR_VERSION >= 18
            /*table_void_GtkWidgetP_gboolean*/ /*gtk_widget_set_limit_events*/
          #endif
          table_add(table_void_GtkWidgetP_int, "gtk_widget_set_margin_bottom",gtk_widget_set_margin_bottom);
          table_add(table_void_GtkWidgetP_int, "gtk_widget_set_margin_end",gtk_widget_set_margin_end);
          table_add(table_void_GtkWidgetP_int, "gtk_widget_set_margin_start",gtk_widget_set_margin_start);
          table_add(table_void_GtkWidgetP_int, "gtk_widget_set_margin_top",gtk_widget_set_margin_top);
          table_add(table_void_GtkWidgetP_constcharP, "gtk_widget_set_name",gtk_widget_set_name);
          table_add(table_void_GtkWidgetP_double, "gtk_widget_set_opacity",gtk_widget_set_opacity);
          table_add(table_void_GtkWidgetP_GtkOverflow, "gtk_widget_set_overflow",gtk_widget_set_overflow);
          /*table_void_GtkWidgetP_GtkWidgetP*/ /*gtk_widget_set_parent*/
          table_add(table_void_GtkWidgetP_gboolean, "gtk_widget_set_receives_default",gtk_widget_set_receives_default);
          table_add(table_void_GtkWidgetP_gboolean, "gtk_widget_set_sensitive",gtk_widget_set_sensitive);
          table_add(table_void_GtkWidgetP_int_int, "gtk_widget_set_size_request",gtk_widget_set_size_request);
          table_add(table_void_GtkWidgetP_GtkStateFlags_gboolean, "gtk_widget_set_state_flags",gtk_widget_set_state_flags);
          table_add(table_void_GtkWidgetP_constcharP, "gtk_widget_set_tooltip_markup",gtk_widget_set_tooltip_markup);
          table_add(table_void_GtkWidgetP_constcharP, "gtk_widget_set_tooltip_text",gtk_widget_set_tooltip_text);
          table_add(table_void_GtkWidgetP_GtkAlign, "gtk_widget_set_valign",gtk_widget_set_valign);
          table_add(table_void_GtkWidgetP_gboolean, "gtk_widget_set_vexpand",gtk_widget_set_vexpand);
          table_add(table_void_GtkWidgetP_gboolean, "gtk_widget_set_vexpand_set",gtk_widget_set_vexpand_set);
          table_add(table_void_GtkWidgetP_gboolean, "gtk_widget_set_visible",gtk_widget_set_visible);
          table_add(table_gboolean_GtkWidgetP, "gtk_widget_should_layout",gtk_widget_should_layout);
          #if GTK_MINOR_VERSION < 10
            table_add(table_void_GtkWidgetP, "gtk_widget_show",gtk_widget_show);
          #endif
          /*table_void_GtkWidgetP_constGtkAllocationP_int*/ /*gtk_widget_size_allocate*/
          /*table_void_GtkWidgetP_GtkWidgetP_GtkSnapshotP*/ /*gtk_widget_snapshot_child*/
          #if GTK_MINOR_VERSION < 12
            /*table_gboolean_GtkWidgetP_GtkWidgetP_double_double_doubleP_doubleP*/ /*gtk_widget_translate_coordinates*/
          #endif
          table_add(table_void_GtkWidgetP, "gtk_widget_trigger_tooltip_query",gtk_widget_trigger_tooltip_query);
          table_add(table_void_GtkWidgetP, "gtk_widget_unmap",gtk_widget_unmap);
          table_add(table_void_GtkWidgetP, "gtk_widget_unparent",gtk_widget_unparent);
          table_add(table_void_GtkWidgetP, "gtk_widget_unrealize",gtk_widget_unrealize);
          /*table_void_GtkWidgetP_GtkStateFlags*/ /*gtk_widget_unset_state_flags*/
        /* Class methods */
          /*table_void_GtkWidgetClassP_guint_GdkModifierType_GtkShortcutFunc_constcharP_Elipse*/ /*gtk_widget_class_add_binding*/
          /*table_void_GtkWidgetClassP_guint_GdkModifierType_constcharP_constcharP_Elipse*/ /*gtk_widget_class_add_binding_action*/
          /*table_void_GtkWidgetClassP_guint_GdkModifierType_constcharP_constcharP_Elipse*/ /*gtk_widget_class_add_binding_signal*/
          /*table_void_GtkWidgetClassP_GtkShortcutP*/ /*gtk_widget_class_add_shortcut*/
          /*table_void_GtkWidgetClassP_constcharP_GCallback*/ /*gtk_widget_class_bind_template_callback_full*/
          /*table_void_GtkWidgetClassP_constcharP_gboolean_gssize*/ /*gtk_widget_class_bind_template_child_full*/
          /*table_GtkAccessibleRole_GtkWidgetClassP*/ /*gtk_widget_class_get_accessible_role*/
          table_add(table_guint_GtkWidgetClassP, "gtk_widget_class_get_activate_signal",gtk_widget_class_get_activate_signal);
          table_add(table_constcharP_GtkWidgetClassP, "gtk_widget_class_get_css_name",gtk_widget_class_get_css_name);
          /*table_GType_GtkWidgetClassP*/ /*gtk_widget_class_get_layout_manager_type*/
          /*table_void_GtkWidgetClassP_constcharP_constcharP_GtkWidgetActionActivateFunc*/ /*gtk_widget_class_install_action*/
          /*table_void_GtkWidgetClassP_constcharP_constcharP*/ /*gtk_widget_class_install_property_action*/
          /*table_gboolean_GtkWidgetClassP_guint_GTypeP_constcharPP_constGVariantTypePP_constcharPP*/ /*gtk_widget_class_query_action*/
          /*table_void_GtkWidgetClassP_GtkAccessibleRole*/ /*gtk_widget_class_set_accessible_role*/
          table_add(table_void_GtkWidgetClassP_guint, "gtk_widget_class_set_activate_signal",gtk_widget_class_set_activate_signal);
          table_add(table_void_GtkWidgetClassP_constcharP, "gtk_widget_class_set_activate_signal_from_name",gtk_widget_class_set_activate_signal_from_name);
          table_add(table_void_GtkWidgetClassP_constcharP, "gtk_widget_class_set_css_name",gtk_widget_class_set_css_name);
          /*table_void_GtkWidgetClassP_GType*/ /*gtk_widget_class_set_layout_manager_type*/
          table_add(table_void_GtkWidgetClassP_GBytesP, "gtk_widget_class_set_template",gtk_widget_class_set_template);
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
      /* GtkWindow **********************************************************/
        /* Functions */
          table_add(table_constcharP,"gtk_window_get_default_icon_name",gtk_window_get_default_icon_name); // const char* void
          /*table_GListModelP*/ /*gtk_window_get_toplevels*/ // GListModel* void
          /*table_GListP*/ /*gtk_window_list_toplevels*/ // GList* void
          table_add(table_void_gboolean,"gtk_window_set_auto_startup_notification",gtk_window_set_auto_startup_notification); // void gboolean
          table_add(table_void_constcharP,"gtk_window_set_default_icon_name",gtk_window_set_default_icon_name); // void const char *
          table_add(table_void_gboolean,"gtk_window_set_interactive_debugging",gtk_window_set_interactive_debugging); // void gboolean
        /* Instance methods */
          table_add(table_void_GtkWindowP,"gtk_window_close",gtk_window_close); // void GtkWindow*
          table_add(table_void_GtkWindowP,"gtk_window_destroy", gtk_window_destroy); // void GtkWindow*
          table_add(table_void_GtkWindowP,"gtk_window_fullscreen", gtk_window_fullscreen); // void GtkWindow*
          /*table_void_GtkWindowP_GdkMonitorP*/ /*gtk_window_fullscreen_on_monitor*/ // void GtkWindow* GdkMonitor*
          /*table_GtkApplication_GtkWindowP*/ /*gtk_window_get_application*/ // GtkApplication* GtkWindow*
          /*table_GtkWidgetP_GtkWindowP*/ /*gtk_window_get_child*/ // GtkWidget* GtkWindow*
          table_add(table_gboolean_GtkWindowP,"gtk_window_get_decorated",gtk_window_get_decorated); // gboolean GtkWindow*
          table_add(table_void_GtkWindowP_intP_intP,"gtk_window_get_default_size",gtk_window_get_default_size); // void GtkWindow* int* int*
          /*table_GtkWidgetP_GtkWindowP*/ /*gtk_window_get_default_widget*/ // GtkWidget* GtkWindow*
          table_add(table_gboolean_GtkWindowP,"gtk_window_get_deletable",gtk_window_get_deletable); // gboolean GtkWindow*
          table_add(table_gboolean_GtkWindowP,"gtk_window_get_destroy_with_parent",gtk_window_get_destroy_with_parent); // gboolean GtkWindow*
          /*table_GtkWidgetP_GtkWindowP*/ /*gtk_window_get_focus*/ // GtkWidget* GtkWindow*
          table_add(table_gboolean_GtkWindowP,"gtk_window_get_focus_visible",gtk_window_get_focus_visible); // gboolean GtkWindow*
          #if GTK_MINOR_VERSION >= 20
            table_add(table_GtkWindowGravity_GtkWindowP,"gtk_window_get_gravity",gtk_window_get_gravity); // GtkWindowGravity GtkWindow*
          #endif
          /*table_GtkWindowGroupP_GtkWindowP*/ /*gtk_window_get_group*/ // GtkWindowGroup* GtkWindow*
          table_add(table_gboolean_GtkWindowP,"gtk_window_get_handle_menubar_accel",gtk_window_get_handle_menubar_accel); // gboolean GtkWindow*
          table_add(table_gboolean_GtkWindowP,"gtk_window_get_hide_on_close",gtk_window_get_hide_on_close); // gboolean GtkWindow*
          table_add(table_constcharP_GtkWindowP,"gtk_window_get_icon_name",gtk_window_get_icon_name); // const char* GtkWindow*
          table_add(table_gboolean_GtkWindowP,"gtk_window_get_mnemonics_visible",gtk_window_get_mnemonics_visible); // gboolean GtkWindow*
          table_add(table_gboolean_GtkWindowP,"gtk_window_get_modal",gtk_window_get_modal); // gboolean GtkWindow*
          table_add(table_gboolean_GtkWindowP,"gtk_window_get_resizable",gtk_window_get_resizable); // gboolean GtkWindow*
          table_add(table_constcharP_GtkWindowP,"gtk_window_get_title",gtk_window_get_title); // const char* GtkWindow*
          /*table_GtkWidgetP_GtkWindowP*/ /*gtk_window_get_titlebar*/ // GtkWidget* GtkWindow*
          /*table_GtkWindowP_GtkWindowP*/ /*gtk_window_get_transient_for*/ // GtkWindow* GtkWindow*
          table_add(table_gboolean_GtkWindowP,"gtk_window_has_group",gtk_window_has_group); // gboolean GtkWindow*
          table_add(table_gboolean_GtkWindowP,"gtk_window_is_active",gtk_window_is_active); // gboolean GtkWindow*
          table_add(table_gboolean_GtkWindowP,"gtk_window_is_fullscreen",gtk_window_is_fullscreen); // gboolean GtkWindow*
          table_add(table_gboolean_GtkWindowP,"gtk_window_is_maximized",gtk_window_is_maximized); // gboolean GtkWindow*
          #if GTK_MINOR_VERSION >= 12
            table_add(table_gboolean_GtkWindowP,"gtk_window_is_suspended",gtk_window_is_suspended); // gboolean GtkWindow*
          #endif
          table_add(table_void_GtkWindowP,"gtk_window_maximize", gtk_window_maximize); // void GtkWindow*
          table_add(table_void_GtkWindowP,"gtk_window_minimize", gtk_window_minimize); // void GtkWindow*
          table_add(table_void_GtkWindowP,"gtk_window_present", gtk_window_present); // void GtkWindow*
          #if GTK_MINOR_VERSION < 14
            table_add(table_void_GtkWindowP_guint32,"gtk_window_present_with_time",gtk_window_present_with_time); // void GtkWindow* guint32
          #endif
          /*table_void_GtkWindowP_GtkApplicationP*/ /*gtk_window_set_application*/ // void GtkWindow* GtkApplication*
          /*table_void_GtkWindowP_GtkWidgetP*/ /*gtk_window_set_child*/ // void GtkWindow* GtkWidget*
          table_add(table_void_GtkWindowP_gboolean,"gtk_window_set_decorated",gtk_window_set_decorated); // void GtkWindow* gboolean
          table_add(table_void_GtkWindowP_int_int,"gtk_window_set_default_size",gtk_window_set_default_size); // void GtkWindow* int int
          /*table_void_GtkWindowP_GtkWidgetP*/ /*gtk_window_set_default_widget*/ // void GtkWindow* GtkWidget*
          table_add(table_void_GtkWindowP_gboolean,"gtk_window_set_deletable",gtk_window_set_deletable); // void GtkWindow* gboolean
          table_add(table_void_GtkWindowP_gboolean,"gtk_window_set_destroy_with_parent",gtk_window_set_destroy_with_parent); // void GtkWindow* gboolean
          /*table_void_GtkWindowP_GdkDisplayP*/ /*gtk_window_set_display*/ // void GtkWindow* GdkDisplay*
          /*table_void_GtkWindowP_GtkWidgetP*/ /*gtk_window_set_focus*/ // void GtkWindow* GtkWidget*
          table_add(table_void_GtkWindowP_gboolean,"gtk_window_set_focus_visible",gtk_window_set_focus_visible); // void GtkWindow* gboolean
          #if GTK_MINOR_VERSION >= 20
            table_add(table_void_GtkWindowP_GtkWindowGravity,"gtk_window_set_gravity",gtk_window_set_gravity); // void GtkWindow* GtkWindowGravity
          #endif
          table_add(table_void_GtkWindowP_gboolean,"gtk_window_set_handle_menubar_accel",gtk_window_set_handle_menubar_accel); // void GtkWindow* gboolean
          table_add(table_void_GtkWindowP_gboolean,"gtk_window_set_hide_on_close",gtk_window_set_hide_on_close); // void GtkWindow* gboolean
          table_add(table_void_GtkWindowP_constcharP,"gtk_window_set_icon_name",gtk_window_set_icon_name); // void GtkWindow* const char*
          table_add(table_void_GtkWindowP_gboolean,"gtk_window_set_mnemonics_visible",gtk_window_set_mnemonics_visible); // void GtkWindow* gboolean
          table_add(table_void_GtkWindowP_gboolean,"gtk_window_set_modal",gtk_window_set_modal); // void GtkWindow* gboolean
          table_add(table_void_GtkWindowP_gboolean,"gtk_window_set_resizable",gtk_window_set_resizable); // void GtkWindow* gboolean
          table_add(table_void_GtkWindowP_constcharP,"gtk_window_set_startup_id",gtk_window_set_startup_id); // void GtkWindow* const char*
          table_add(table_void_GtkWindowP_constcharP,"gtk_window_set_title",gtk_window_set_title); // void GtkWindow* const char*
          /*table_void_GtkWindowP_GtkWidgetP*/ /*gtk_window_set_titlebar*/ // void GtkWindow* GtkWidget*
          /*table_void_GtkWindowP_GtkWindowP*/ /*gtk_window_set_transient_for*/ // void GtkWindow* GtkWindow*
          table_add(table_void_GtkWindowP,"gtk_window_unfullscreen", gtk_window_unfullscreen); // void GtkWindow*
          table_add(table_void_GtkWindowP,"gtk_window_unmaximize", gtk_window_unmaximize); // void GtkWindow*
          table_add(table_void_GtkWindowP,"gtk_window_unminimize", gtk_window_unminimize); // void GtkWindow*
        /* Virtual methods */
          // void activate_default (GtkWindow* window)
          // void activate_focus (GtkWindow* window)
          // gboolean close_request (GtkWindow* window)
          // gboolean enable_debugging (GtkWindow* window, gboolean toggle)
          // void keys_changed (GtkWindow* window)
    //
    while(TRUE) {
        fgets(input, 1024, pargs->fpin);
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
        if(table_get(table_void_gboolean, command) || table_get(table_void_constcharP, command)) {
            operanda = widget_id;
            widget_id = NULL;
        }
        if(VERBOSE) fprintf(stderr, "CALLBACK:> %s %s %s %s\n", command, widget_id == NULL ? "NULL" : widget_id, operanda == NULL ? "NULL" : operanda, operandb == NULL ? "NULL" : operandb);
        GtkWidget *widget = widget_id == NULL ? NULL : GTK_WIDGET(gtk_builder_get_object(pargs->builder, widget_id));


        if(NULL != (vu=table_get(table_charPP_GtkWidgetP,command))) { /*001*/} else
        if(NULL != (vu=table_get(table_constcairo_font_options_tP_GtkWidgetP,command))) { /*002*/} else
        if(NULL != (vu=table_get(table_constcharP,command))) { /*005*/
            if(DEBUG) fprintf(stderr,"CALLBACK005 %s",command);
            const char* value = ((sig_constcharP)vu)();
            if(DEBUG) fprintf(stderr," %s\n",value);
            fprintf(pargs->fpout, "%s\n", value);
            fflush(pargs->fpout);
        } else
        if(NULL != (vu=table_get(table_constcharP_GtkWidgetClassP,command))) { /*003*/} else
        if(NULL != (vu=table_get(table_constcharP_GtkWidgetP,command))) { /*004*/} else
        if(NULL != (vu=table_get(table_constcharP_GtkWindowP,command))) { /*006*/
            if(DEBUG) fprintf(stderr,"CALLBACK006 %s, widget_id: %s\n",command,widget_id);
            const char*value = ((sig_constcharP_GtkWindowP)vu)(GTK_WINDOW(widget));
            fprintf(pargs->fpout, "%s\n", value);
            fflush(pargs->fpout);
        } else
        if(NULL != (vu=table_get(table_double_GtkWidgetP,command))) { /*007*/} else
        if(NULL != (vu=table_get(table_gboolean_GtkWidgetClassP_guint_GTypeP_constcharPP_constGVariantTypePP_constcharPP,command))) { /*008*/} else
        if(NULL != (vu=table_get(table_gboolean_GtkWidgetP_constcharP,command))) { /*009*/} else
        if(NULL != (vu=table_get(table_gboolean_GtkWidgetP_constcharP_constcharP_Elipse,command))) { /*010*/} else
        if(NULL != (vu=table_get(table_gboolean_GtkWidgetP_constcharP_GVariantP,command))) { /*011*/} else
        if(NULL != (vu=table_get(table_gboolean_GtkWidgetP_double_double,command))) { /*012*/} else
        if(NULL != (vu=table_get(table_gboolean_GtkWidgetP_gboolean,command))) { /*013*/} else
        if(NULL != (vu=table_get(table_gboolean_GtkWidgetP_GtkDirectionType,command))) { /*014*/} else
        if(NULL != (vu=table_get(table_gboolean_GtkWidgetP_GtkOrientation,command))) { /*015*/} else
        if(NULL != (vu=table_get(table_gboolean_GtkWidgetP_GtkWidgetP_constgraphene_point_tP_graphene_point_tP,command))) { /*016*/} else
        if(NULL != (vu=table_get(table_gboolean_GtkWidgetP_GtkWidgetP_double_double_doubleP_doubleP,command))) { /*017*/} else
        if(NULL != (vu=table_get(table_gboolean_GtkWidgetP_GtkWidgetP_graphene_matrix_tP,command))) { /*018*/} else
        if(NULL != (vu=table_get(table_gboolean_GtkWidgetP_GtkWidgetP,command))) { /*019*/} else
        if(NULL != (vu=table_get(table_gboolean_GtkWidgetP_GtkWidgetP_graphene_rect_tP,command))) { /*020*/} else
        if(NULL != (vu=table_get(table_gboolean_GtkWidgetP_int_int_int_int,command))) { /*021*/} else
        if(NULL != (vu=table_get(table_gboolean_GtkWidgetP,command))) { /*022*/} else
        if(NULL != (vu=table_get(table_gboolean_GtkWindowP,command))) { /*023*/
            if(DEBUG) fprintf(stderr,"CALLBACK023 %s, widget_id: %s\n",command,widget_id);
            gboolean b = ((sig_gboolean_GtkWindowP)vu)(GTK_WINDOW(widget));
            fprintf(pargs->fpout, "%i\n", b);
            fflush(pargs->fpout);
        } else
        if(NULL != (vu=table_get(table_GdkClipboardP_GtkWidgetP,command))) { /*024*/} else
        if(NULL != (vu=table_get(table_GdkCursorP_GtkWidgetP,command))) { /*025*/} else
        if(NULL != (vu=table_get(table_GdkDisplayP_GtkWidgetP,command))) { /*026*/} else
        if(NULL != (vu=table_get(table_GdkFrameClockP_GtkWidgetP,command))) { /*027*/} else
        if(NULL != (vu=table_get(table_GListP,command))) { /*028*/} else
        if(NULL != (vu=table_get(table_GListP_GtkWidgetP,command))) { /*029*/} else
        if(NULL != (vu=table_get(table_GListModelP,command))) { /*030*/} else
        if(NULL != (vu=table_get(table_GListModelP_GtkWidgetP,command))) { /*031*/} else
        if(NULL != (vu=table_get(table_GObjectP_GtkWidgetP_GType_constcharP,command))) { /*032*/} else
        if(NULL != (vu=table_get(table_GtkAccessibleRole_GtkWidgetClassP,command))) { /*033*/} else
        if(NULL != (vu=table_get(table_GtkAlign_GtkWidgetP,command))) { /*034*/} else
        if(NULL != (vu=table_get(table_GtkApplication_GtkWindowP,command))) { /*035*/} else
        if(NULL != (vu=table_get(table_GtkLayoutManagerP_GtkWidgetP,command))) { /*036*/} else
        if(NULL != (vu=table_get(table_GtkNativeP_GtkWidgetP,command))) { /*037*/} else
        if(NULL != (vu=table_get(table_GtkOverflow_GtkWidgetP,command))) { /*038*/} else
        if(NULL != (vu=table_get(table_GtkRootP_GtkWidgetP,command))) { /*039*/} else
        if(NULL != (vu=table_get(table_GtkSettingsP_GtkWidgetP,command))) { /*040*/} else
        if(NULL != (vu=table_get(table_GtkSizeRequestMode_GtkWidgetP,command))) { /*041*/} else
        if(NULL != (vu=table_get(table_GtkStateFlags_GtkWidgetP,command))) { /*042*/} else
        if(NULL != (vu=table_get(table_GtkStyleContextP_GtkWidgetP,command))) { /*043*/} else
        if(NULL != (vu=table_get(table_GtkTextDirection_GtkWidgetP,command))) { /*044*/} else
        if(NULL != (vu=table_get(table_GtkTextDirection_void,command))) { /*045*/} else
        if(NULL != (vu=table_get(table_GtkWidgetP_GtkWidgetP,command))) { /*046*/} else
        if(NULL != (vu=table_get(table_GtkWidgetP_GtkWidgetP_double_double_GtkPickFlags,command))) { /*047*/} else
        if(NULL != (vu=table_get(table_GtkWidgetP_GtkWidgetP_GType,command))) { /*048*/} else
        if(NULL != (vu=table_get(table_GtkWidgetP_GtkWindowP,command))) { /*049*/} else
        #if GTK_MINOR_VERSION >= 20
          if(NULL != (vu=table_get(table_GtkWindowGravity_GtkWindowP,command))) { /*118*/
            if(DEBUG) fprintf(stderr,"CALLBACK118 %s, widget_id: %s\n",command,widget_id);
            GtkWindowGravity g = ((sig_GtkWindowGravity_GtkWindowP)vu)(GTK_WINDOW(widget));
            fprintf(pargs->fpout, "%i\n", g);
            fflush(pargs->fpout);
          } else
        #endif
        if(NULL != (vu=table_get(table_GtkWindowGroupP_GtkWindowP,command))) { /*050*/} else
        if(NULL != (vu=table_get(table_GtkWindowP_GtkWindowP,command))) { /*051*/} else
        if(NULL != (vu=table_get(table_GType_GtkWidgetClassP,command))) { /*052*/} else
        if(NULL != (vu=table_get(table_guint_GtkWidgetClassP,command))) { /*053*/} else
        if(NULL != (vu=table_get(table_guint_GtkWidgetP_GtkTickCallback_gpointer_GDestroyNotify,command))) { /*054*/} else
        if(NULL != (vu=table_get(table_int_GtkWidgetP,command))) { /*055*/} else
        if(NULL != (vu=table_get(table_int_GtkWidgetP_GtkOrientation,command))) { /*056*/} else
        if(NULL != (vu=table_get(table_PangoContextP_GtkWidgetP,command))) { /*057*/} else
        if(NULL != (vu=table_get(table_PangoFontMapP_GtkWidgetP,command))) { /*058*/} else
        if(NULL != (vu=table_get(table_PangoLayoutP_GtkWidgetP_constcharP,command))) { /*059*/} else
        if(NULL != (vu=table_get(table_void_constcharP,command))) { /*060*/
            if(DEBUG) fprintf(stderr,"CALLBACK060 %s, argument: %s\n",command,operanda);
            ((sig_void_constcharP)vu)(operanda);
        } else
        if(NULL != (vu=table_get(table_void_gboolean,command))) { /*061*/
            if(DEBUG) fprintf(stderr,"CALLBACK061 %s, argument: %s\n",command,operanda);
            gboolean b = (int) strtol(operanda, (char **)NULL, 10) == 0 ? FALSE : TRUE;
            ((sig_void_gboolean)vu)(b);
        } else
        if(NULL != (vu=table_get(table_void_GtkTextDirection,command))) { /*062*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetClassP_constcharP,command))) { /*063*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetClassP_constcharP_constcharP,command))) { /*064*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetClassP_constcharP_constcharP_GtkWidgetActionActivateFunc,command))) { /*065*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetClassP_constcharP_gboolean_gssize,command))) { /*066*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetClassP_constcharP_GCallback,command))) { /*067*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetClassP_GBytesP,command))) { /*068*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetClassP_GtkAccessibleRole,command))) { /*069*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetClassP_GtkBuilderScopeP,command))) { /*070*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetClassP_GtkShortcutP,command))) { /*071*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetClassP_GType,command))) { /*072*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetClassP_guint,command))) { /*073*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetClassP_guint_GdkModifierType_constcharP_constcharP_Elipse,command))) { /*074*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetClassP_guint_GdkModifierType_GtkShortcutFunc_constcharP_Elipse,command))) { /*075*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP,command))) { /*076*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_constcairo_font_options_tP,command))) { /*077*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_constcharP,command))) { /*078*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_constcharP_GActionGroupP,command))) { /*079*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_constcharP_gboolean,command))) { /*080*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_constcharPP,command))) { /*081*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_constGtkAllocationP_int,command))) { /*082*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_double,command))) { /*083*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_gboolean,command))) { /*084*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_GdkCursorP,command))) { /*085*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_GdkRGBA,command))) { /*086*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_GtkAlign,command))) { /*087*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_GtkAllocationP,command))) { /*088*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_GtkEventControllerP,command))) { /*089*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_GtkLayoutManagerP,command))) { /*090*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_GtkOrientation_int_intP_intP_intP_intP,command))) { /*091*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_GtkOverflow,command))) { /*092*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_GtkRequisitionP_GtkRequisitionP,command))) { /*093*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_GtkStateFlags_gboolean,command))) { /*094*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_GtkStateFlags,command))) { /*095*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_GtkTextDirection,command))) { /*096*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_GtkWidgetP,command))) { /*097*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_GtkWidgetP_GtkSnapshotP,command))) { /*098*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_GtkWidgetP_GtkWidgetP,command))) { /*099*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_GType,command))) { /*100*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_guint,command))) { /*101*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_int,command))) { /*102*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_int_int,command))) { /*103*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_int_int_int_GskTransformP,command))) { /*104*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_intP_intP,command))) { /*105*/} else
        if(NULL != (vu=table_get(table_void_GtkWidgetP_PangoFontMapP,command))) { /*106*/} else
        if(NULL != (vu=table_get(table_void_GtkWindowP,command))) { /*107*/
            if(DEBUG) fprintf(stderr,"CALLBACK107 %s, widget_id: %s\n",command,widget_id);
            ((sig_void_GtkWindowP)vu)(GTK_WINDOW(widget));
        } else
        if(NULL != (vu=table_get(table_void_GtkWindowP_constcharP,command))) { /*108*/
            if(DEBUG) fprintf(stderr,"CALLBACK108 %s, widget_id: %s, argument: %s\n",command,widget_id,operanda);
            ((sig_void_GtkWindowP_constcharP)vu)(GTK_WINDOW(widget),operanda);
        } else
        if(NULL != (vu=table_get(table_void_GtkWindowP_gboolean,command))) { /*109*/
            if(DEBUG) fprintf(stderr,"CALLBACK109 %s, widget_id: %s, argument: %s\n",command,widget_id,operanda);
            gboolean b = (int) strtol(operanda, (char **)NULL, 10) == 0 ? FALSE : TRUE;
            ((sig_void_GtkWindowP_gboolean)vu)(GTK_WINDOW(widget),b);
        } else
        if(NULL != (vu=table_get(table_void_GtkWindowP_GdkDisplayP,command))) { /*110*/} else
        if(NULL != (vu=table_get(table_void_GtkWindowP_GdkMonitorP,command))) { /*111*/} else
        if(NULL != (vu=table_get(table_void_GtkWindowP_GtkApplicationP,command))) { /*112*/} else
        if(NULL != (vu=table_get(table_void_GtkWindowP_GtkWidgetP,command))) { /*113*/} else
        #if GTK_MINOR_VERSION >= 20
          if(NULL != (vu=table_get(table_void_GtkWindowP_GtkWindowGravity,command))) { /*119*/ 
            if(DEBUG) fprintf(stderr,"CALLBACK119 %s, widget_id: %s, argument: %s\n",command,widget_id,operanda);
            GtkWindowGravity g = (int) strtol(operanda, (char **)NULL, 10);
            ((sig_void_GtkWindowP_GtkWindowGravity)vu)(GTK_WINDOW(widget),g);
          } else
        #endif
        if(NULL != (vu=table_get(table_void_GtkWindowP_GtkWindowP,command))) { /*114*/} else
        if(NULL != (vu=table_get(table_void_GtkWindowP_guint32,command))) { /*115*/
            if(DEBUG) fprintf(stderr,"CALLBACK115 %s, widget_id: %s, argument: %s\n",command,widget_id,operanda);
            guint32 i = (int) strtol(operanda, (char **)NULL, 10);
            ((sig_void_GtkWindowP_guint32)vu)(GTK_WINDOW(widget),i);
        } else
        if(NULL != (vu=table_get(table_void_GtkWindowP_int_int,command))) { /*116*/
            if(DEBUG) fprintf(stderr,"CALLBACK116 %s, widget_id: %s, argument1: %s, argument2: %s\n",command,widget_id,operanda, operandb);
            int x = (int) strtol(operanda, (char **)NULL, 10);
            int y = (int) strtol(operandb, (char **)NULL, 10);
            ((sig_void_GtkWindowP_int_int)vu)(GTK_WINDOW(widget),x,y);
        } else
        if(NULL != (vu=table_get(table_void_GtkWindowP_intP_intP,command))) { /*117*/
            if(DEBUG) fprintf(stderr,"CALLBACK117 %s, widget_id: %s\n",command,widget_id);
            int x, y;
            ((sig_void_GtkWindowP_intP_intP)vu)(GTK_WINDOW(widget),&x, &y);
            fprintf(pargs->fpout, "%i\n", x);
            fprintf(pargs->fpout, "%i\n", y);
            fflush(pargs->fpout);
        } else /*118*//*119*/
        

       
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
            fprintf(pargs->fpout, "%s\n", mtext);  
            fflush(pargs->fpout);
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
            fprintf(pargs->fpout, "%s\n", mtext);  
            fflush(pargs->fpout);
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
            fprintf(pargs->fpout, "%d\n", gtk_combo_box_get_active(GTK_COMBO_BOX(widget)));  
            fflush(pargs->fpout);
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
                fprintf(pargs->fpout, "1\n");
            else
                fprintf(pargs->fpout, "0\n");
            fflush(pargs->fpout);
        } else

        if(!strcmp(command, "gtk_editable_get_text")) {
            const char* mtext=gtk_editable_get_text(GTK_EDITABLE(widget));
            fprintf(pargs->fpout, "%s\n", mtext);  
            fflush(pargs->fpout);
        }

    }

    fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ENDE %s()... Ich dachte, da kommen wir nie hin \n", __func__);
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
    if(DEBUG) fprintf(stderr, "  HANDLER %s()... ruft nur app_do auf\n", __func__);
    /* Dieser Funktion endet mit dem Anzeigen des Dialogs. 
     * Danach befinden wir uns in g_application_run */
    app_do(app, user_data);
  }
static void app_open(GtkApplication *app, GFile ** files, gint n_files, gchar *hint, gpointer *user_data) {
    if(DEBUG) fprintf(stderr, "  HANDLER %s()... ruft nur app_do auf\n", __func__);
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
            if(DEBUG) fprintf(stderr, "%s(): thread was canceled\n", __func__);
            pargs->thread=0;
        } else
            fprintf(stderr, "!!!!ERROR %s(): thread wasn't canceled (shouldn't happen!)\n", __func__);
    }
    g_object_unref(pargs->builder);pargs->builder=NULL;
    char **tmp = pargs->SIGNALS;
    while(*tmp)
        free(*tmp++);
    free(pargs->SIGNALS);pargs->SIGNALS=NULL;
    tables_free();
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
    args.name_out  = NULL;
    args.name_in   = NULL;
    args.win_id    = "window1";
    args.builder   = NULL;
    args.SIGNALS   = NULL;
    args.thread    = 0;
    args.fpout     = NULL;
    args.fpin      = NULL;

    read_opts(&args, &argc, &argv);
    if(DEBUG) fprintf(stderr, "MAIN:Optionen eingelesen ...\n");
    if(!args.ui_file) 
        help(args.app_name);
    if((args.name_out && !args.name_in) || (args.name_in && !args.name_out))
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
    g_object_unref(app);
    return stat;
}

