#include "am3_usb_interface.h"
#include "am3_devices_defines.h"
#include "am3_log.h"

#include <QFile>

USBInterface::USBInterface(libusb_device_handle *handle,
                           const CommunicationInterfaceInfos& infos)
    : HardwareInterface(infos),m_handle(nullptr)
{
    BEGIN_LOG("USBInterface::USBInterface");
    m_handle = handle;


     libusb_claim_interface(m_handle,0);
     for(auto endpoint : m_infos.m_rx){
         m_rx[endpoint] = new USBEndpoint(endpoint,this);
         for(auto i=0;i<10;i++)
            m_rx[endpoint]->prepareReceiving(&m_pool);
     }

     for(auto endpoint : m_infos.m_tx){
         m_tx[endpoint] = new USBEndpoint(endpoint,this);
     }

}

USBInterface::~USBInterface()
{
    libusb_release_interface(m_handle,0);
    libusb_close(m_handle);
}

void USBInterface::rx(QSharedPointer<Mail> mail)
{
    Q_UNUSED(mail)
}

void USBInterface::linkDown()
{
     // TODO
}

bool USBInterface::isMailAccepted(const Mail::MailType &type) {
    Q_UNUSED(type)
    return true;
}

void USBInterface::transfer(const QSharedPointer<Buffer<quint8> > &buffer)
{
    if ((m_current->type() == Mail::DEVICE_CALIBRATE) || (m_current->type() == Mail::DEVICE_GET_CALIBRATION))
    {
        auto data_buffer = buffer->data();
        data_buffer[0] = 50;
        data_buffer[1] = 171;
    }

    m_tx.first()->transfer(buffer);
}

void USBInterface::prepareReceiving(void *channel)
{
    auto endpoint = (USBEndpoint*) channel;
    endpoint->prepareReceiving(&m_pool);
}

void USBInterface::receiving(void *channel, const QSharedPointer<Buffer<quint8> > &buffer)
{
    prepareReceiving(channel);
    HardwareInterface::receiving(channel,buffer);
}


libusb_device_handle *USBInterface::handle()
{
    return m_handle;
}
