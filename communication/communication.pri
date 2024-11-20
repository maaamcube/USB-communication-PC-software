INCLUDEPATH += $$PWD
INCLUDEPATH +=  $$PWD/usb/ \
                $$PWD/mailbox/



HEADERS +=  $$PWD/mailbox/am3_mail.h \
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

WITH_FTD_32_BIT {
HEADERS +=$$PWD/usb/am3_ftdi_builder.h \
    $$PWD/usb/am3_ftdi_central.h \
    $$PWD/usb/am3_ftdi_endpoint.h \
    $$PWD/usb/am3_ftdi_interface.h
}

SOURCES +=  $$PWD/mailbox/am3_mail.cpp \
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

WITH_FTD_32_BIT {
SOURCES +=$$PWD/usb/am3_ftdi_builder.cpp \
$$PWD/usb/am3_ftdi_central.cpp \
$$PWD/usb/am3_ftdi_endpoint.cpp \
$$PWD/usb/am3_ftdi_interface.cpp
}
