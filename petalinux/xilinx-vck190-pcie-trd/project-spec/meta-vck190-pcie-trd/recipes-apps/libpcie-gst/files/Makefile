APP = pciegst

LIBSOURCES=*.c
OUTS = *.o
NAME := pciegst
MAJOR = 1.0
MINOR = 1
VERSION = $(MAJOR).$(MINOR)

PCI_CFLAGS_PKG_CONFIG = $$(pkg-config --cflags  gstreamer-1.0 gstreamer-plugins-base-1.0 gstreamer-app-1.0 libdrm libv4l2 libmediactl)
PCI_LIBS_PKG_CONFIG = $$(pkg-config --libs gstreamer-1.0 gstreamer-plugins-base-1.0 gstreamer-app-1.0 gstreamer-video-1.0 libdrm libmediactl libv4l2)

all: lib$(NAME).so
lib$(NAME).so.$(VERSION): $(OUTS)
	$(CC) $(LDFLAGS) $(PCI_LIBS_PKG_CONFIG) $(OUTS)  -shared -Wl,-soname,lib$(NAME).so.$(MAJOR) -o lib$(NAME).so.$(VERSION)

lib$(NAME).so: lib$(NAME).so.$(VERSION)
	rm -f lib$(NAME).so.$(MAJOR) lib$(NAME).so
	ln -s lib$(NAME).so.$(VERSION) lib$(NAME).so.$(MAJOR)
	ln -s lib$(NAME).so.$(MAJOR) lib$(NAME).so

%.o: %.c
	$(CC) $(CFLAGS) $(PCI_CFLAGS_PKG_CONFIG) -c -fPIC $(LIBSOURCES)

clean:
	rm -rf *.o *.so *.so.*

