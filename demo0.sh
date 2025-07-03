#!/bin/bash

GLADE_FILE="demo1.glade"
GTK_WRAP="gtk4bash -f $GLADE_FILE"

on_close_button_clicked(){
    echo "==============on_close_button_clicked======================"
}
on_ok_button_clicked(){
    echo "==============on_ok_button_clicked======================"
}
on_win1_destroy() {
    echo "==============on_win1_destroy======================"
}
G_ENABLE_DIAGNOSTIC=1;gtk4bash -f dlg.ui -m win1| while read line
do
    echo "LINE: $line"
    $line
done
