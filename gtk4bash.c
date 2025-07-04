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

short DEBUG   = 0;
short VERBOSE = 0;
short RUNNING = 0;

typedef struct {
    char       *app_name;
    char       *ui_file;
    char       *win_id;
    GtkBuilder *builder;
    char*      *SIGNALS;
    char       *fpipeout;
    char       *fpipein;
    pthread_t   thread;
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
    char *object = NULL;
    char *command = NULL;


    while(RUNNING){
        fgets(input, 1024, filein);

        if(!RUNNING)
            break; 

        object = input;
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
            fprintf(stderr, "Command:> %s %s %s\n", object, command, operanda);
  
        GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(pargs->builder, object));
        
 
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
static void app_do(GtkApplication *app, gpointer *user_data) {
    _args *pargs = (_args *)user_data;
    GtkWidget *win; // GtkDialog deprecated with GTK4.10, use GtkWindow

    pargs->builder = gtk_builder_new();
    gtk_builder_add_from_file(pargs->builder, pargs->ui_file, NULL);
    win = GTK_WIDGET(gtk_builder_get_object(pargs->builder, pargs->win_id));
    gtk_window_set_application(GTK_WINDOW(win), GTK_APPLICATION(app));

    wrap_add_signals(pargs->ui_file, pargs);
    gtk_window_present(GTK_WINDOW(win));
    RUNNING = 1;
    if(pargs->fpipeout)
        pthread_create(&(pargs->thread), NULL, wrap_reader_loop, pargs);
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
static void app_end(GtkApplication *app, gpointer *user_data) {
    if(DEBUG) fprintf(stderr, "START app_end...\n");
    RUNNING = 0;
  }
static void help(char *appname){
    printf("\
    Aufruf:\n\
    %s -f string [-m string][-o string][-i string][-d][-v]\n\
    -f DLGFILE\t\t Die Dialogdatei\n\
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

    while((opt = getopt(argc, argv, ":dvhf:m:o:i:")) != -1) { 
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
    args.win_id    = "window1";
    args.builder   = NULL;
    args.SIGNALS   = NULL;
    args.fpipeout  = NULL;
    args.fpipein   = NULL;
    args.thread    = 0;

    read_opts(&args, &argc, &argv);
    if(!args.ui_file) 
        help(args.app_name);
    if((args.fpipeout && !args.fpipein) || (args.fpipein && !args.fpipeout))
        help(args.app_name);
    if(VERBOSE) for(idx=0; idx < argc; idx++)printf("%i: %s\n", idx, argv[idx]);
    if(VERBOSE) printf("UI-Datei: %s; TOP-WINDOW: %s\n", args.ui_file, args.win_id);
    app = gtk_application_new(APP_ID, G_APPLICATION_HANDLES_OPEN);
    g_signal_connect(app, "activate", G_CALLBACK (app_activate), &args);
    g_signal_connect(app, "open", G_CALLBACK (app_open), &args);
    g_signal_connect(app, "shutdown", G_CALLBACK (app_end), &args);
    stat = g_application_run(G_APPLICATION(app), argc, argv);
    if(DEBUG) fprintf(stderr, "ENDE App läuft nicht mehr...\n");
    wrap_cleanup(&args);
    g_object_unref(app);
    return stat;
}

