// wrap_add_signals von: https://github.com/abecadel/gtkwrap
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <libintl.h>

#define APP_ID "daheim.oskopia.gtk4bash"

#define STRING_SIZE 128
#define OBJECT_TAG "<object class=\""
#define SIGNAL_TAG "<signal name=\""
#define ID_TAG "\" id=\""
#define HANDLER_TAG "\" handler=\""

char **SIG_HANDLERS = NULL;
short VERBOSE = 0;

typedef struct {
    char *app_name;
    char *ui_file;
    char *win_id;
} _args;

void cbk_wrap_signal_handler(gpointer user_data, GObject *object){
    char *signal = (char *)user_data;
    fprintf(stdout, "%s\n", signal);
    fflush(stdout);
  }
static void wrap_cleanup(_args* pargs) {
    char **tmp = SIG_HANDLERS;
    while(*tmp)
        free(*tmp++);
    free(SIG_HANDLERS);
  }
void wrap_add_signals(char *filename, GtkBuilder *builder){
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
    
    SIG_HANDLERS = (char**)calloc(50, sizeof(char*));
    if(!SIG_HANDLERS){
        if(VERBOSE) printf( "Error allocating memory: SIG_HANDLERS!\n");
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
            SIG_HANDLERS[hand_count] = (char*)calloc(strlen(sighandler) + 1, sizeof(char));
            strncpy(SIG_HANDLERS[hand_count], sighandler, strlen(sighandler));
            GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object( builder, objname));
            g_signal_connect_swapped(widget, signame, G_CALLBACK(cbk_wrap_signal_handler), SIG_HANDLERS[hand_count]);
            hand_count++;
            continue;
        }
    }
    SIG_HANDLERS[hand_count] = NULL;

    fclose(file); 
  }
static void app_do(GtkApplication *app, gpointer *user_data) {
    _args *pargs = (_args *)user_data;
    GtkWidget *win; // GtkDialog deprecated with GTK4.10, use GtkWindow

    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, pargs->ui_file, NULL);
    win = GTK_WIDGET(gtk_builder_get_object(builder, pargs->win_id));
    gtk_window_set_application(GTK_WINDOW(win), GTK_APPLICATION(app));

    wrap_add_signals(pargs->ui_file, builder);
    gtk_window_present(GTK_WINDOW(win));
    g_object_unref(builder);
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
static void help(char *appname){
    printf("\
    Aufruf:\n\
    %s -f string [-m string][-v]\n\
    -f DLGFILE\t\t Die Dialogdatei\n\
    -m OBJECTNAME \t OBJECTNAME fürs main window. Vorgabe ist \"window1\".\n\
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

    while((opt = getopt(argc, argv, ":vhf:m:")) != -1) { 
        switch(opt) { 
            case 'v': 
                VERBOSE = 1;
                if(VERBOSE) printf("-v\n"); 
                break; 
            case 'h': 
                if(VERBOSE) printf("-h\n"); 
                help(pargs->app_name);
                break; 
            case 'f': 
                if(VERBOSE) printf("-%c: %s\n", opt, optarg);
                if(optarg[0]=='-') printf("ACHTUNG: -%c: %s\n", opt, optarg);
                pargs->ui_file  = optarg;
                break; 
            case 'm': 
                if(VERBOSE) printf("-%c: %s\n", opt, optarg);
                if(optarg[0]=='-') printf("ACHTUNG: -%c: %s\n", opt, optarg);
                pargs->win_id  = optarg;
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
    args.win_id    = "window1";

    read_opts(&args, &argc, &argv);
    if(!args.ui_file) 
        help(args.app_name);
    if(VERBOSE) for(idx=0; idx < argc; idx++)printf("%i: %s\n", idx, argv[idx]);
    if(VERBOSE) printf("UI-Datei: %s; TOP-WINDOW: %s\n", args.ui_file, args.win_id);
    app = gtk_application_new(APP_ID, G_APPLICATION_HANDLES_OPEN);
    g_signal_connect(app, "activate", G_CALLBACK (app_activate), &args);
    g_signal_connect(app, "open", G_CALLBACK (app_open), &args);
    stat = g_application_run(G_APPLICATION(app), argc, argv);
    
    wrap_cleanup(&args);

    g_object_unref(app);
    return stat;
}

