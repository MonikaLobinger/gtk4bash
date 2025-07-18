#!/usr/bin/env python3

import sys
import gi
gi.require_version('Gtk', '4.0')
gi.require_version('Adw', '1')
from gi.repository import Gtk, Adw

class MainWindow(Gtk.ApplicationWindow):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.box1 = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.set_child(self.box1)

        self.button1 = Gtk.Button(label="Hello")
        self.box1.append(self.button1)
        self.button1.connect('clicked', self.hello)

        self.button2 = Gtk.Button(label="About")
        self.box1.append(self.button2)
        self.button2.connect('clicked', self.open_about_window)

    def hello(self, button):
        print("Hello world")

    def open_about_window(self, widget): 
        dialog = Adw.AboutWindow(transient_for=app.get_active_window())
        dialog.set_application_name("App name")
        dialog.set_version("1.0")
        dialog.set_developer_name("Developer")
        dialog.set_license_type(Gtk.License(Gtk.License.GPL_3_0))
        dialog.set_comments("Adw about Window example")
        dialog.set_website("https://github.com/Tailko2k/GTK4PythonTutorial")
        dialog.set_issue_url("https://github.com/Tailko2k/GTK4PythonTutorial/issues")
        dialog.add_credit_section("Contributors", ["Name1 url"])
        dialog.set_translator_credits("Name1 url")
        dialog.set_copyright("© 2022 developer")
        dialog.set_developers(["Developer"])
        dialog.set_application_icon("com.github.devname.appname")  # icon must be uploaded in ~/.local/share/icons or /usr/share/icons
        dialog.set_visible(True)

class MyApp(Adw.Application):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.connect('activate', self.on_activate)

    def on_activate(self, app):
        self.win = MainWindow(application=app)
        self.win.present()

app = MyApp(application_id="com.example.GtkApplication")
app.run(sys.argv)


