#!/bin/bash

UI_FILE="dlg.ui"
WRAP="gtk4bash -f $UI_FILE"

on_button1_clicked(){
    echo "==============on_button1_clicked======================"
}

on_togglebutton1_toggled(){
    echo "==============on_togglebutton1_toggled======================"
    echo toggled
}

on_window1_destroy(){
    echo "==============on_window1_destroy======================"
}

G_ENABLE_DIAGNOSTIC=1;$WRAP | while read line
do
    echo "LINE: $line"
    $line
done
