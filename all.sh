#!/bin/bash
for blp in `ls BLP/*.blp`; do
    echo "dlg.sh -f $(dirname $blp)/$(basename -s .blp $blp).ui"
    dlg.sh -f $(dirname $blp)/$(basename -s .blp $blp).ui
done
