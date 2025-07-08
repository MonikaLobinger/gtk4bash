#!/bin/bash
UI="UI/twoway.ui"
ID="win1"
IN="/tmp/${0}.${$}.in"
OUT="/tmp/${0}.${$}.out"

WRAP="gtk4bash $@ -f $UI -m $ID "
WRAP="gtk4bash $@ -f $UI -m $ID -i $IN -o $OUT"

on_plus_clicked(){
    echo "get_textview_text textview1 a"
    echo "get_textview_text textview2 a"
    read a < $OUT
    read b < $OUT
    let "c = $a + $b"
    echo "set_textview_text textview3 $c"
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
