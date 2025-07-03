#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <libintl.h>


#include <gtk/gtk.h>

GtkBuilder *g_builder;
GtkWidget *g_window;
GtkApplication *g_app;

void ok_button_clicked(GObject *object, gpointer user_data){
    g_print("START ok_button_clicked\n");
    g_print("END ok_button_clicked\n");
}

static void app_activate(GtkApplication *app, gpointer user_data) {
    g_print("START app_activate\n");
    GtkBuilder *builder;
    GtkWidget *dialog; // GtkDialog deprecated with GTK4.10
    char *filename="dlg.ui";
    char *main_object="win1";


    builder = gtk_builder_new_from_file (filename);
    //g_builder = builder;
    dialog = GTK_WIDGET (gtk_builder_get_object (builder, main_object));
    gtk_window_set_application (GTK_WINDOW (dialog), GTK_APPLICATION (app));
    //g_signal_connect (btn, "clicked", G_CALLBACK (ok_button_clicked), NULL);
    //g_window = window;
    //gtk_window_set_transient_for(GTK_WINDOW (dialog), GTK_WINDOW (appwin));
    gtk_window_present (GTK_WINDOW (dialog));

    //g_object_unref (appwin);
    //g_object_unref (builder);
    g_print("END app_activate\n");
}

int main(int argc, char **argv) {
    g_print("START main\n");
    GtkApplication *app;
    int stat;

    app = gtk_application_new ("daheim.oskopia.gtk4bash", G_APPLICATION_DEFAULT_FLAGS);
    //g_app = app;
    g_signal_connect (app, "activate", G_CALLBACK (app_activate), NULL);
    stat =g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);
    g_print("END main\n");
    return stat;
}

