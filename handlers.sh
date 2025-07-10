#!/bin/bash
UI="UI/handlers.ui"
ID="win1"
CSS="UI/handlers.css"
IN="/tmp/${0}.${$}.in"
OUT="/tmp/${0}.${$}.out"

WRAP="gtk4bash $@ -f $UI -m $ID -s $CSS -i $IN -o $OUT"

on_button_gtk_window_get_default_icon_name_clicked(){
    val=
    echo "gtk_window_get_default_icon_name"
    read val < $OUT
    echo " set_label_text  result  $val"
} > $IN

on_button_gtk_widget_get_name_clicked(){
    val=
    echo "gtk_widget_get_name gtk_widget_get_name"
    read val < $OUT
    echo "set_label_text result $val"
} > $IN

on_button_gtk_window_set_auto_startup_notification_clicked() {
    echo "gtk_window_set_auto_startup_notification 1"
} > $IN

on_button_gtk_window_get_set_title_clicked(){
    val=
    echo "|gtk_window_get_title|win1"
    read val < $OUT
    echo "|set_label_text|result|$val"
    echo "|gtk_window_set_title|win1|Neuer Titel"
} > $IN

on_button_gtk_window_close_clicked(){
    echo "gtk_window_close win1"
} > $IN

on_button_gtk_window_toggle_fullscreen_clicked(){
    echo "gtk_window_is_fullscreen win1"
    read val < $OUT
    echo "set_label_text result $val"
    if (( val )); then
        echo "gtk_window_unfullscreen win1"
    else
        echo "gtk_window_fullscreen win1"
    fi
} > $IN

on_button_gtk_window_toggle_maximize_clicked(){
    echo "gtk_window_is_maximized win1"
    read val < $OUT
    echo "set_label_text result $val"
    if (( val )); then
        echo "gtk_window_unmaximize win1"
    else
        echo "gtk_window_maximize win1"
    fi
} > $IN

on_button_gtk_window_minimize_clicked(){
    echo "gtk_window_minimize win1"
} > $IN

G_ENABLE_DIAGNOSTIC=1;$WRAP | while read line
do
    echo "LINE: $line"
    $line
done
