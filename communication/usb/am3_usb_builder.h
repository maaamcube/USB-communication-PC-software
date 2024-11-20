#ifndef AM3_US_BBUILDER_H
#define AM3_US_BBUILDER_H

#include "am3_usb_interface.h"

#include <QMap>
#include <QString>

class USBBuilder
{
public:
    USBBuilder();

    USBInterface *create(libusb_device *device);

private:
    QMap<QString,CommunicationInterfaceInfos> m_templates;
};

#endif // AM3_US_BBUILDER_H
