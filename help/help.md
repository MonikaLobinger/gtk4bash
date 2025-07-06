Steuerung eines GTK4 Fenster mit der Bash
=========================================
My English is not as good as any automatic translation, e.g. [Google translate](https://translate.google.com/?hl=de&sl=de&tl=en&op=translate).
# 1. gtk4bash
Gtk4bash ist ein Tool um einen GTK4 Dialog mit Bash zu steuern. Das Projekt [gtkwrap](https://github.com/abecadel/gtkwrap) funktioniert nur mit GTK2 und GTK3. Dessen Quellen wurden für GTK4 angepasst. 

# 2. Installation
Das Projekt besteht im wesentlichen aus einer C Datei und einem Makefile. Die GTK4 Entwicklungsbibliotheken müssen installiert sein. Dann erzeugt make eine ausführbare Datei. 

# 3. Verwendung
Die mit make erzeugte ausführbare Datei gtk4bash kann mit den Scripten getestet werden. 
# 3.1 Proof of Concept
# 3.1.1 Oneway
oneway.sh
![](oneway.png)
# 3.1.2 Twoway
twoway.sh
![](twoway.png)
# 3.1.3 Loadstyle
loadstyle.sh
![](loadstyle.png)
# 3.2 Systematisch Adwaita