import pygtk
pygtk.require('2.0')
import gtk
import gobject

gtk.widget_set_default_direction(gtk.TEXT_DIR_LTR)

window = gtk.Window(gtk.WINDOW_TOPLEVEL)
window.set_title("Hello Buttons!")
window.connect("delete_event", lambda *args: False)
window.connect("destroy", lambda *args: gtk.main_quit())
window.set_border_width(10)
box1 = gtk.HBox(False, 20)
window.add(box1)
box1.show()
button1 = gtk.Button("Hello World")

def hello(widget, data=None):
    print ("Hello World", widget, data)

button1.connect("clicked", hello, None)
button1.show()
box1.pack_start(button1, True, True, 0)

button2 = gtk.Button("Button 2")
button2.show()
box1.pack_start(button2, True, True, 0)
print dir(box1)

window.show()
gtk.main()

#gobject.timeout_add(interval, function, ...)
#gobject.io_add_watch(source, condition, callback)
#gobject.idle_add(callback, ...)



