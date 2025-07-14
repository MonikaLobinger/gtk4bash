#!/bin/bash
UI="UI/twoway.ui"
ID="win1"
IN="/tmp/${0}.${$}.in"
OUT="/tmp/${0}.${$}.out"

WRAP="gtk4bash $@ -f $UI -m $ID "
WRAP="gtk4bash $@ -f $UI -m $ID -i $IN -o $OUT"

on_plus_clicked(){
    nobuf=
    buf1=
    buf2=
    buf3=
    iter1a=
    iter1e=
    iter2a=
    iter2e=
    val1=
    val2=
    echo "gtk_text_view_get_buffer textview1"
    echo "gtk_text_view_get_buffer textview2"
    read buf1 < $OUT
    read buf2 < $OUT
    echo "newGtkTextIter"
    echo "newGtkTextIter"
    echo "newGtkTextIter"
    echo "newGtkTextIter"
    read iter1a < $OUT
    read iter1e < $OUT
    read iter2a < $OUT
    read iter2e < $OUT
    echo "gtk_text_buffer_get_start_iter $buf1 $iter1a"
    echo "gtk_text_buffer_get_end_iter $buf1 $iter1e"
    echo "gtk_text_buffer_get_start_iter $buf2 $iter2a"
    echo "gtk_text_buffer_get_end_iter $buf2 $iter2e"
    echo "gtk_text_buffer_get_text $buf1 $iter1a $iter1e 0"
    echo "gtk_text_buffer_get_text $buf2 $iter2a $iter2e 0"
    read val1 < $OUT
    read val2 < $OUT
    let "sum = $val1 + $val2"
    echo "gtk_text_view_get_buffer textview3"
    read buf3 < $OUT
    echo "gtk_text_buffer_set_text $buf3 $sum -1"
} > $IN

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
