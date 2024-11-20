#ifdef WITH_FTD_32_BIT

#ifndef AM3_FTDIBUILDER_H
#define AM3_FTDIBUILDER_H

#include "am3_ftdi_interface.h"

#include <QMap>
#include <QString>

class FTDIBuilder
{
public:
    FTDIBuilder();

    FTDIInterface *create(int numDev);

private:
    QMap<QString,CommunicationInterfaceInfos> m_templates;
};

#endif // AM3_FTDIBUILDER_H

#endif
