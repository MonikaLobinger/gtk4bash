#!/bin/bash
blueprint-compiler compile UI/test.blp > UI/test.ui
cmd="gtk4bash $@ -f UI/test.ui -s UI/test.css -m window"
on_btn1_clicked(){
    echo "==============on_btn1_clicked======================"
}
on_win1_destroy() {
    echo "==============on_win1_destroy======================"
}
on_txt1_insert-at-cursor() {
    echo "==============on_txt1_insert-at-cursor======================"
}
G_ENABLE_DIAGNOSTIC=1;$cmd | while read line
do
    echo "LINE: $line"
    $line
done
