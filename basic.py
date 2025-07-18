#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import gi
gi.require_version("Gtk", "4.0")
from gi.repository import Gtk

def on_activate(app):
    win = Gtk.ApplicationWindow(application=app)
    win.set_title("GTK4 Beispiel")
    win.set_default_size(300, 200)
    btn = Gtk.Button(label="Klick mich!")
    btn.connect('clicked', lambda x: win.close())
    win.set_child(btn)
    win.present()

app = Gtk.Application(application_id='org.gtk.Example')
app.connect('activate', on_activate)
app.run(None)