#ifndef AM3_HARDWARE_INTERFACE_H
#define AM3_HARDWARE_INTERFACE_H

#include "am3_communication_interface.h"

class HardwareInterface : public CommunicationInterface
{
public:
    HardwareInterface(const CommunicationInterfaceInfos &infos);
    virtual ~HardwareInterface();

    virtual void receive(QSharedPointer<Mail> mail);
    virtual void rawReceive(const QSharedPointer<Buffer<quint8> > &buffer,quint8 hub);
    QSharedPointer<Buffer<quint8>> nextPart();

    virtual void transfer(const QSharedPointer<Buffer<quint8>>& buffer) = 0;
    void receiving(void *channel, const QSharedPointer<Buffer<quint8>>& buffer);
    void receivingFTDI(void *channel, const QSharedPointer<Buffer<quint8> > &buffer);
protected:
    QSharedPointer<Mail> m_current;
    //correct to up to quint16 because more than 255 parts
    QMap<quint16,QSharedPointer<Buffer<quint8>>> m_parts;
};

#endif // AM3_HARDWARE_INTERFACE_H
