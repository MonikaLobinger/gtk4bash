#!/bin/bash
UI="UI/twoway.ui"
ID="win1"
IN="/tmp/${0}.${$}.in"
OUT="/tmp/${0}.${$}.out"

WRAP="gtk4bash $@ -f $UI -m $ID "
WRAP="gtk4bash $@ -f $UI -m $ID -i $IN -o $OUT"

function get_text {
    widget_id=$1
    buf=
    itera=
    itere=
    val=
    echo "gtk_text_view_get_buffer $widget_id" > $IN
    read buf < $OUT
    echo "newGtkTextIter" > $IN
    echo "newGtkTextIter" > $IN
    read itera < $OUT
    read itere < $OUT
    echo "gtk_text_buffer_get_start_iter $buf $itera" > $IN
    echo "gtk_text_buffer_get_end_iter $buf $itere" > $IN
    echo "gtk_text_buffer_get_text $buf $itera $itere 0" > $IN
    read val < $OUT
    echo "free" > $IN
    echo $val
}

function set_text {
    widget_id=$1
    text=$2
    buf=
    echo "gtk_text_view_get_buffer $widget_id"
    read buf < $OUT
    echo "gtk_text_buffer_set_text $buf $text -1"
    echo "free"
} > $IN

on_plus_clicked(){
    val1=`get_text textview1`
    val2=`get_text textview2`
    let "sum = $val1 + $val2"
    set_text textview3 $sum
}

on_btn111_clicked(){
    echo "==============on_btn111_clicked======================"
}
on_btn112_clicked(){
    echo "==============on_btn112_clicked======================"
}
on_btn113_clicked(){
    echo "==============on_btn113_clicked======================"
}
on_btn114_clicked(){
    echo "==============on_btn114_clicked======================"
}
on_btn115_clicked(){
    echo "==============on_btn115_clicked======================"
}
G_ENABLE_DIAGNOSTIC=1;$WRAP | while read line
do
    echo "LINE: $line"
    $line
done
