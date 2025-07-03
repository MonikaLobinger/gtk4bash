#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <libintl.h>

short VERBOSE = 0;

typedef struct {
    char *app_name;
    char *ui_file;
    char *dlg_id;
    char *pipe_out;
    char *pipe_in;
} _args;

void on_close(GObject *object, gpointer user_data){
    g_print("START on_close\n");
    GtkWindow *win;
    win = (GtkWindow *)user_data;

    gtk_window_destroy (win);

    g_print("END on_close\n");
}

static void do_dialog(GtkApplication *app, gpointer *user_data) {
    g_print("START do_dialog\n");
    _args *args;
    GtkBuilder *builder;
    GtkWidget *dialog; // GtkDialog deprecated with GTK4.10, use GtkWindow
    GtkWidget *btn_close;
    char *filename;
    char *main_object;

    args =        (_args *)user_data;
    filename =    args->ui_file;
    main_object = args->dlg_id;

    builder = gtk_builder_new_from_file(filename);
    dialog = GTK_WIDGET(gtk_builder_get_object(builder, main_object));
    gtk_window_set_application(GTK_WINDOW(dialog), GTK_APPLICATION(app));

    btn_close = GTK_WIDGET(gtk_builder_get_object(builder, "close_button"));
    g_signal_connect(btn_close, "clicked", G_CALLBACK(on_close), dialog);

    gtk_window_present(GTK_WINDOW(dialog));

    g_object_unref(builder);
    g_print("END do_dialog\n");
}
static void app_activate(GtkApplication *app, gpointer user_data) {
    /* Dieser Funktion endet mit dem Anzeigen des Dialogs. 
     * Danach befinden wir uns in g_application_run */
    g_print("START app_activate\n");
    do_dialog(app, user_data);
    g_print("END app_activate\n");
  }
static void app_open(GtkApplication *app, GFile ** files, gint n_files, gchar *hint, gpointer *user_data) {
    /* Dieser Funktion endet mit dem Anzeigen des Dialogs. 
     * Danach befinden wir uns in g_application_run */
    g_print("START app_open\n");
    do_dialog(app, user_data);
    g_print("END app_open\n");
  }
static void help(){
    char* appname="xxx";
    printf("\
    Aufruf:\n\
    %s -f string [-m string][-v][-i string][-o string]\n\
    -f DLGFILE\t\t Die Dialogdatei\n\
    -m OBJECTNAME \t OBJECTNAME fürs main window. Vorgabe ist \"window1\".\n\
    -v \t\t\t Verbose.\n\
    -i INPIPE \t\t Benutze INPIPE anstatt standard input.\n\
    -o OUTPIPE \t\t Benutze OUTPIPE anstatt standard output.\n"
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

    while((opt = getopt(argc, argv, ":vhf:m:i:o:")) != -1) { 
        switch(opt) { 
            case 'v': 
                VERBOSE = 1;
                if(VERBOSE) printf("-v\n"); 
                break; 
            case 'h': 
                if(VERBOSE) printf("-h\n"); 
                help();
                break; 
            case 'f': 
                if(VERBOSE) printf("-%c: %s\n", opt, optarg);
                if(optarg[0]=='-') printf("ACHTUNG: -%c: %s\n", opt, optarg);
                pargs->ui_file  = optarg;
                break; 
            case 'm': 
                if(VERBOSE) printf("-%c: %s\n", opt, optarg);
                if(optarg[0]=='-') printf("ACHTUNG: -%c: %s\n", opt, optarg);
                pargs->dlg_id  = optarg;
                break; 
            case 'i': 
                if(VERBOSE) printf("-%c: %s\n", opt, optarg);
                if(optarg[0]=='-') printf("ACHTUNG: -%c: %s\n", opt, optarg);
                pargs->pipe_out  = optarg;
                break; 
            case 'o': 
                if(VERBOSE) printf("-%c: %s\n", opt, optarg);
                if(optarg[0]=='-') printf("ACHTUNG: -%c: %s\n", opt, optarg);
                pargs->pipe_in  = optarg;
                break; 
            case ':': 
                if(VERBOSE) printf("Die Option braucht einen Wert\n"); 
                break; 
            case '?': 
                if(VERBOSE) printf("Option %c wird nicht unterstützt\n", optopt);
                break; 
        } 
    } 
    
    for(pos=argc-optind-1; optind < argc; optind++){     
        if(VERBOSE) printf("ARG: %s\n", argv[optind]); 
        argv[pos++]=argv[optind];
    }  
    *pargc=pos;
  }
int main(int argc, char **argv) {
    g_print("START main\n");
    int stat;
    int idx;
    GtkApplication *app;
    const char *app_id;
    GApplicationFlags app_flags;
    _args args;

    app_id =       "daheim.oskopia.gtk4bash";
    app_flags =    G_APPLICATION_HANDLES_OPEN;

    args.app_name = argv[0];
    args.ui_file  = NULL;
    args.dlg_id   = "window1";
    args.pipe_out = NULL;
    args.pipe_in  = NULL;

    read_opts(&args, &argc, &argv);
    if(!args.ui_file) 
        help();
    if(VERBOSE) for(idx=0; idx < argc; idx++)printf("%i: %s\n", idx, argv[idx]);
    if(VERBOSE) printf("UI-Datei: %s; TOP-WINDOW: %s\n", args.ui_file, args.dlg_id);

    app = gtk_application_new(app_id, app_flags);
    g_signal_connect(app, "activate", G_CALLBACK (app_activate), &args);
    g_signal_connect(app, "open", G_CALLBACK (app_open), &args);
    stat = g_application_run(G_APPLICATION(app), argc, argv);
    
    g_object_unref(app);
    g_print("END main\n");
    return stat;
}

