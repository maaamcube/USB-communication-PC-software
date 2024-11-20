INCLUDEPATH += $$PWD
INCLUDEPATH +=  $$PWD/usb/ \
                $$PWD/mailbox/


INCLUDEPATH += /usr/local/opt/libusb/lib/libusb-1.0_include
DEPENDPATH += /usr/local/opt/libusb/lib/ibusb-1.0_include
#LIBS += /Users/amcube/footlyser6mac/Footlyser6/libusb-1.0.0.dylib
LIBS += /usr/local/opt/libusb/lib/libusb-1.0.a

INCLUDEPATH += /Users/amcube/footlyser6mac/ftd2x
DEPENDPATH += /Users/amcube/footlyser6mac/ftd2x
#LIBS += /Users/amcube/footlyser6mac/ftd2x/build/libftd2xx.1.4.24.dylib
#LIBS += -L/usr/local/opt/libam3/ -lftd2xx
LIBS += /usr/local/opt/libusb/lib/libftd2xx.a
#INCLUDEPATH += /Users/amcube/footlyser6mac/ftd2x
#DEPENDPATH += /Users/amcube/footlyser6mac/ftd2x
##LIBS += /Users/amcube/footlyser6mac/ftd2x/build/libftd2xx.1.4.24.dylib
##LIBS += /Users/amcube/footlyser6mac/FOOTLYSER6_INSTALL/packages/Footlyser6.root/data/Footwork_Edition.app/Contents/MacOS/libftd2xx.dylib
#LIBS += -L$$OUT_PWD/ -lftd2xx
#PRE_TARGETDEPS +=$$OUT_PWD/libftd2xx.dylib


HEADERS +=  $$PWD/mailbox/am3_mail.h \
    $$PWD/usb/am3_ftdi_builder.h \
    $$PWD/usb/am3_ftdi_central.h \
    $$PWD/usb/am3_ftdi_endpoint.h \
    $$PWD/usb/am3_ftdi_interface.h \
            $$PWD/usb/am3_usb_endpoint.h \
            $$PWD/usb/am3_usb_interface.h \
            $$PWD/usb/am3_usb_central.h \
            $$PWD/am3_communication_interface.h \
            $$PWD/am3_communication_link.h \
            $$PWD/am3_communication_central.h \
            $$PWD/am3_hub.h \
            $$PWD/usb/am3_usb_builder.h \
            $$PWD/mailbox/am3_mail_central.h \
            $$PWD/am3_hardware_interface.h \
            $$PWD/am3_virtual_interface.h \
    $$PWD/am3_communication_wrapper.h

SOURCES +=  $$PWD/mailbox/am3_mail.cpp \
    $$PWD/usb/am3_ftdi_builder.cpp \
    $$PWD/usb/am3_ftdi_central.cpp \
    $$PWD/usb/am3_ftdi_endpoint.cpp \
    $$PWD/usb/am3_ftdi_interface.cpp \
            $$PWD/usb/am3_usb_endpoint.cpp \
            $$PWD/usb/am3_usb_interface.cpp \
            $$PWD/usb/am3_usb_central.cpp \
            $$PWD/am3_communication_interface.cpp \
            $$PWD/am3_communication_link.cpp \
            $$PWD/am3_communication_central.cpp \
            $$PWD/am3_hub.cpp \
            $$PWD/usb/am3_usb_builder.cpp \
            $$PWD/mailbox/am3_mail_central.cpp \
            $$PWD/am3_hardware_interface.cpp \
            $$PWD/am3_virtual_interface.cpp \
    $$PWD/am3_communication_wrapper.cpp
