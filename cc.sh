#!/bin/bash
#ls -l ~/test/cc.ui
#cp ~/test/cc.ui ./UI
cmd="gtk4bash $@ -f UI/cc.ui -m win1"
on_btn1_clicked(){
    echo "==============on_btn1_clicked======================"
}
on_btn2_clicked(){
    echo "==============on_btn2_clicked======================"
}
on_win1_destroy() {
    echo "==============on_win1_destroy======================"
}
G_ENABLE_DIAGNOSTIC=1;$cmd | while read line
do
    echo "LINE: $line"
    $line
done
