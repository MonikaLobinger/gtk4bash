#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <libintl.h>

short RUNNING = 1;
#define STRING_SIZE 128
#define OBJECT_TAG "<object class=\""
#define SIGNAL_TAG "<signal name=\""
#define ID_TAG "\" id=\""
#define HANDLER_TAG "\" handler=\""
char **SIG_HANDLERS;

short VERBOSE = 0;

typedef struct {
    char *app_name;
    char *ui_file;
    char *dlg_id;
    char *pipe_out;
    char *pipe_in;
    pthread_t thread;
    GtkBuilder *builder;
} _args;

void signal_handler(gpointer user_data, GObject *object){
    char *jo = (char *)user_data;
    fprintf(stdout, "%s\n", jo);
    fflush(stdout);
}
static void free_all_signals() {
    char **tmp = SIG_HANDLERS;
    while(*tmp)
        free(*tmp++);
    free(SIG_HANDLERS);
  }
//Quelle: https://github.com/abecadel/gtkwrap
void *reader_loop(void* user_data){
    _args *args;
    args =        (_args *)user_data;
    


    mkfifo(args->pipe_out, S_IRWXU);
    FILE *fileout = fopen(args->pipe_out, "a+");
    if(!fileout){
        fprintf(stderr, "Error opening pipe %s !\n", args->pipe_out);
        pthread_exit(NULL);
    }


    mkfifo(args->pipe_in, S_IRWXU);
    FILE *filein = fopen(args->pipe_in, "r+");    
    if(!filein){
        fprintf(stderr, "Error opening pipe %s !\n", args->pipe_in);
        pthread_exit(NULL);
    }

    if(VERBOSE)
        fprintf(stderr, "Using pipes out:%s in:%s\n", args->pipe_out, args->pipe_in);


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
  
        GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(args->builder, object));
        
 
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
//Quelle: https://github.com/abecadel/gtkwrap
void auto_add_signals(char *filename, GtkBuilder *builder){
    //Adding signals handled in glade file
    //TODO: make less dumb, replace by real xml parser
    //Quelle: https://github.com/abecadel/gtkwrap

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
            g_signal_connect_swapped(widget, signame, G_CALLBACK(signal_handler), SIG_HANDLERS[hand_count]);
            hand_count++;
            continue;
        }
    }
    SIG_HANDLERS[hand_count] = NULL;

    fclose(file); 
}

static void do_dialog(GtkApplication *app, gpointer *user_data) {
    _args *args;
    GtkWidget *dialog; // GtkDialog deprecated with GTK4.10, use GtkWindow
    GtkWidget *btn_close;
    char *filename;
    char *main_object;

    args =        (_args *)user_data;
    filename =    args->ui_file;
    main_object = args->dlg_id;

    args->builder = gtk_builder_new();
    gtk_builder_add_from_file(args->builder, filename, NULL);
    dialog = GTK_WIDGET(gtk_builder_get_object(args->builder, main_object));
    gtk_window_set_application(GTK_WINDOW(dialog), GTK_APPLICATION(app));

    auto_add_signals(filename, args->builder);
    gtk_window_present(GTK_WINDOW(dialog));
    if(args->pipe_out)
        pthread_create(&(args->thread), NULL, reader_loop, args);
    g_object_unref(args->builder);
}
static void app_activate(GtkApplication *app, gpointer user_data) {
    /* Dieser Funktion endet mit dem Anzeigen des Dialogs. 
     * Danach befinden wir uns in g_application_run */
    do_dialog(app, user_data);
  }
static void app_open(GtkApplication *app, GFile ** files, gint n_files, gchar *hint, gpointer *user_data) {
    /* Dieser Funktion endet mit dem Anzeigen des Dialogs. 
     * Danach befinden wir uns in g_application_run */
    do_dialog(app, user_data);
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
    const char *app_id;
    GApplicationFlags app_flags;
    _args args;

    app_id =       "daheim.oskopia.gtk4bash";
    app_flags =    G_APPLICATION_HANDLES_OPEN;

    args.app_name  = argv[0];
    args.ui_file   = NULL;
    args.dlg_id    = "window1";
    args.pipe_out  = NULL;
    args.pipe_in   = NULL;
    args.thread    = 0;
    args.builder   = NULL;

    read_opts(&args, &argc, &argv);
    if(!args.ui_file) 
        help();
    if(VERBOSE) for(idx=0; idx < argc; idx++)printf("%i: %s\n", idx, argv[idx]);
    if(VERBOSE) printf("UI-Datei: %s; TOP-WINDOW: %s\n", args.ui_file, args.dlg_id);
    app = gtk_application_new(app_id, app_flags);
    g_signal_connect(app, "activate", G_CALLBACK (app_activate), &args);
    g_signal_connect(app, "open", G_CALLBACK (app_open), &args);
    stat = g_application_run(G_APPLICATION(app), argc, argv);
    
    RUNNING = 0;
    if(args.pipe_out)
        pthread_cancel(args.thread);
    unlink(args.pipe_out);
    unlink(args.pipe_in);
    free_all_signals();

    g_object_unref(app);
    return stat;
}

