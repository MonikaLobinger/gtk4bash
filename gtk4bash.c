#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <libintl.h>

struct _args {
    char *ui_file;
    char *dlg_id;
};

void on_close(GObject *object, gpointer user_data){
    g_print("START on_close\n");
    GtkWindow *win;
    win = (GtkWindow *)user_data;

    gtk_window_destroy (win);

    g_print("END on_close\n");
}

static void do_dialog(GtkApplication *app, gpointer *user_data) {
    g_print("START do_dialog\n");
    struct _args *args;
    GtkBuilder *builder;
    GtkWidget *dialog; // GtkDialog deprecated with GTK4.10, use GtkWindow
    GtkWidget *btn_close;
    char *filename;
    char *main_object;

    args =        (struct _args *)user_data;
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

int main(int argc, char **argv) {
    g_print("START main\n");
    int stat;
    GtkApplication *app;
    const char *app_id;
    GApplicationFlags app_flags;
    struct _args args;

    app_id =       "daheim.oskopia.gtk4bash";
    app_flags =    G_APPLICATION_HANDLES_OPEN;
    args.ui_file = "dlg.ui";
    args.dlg_id  = "win1";

    app = gtk_application_new(app_id, app_flags);
    g_signal_connect(app, "activate", G_CALLBACK (app_activate), &args);
    g_signal_connect(app, "open", G_CALLBACK (app_open), &args);
    stat = g_application_run(G_APPLICATION(app), argc, argv);
    
    g_object_unref(app);
    g_print("END main\n");
    return stat;
}

