#ifndef AM3_USBINTERFACE_H
#define AM3_USBINTERFACE_H

#include "am3_hardware_interface.h"
#include "am3_usb_endpoint.h"


class USBInterface : public HardwareInterface
{
    public:
        USBInterface(libusb_device_handle *handle,
                     const CommunicationInterfaceInfos &infos);
        virtual ~USBInterface();
        virtual void rx(QSharedPointer<Mail> mail);
        virtual void linkDown();
        virtual bool isMailAccepted(const Mail::MailType& type);

        virtual void transfer(const QSharedPointer<Buffer<quint8>>& buffer);
        void prepareReceiving(void *channel);
        void receiving(void *channel, const QSharedPointer<Buffer<quint8>>& buffer);
        libusb_device_handle* handle();

private:
        libusb_device_handle* m_handle;
        QMap<quint32,USBEndpoint*> m_rx;
        QMap<quint32,USBEndpoint*> m_tx;

};

#endif // AM3_USBINTERFACE_H
