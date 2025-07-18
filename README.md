My English is not as good as any automatic translation, e.g. [Google translate](https://translate.google.com/?hl=de&sl=de&tl=en&op=translate).

# gtk4bash - Steuerung eines gtk4 Fenster mit der Bash

## Aufgegeben

Das Ziel war, UIs mit der Bash zu steuern. 

Da ich plane eine kompliziertere UI zu verwenden und da [Zenity auf Buster](https://manpages.debian.org/buster/zenity/zenity.1.en.html) noch gtk3 verwendet, 
wollte ich in völliger Unkenntnis der Komplexität diesen Wrapper schreiben.


Einfache Dialoge kann man auch mit Zenity machen. [Zenity](https://gitlab.gnome.org/GNOME/zenity) 4 verwendet Adwaita. Auf [Trixie](https://linuxnews.de/debian-13-trixie-rueckt-noch-einen-schritt-naeher/) wird es wohl Zenity 4 geben.


Für kompliziertere UIs ist Zenity nicht geeignet. Um diese mit Bash zu steuern, müßte ich große Teile des GTK4 Interfaces Wrappen. Es existieren schon einige [language bindings](https://www.gtk.org/docs/language-bindings/), darunter auch einige für Script Sprachen, unter anderem [für Python](https://amolenaar.pages.gitlab.gnome.org/pygobject-docs/). Ich werde für den Dialogteil meines Projekts Python verwenden.


Mein ursprüngliches Ansinnen ist nur so zu erklären, daß ich keine Ahnung hatte, wie komplex Gtk4 ist. Ich betrachte das Projekt als proof of concept, es wurde gezeigt, daß man gtk4 und bash direkt verbinden kann. 

Das Projekt wird nun archiviert.

## Das war der Plan

![](help/about.png)


[gtkwrap](https://github.com/abecadel/gtkwrap) für GTK4 angepasst.

[Beschreibung](help/help.md)

