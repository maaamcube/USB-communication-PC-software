#ifndef AM3_VIRTUAL_INTERFACE_H
#define AM3_VIRTUAL_INTERFACE_H

#include "am3_communication_interface.h"

class VirtualInterface : public CommunicationInterface
{
public:
    using CommunicationInterface::create;
    VirtualInterface(const CommunicationInterfaceInfos& infos);

    virtual QSharedPointer<Mail> create(Calibration** calibration) = 0;
};

#endif // AM3_VIRTUAL_INTERFACE_H
