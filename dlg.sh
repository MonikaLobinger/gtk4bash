#!/bin/bash
# Aufruf z.B. >>dlg.sh -f BLP/about.ui<< oder >>dlg.sh -f UI/oneway.ui<<
blank_name=""
dir=""
    while getopts :dvf: opt; do { case ${opt} in
        f) blank_name=$(basename -s .ui ${OPTARG});dir=$(dirname ${OPTARG});;
    esac } done
    OPTIND=1
cmd="gtk4bash -s $dir/$blank_name.css -m window $@";echo $cmd
# << 'BLP2UI-OUTCOMMENTED'
    #cp $dir/$blank_name.ui $dir/$blank_name.sv._$(date +"%y_%m_%d-%H-%M-%S")
    blueprint-compiler compile $dir/$blank_name.blp > $dir/$blank_name.ui
    linenr=`sed -n '/AdwStatusPage/=' $dir/$blank_name.ui`
    if [[ $linenr == "9" ]]; then
        sed -i '/AdwStatusPage/i \
        <object class=\"GtkApplicationWindow\" id=\"window\">\
        <property name=\"default-height\">700</property>\
        <property name=\"default-width\">800</property>\
        <signal name="destroy" handler="on_window_destroy"/>\
        <child>' $dir/$blank_name.ui
        sed -i '/\/interface/i \ \ <\/child>\n  <\/object>' $dir/$blank_name.ui
    fi
# BLP2UI-OUTCOMMENTED
on_button_clicked(){
    echo "==============on_button_clicked======================"
}
on_window_destroy() {
    echo "==============on_window_destroy======================"
}
G_ENABLE_DIAGNOSTIC=1;$cmd | while read line
do
    echo "LINE: $line"
    $line
done
