#!/bin/bash
cmd="gtk4bash $@ -f UI/oneway.ui -m win1"
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
