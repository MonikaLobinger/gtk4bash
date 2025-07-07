#!/bin/bash
# Aufruf z.B. >>dlg.sh -f BLP/about.ui<< oder >>dlg.sh -f UI/oneway.ui<<
DEBUG=0
VERBOSE=0
IN="/tmp/${0}.${$}.in"
OUT="/tmp/${0}.${$}.out"
WIN_ID="window"
blank_name=""
dir=""
    while getopts :dvf: opt; do { case ${opt} in
        d) DEBUG=1;;
        v) VERBOSE=1;;
        f) blank_name=$(basename -s .ui ${OPTARG});dir=$(dirname ${OPTARG});;
    esac } done
    OPTIND=1
cmd="gtk4bash -s $dir/$blank_name.css -m $WIN_ID -i $IN -o $OUT $@";echo $cmd
# << 'BLP2UI-OUTCOMMENTED'
    #cp $dir/$blank_name.ui $dir/$blank_name.sv._$(date +"%y_%m_%d-%H-%M-%S")
    blueprint-compiler compile $dir/$blank_name.blp > $dir/$blank_name.ui
    linenr=`sed -n '/AdwStatusPage/=' $dir/$blank_name.ui`
    if [[ $linenr == "9" ]]; then
        sed -i '/AdwStatusPage/i \
        <object class=\"GtkApplicationWindow\" id=\"abcdef\">\
        <property name=\"default-height\">700</property>\
        <property name=\"default-width\">800</property>\
        <signal name="destroy" handler="on_window_destroy"/>\
        <child>' $dir/$blank_name.ui
        sed -i "s/abcdef/$WIN_ID/g"  $dir/$blank_name.ui
        sed -i '/\/interface/i \ \ <\/child>\n  <\/object>' $dir/$blank_name.ui
    fi
# BLP2UI-OUTCOMMENTED
################################################################################
function flatpak_get_user_information {
    if (( DEBUG )); then echo "FUNCTION extern_get_user_information" >> debug.txt; fi
    _reason=$1
    cbk=$2
    $cbk
}
function flatpak_get_user_information_finish {
    local -n flatpak_result=$1
    flatpak_result="user information"
}
#### General Callbacks #########################################################
on_window_destroy() {
    echo "==============on_window_destroy======================"
}
#### about Callbacks ###########################################################
on_button_clicked(){
    echo "==============on_button_clicked======================"
}
#### account Callbacks #########################################################
on_information_received() {
    if (( DEBUG )); then echo "FUNCTION on_information_received" >> debug.txt; fi
    result=""
    flatpak_get_user_information_finish result
    if (( DEBUG )); then echo "   result: $result" >> debug.txt; fi
}
on_account_request_button_clicked() {
    if (( DEBUG )); then echo "FUNCTION on_account_request_button_clicked" >> debug.txt; fi
    echo "account_entry gtk_editable_get_text reason"
    read reason < $OUT
    if (( DEBUG )); then echo "    reason: $reason" >> debug.txt; fi
    flatpak_get_user_information reason on_information_received
} > $IN

G_ENABLE_DIAGNOSTIC=1
echo "$cmd" > debug.txt
#echo "$WIN_ID set_window_title abcd" > $IN|${cmd} | while read line
${cmd} |
while read line
do
    echo "LINE: $line"
    $line
done
