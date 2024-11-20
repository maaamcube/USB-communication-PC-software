#include "am3_usb_endpoint.h"
#include "am3_usb_interface.h"
#include "am3_log.h"
#include <QDebug>
#include <QFile>
#include <QTime>
#include <QThread>

bool USBEndpoint::isStart = true;


USBEndpoint::USBEndpoint(int endpoint,
                         USBInterface* interface)
    : m_transfers(10000)
{
    BEGIN_LOG("USBEndpoint::USBEndpoint");
    HW_DEBUG("Endpoint {:x} creation", endpoint);
    m_usbInterface = interface;
    m_endpointNumber = endpoint;
}

void USBEndpoint::transferCallback(libusb_transfer *xfr)
{
    auto endpoint = (USBEndpoint*) xfr->user_data;

    switch(xfr->status){
        case LIBUSB_TRANSFER_COMPLETED:
        if(endpoint->isRxEndpoint()){
            // RX case, we received data, we send it through interface
            auto t = endpoint->getTransfer(xfr);
            auto b = endpoint->getBuffer(t);
            endpoint->changeStatus(xfr,COMPLETED);
            endpoint->usbInterface()->receiving(endpoint,b);
        }else{
            // TX case, data has been transfered, we acknowledge reception
        }
        break;

    case LIBUSB_TRANSFER_STALL:
        qDebug() << "LIBUSB_TRANSFER_STALL\t" <<endpoint->usbInterface()->infos().m_hub <<"\t" << xfr <<"\t" <<QThread::currentThreadId();
        break;
    case LIBUSB_TRANSFER_NO_DEVICE:
        qDebug() << "LIBUSB_TRANSFER_NO_DEVICE\t" <<endpoint->usbInterface()->infos().m_hub <<"\t" << xfr <<"\t" << QThread::currentThreadId();
        endpoint->usbInterface()->error();
        break;
    case LIBUSB_TRANSFER_OVERFLOW:
        qDebug() << "LIBUSB_TRANSFER_OVERFLOW\t" <<endpoint->usbInterface()->infos().m_hub <<"\t" << xfr <<"\t" << QThread::currentThreadId();
        endpoint->usbInterface()->error();
        break;
    case LIBUSB_TRANSFER_ERROR:
        qDebug() << "LIBUSB_TRANSFER_ERROR\t" <<endpoint->usbInterface()->infos().m_hub <<"\t" << xfr <<"\t" << QThread::currentThreadId();
        endpoint->usbInterface()->error();
        break;
    case LIBUSB_TRANSFER_CANCELLED:
        qDebug() << "LIBUSB_TRANSFER_CANCELLED\t" <<endpoint->usbInterface()->infos().m_hub <<"\t" << xfr <<"\t" << QThread::currentThreadId();
        break;
    case LIBUSB_TRANSFER_TIMED_OUT:
        qDebug() << "LIBUSB_TRANSFER_TIMED_OUT\t" <<endpoint->usbInterface()->infos().m_hub <<"\t" << xfr <<"\t" << QThread::currentThreadId();
        break;
    }
    endpoint->releaseTransferRessources(xfr);
}

bool USBEndpoint::transfer(QSharedPointer<Buffer<quint8> > buffer)
{
    // Ensure endpoint is TX type
        // Ensure endpoint is not in use
        if(!isRxEndpoint() && !m_status.isEmpty()){
            auto key = m_status.keys().first();
            changeStatus(*key.data(),COMPLETED);
        }
        if(m_status.isEmpty() || isRxEndpoint()){
            //Get transfer container
            auto transfer = m_transfers.get();
            if(!transfer.isNull()){
                // Allocate ressources
                auto xfr = libusb_alloc_transfer(0);

                // Get mail data
                auto data_buffer = buffer->data();

                // Fill libusb transfer
                libusb_fill_bulk_transfer(xfr,
                                          m_usbInterface->handle(),
                                          m_endpointNumber,
                                          (unsigned char*) data_buffer,
                                          buffer->size(),
                                          USBEndpoint::transferCallback,
                                          (void*) this,
                                          0);

                // Check transfer validity
                if(xfr != nullptr){

#ifdef COMM_MODE
    //qDebug() << QTime::currentTime().toString("hh:mm:ss.zzz") << "\ttransfer\t" <<usbInterface()->infos().m_hub <<"\t" << xfr <<"\t" << QThread::currentThreadId();
#endif

                    // Save transfer in allocated buffer
                    (*transfer.data()) = xfr;
                    auto error = libusb_submit_transfer(xfr);
                    if(error < 0){
                        // Error on submition, release ressources
                        libusb_free_transfer(xfr);
                        HW_ERROR("Transfer failed for endpoint {:x}: {}",
                                  m_endpointNumber,
                                  libusb_strerror(static_cast<libusb_error>(error)));

                        return false;
                    }
                    // Everything is ok, keep buffer reference
                    m_buffers[transfer] = buffer;
                    m_status[transfer] = WAITING_FOR_COMPLETION;
                    return true;
                }
            }
            else{
                return false;
            }
        }
        else{
            return false;
        }

    return false;
}

void USBEndpoint::cancelCurrentTransfers()
{

}

void USBEndpoint::changeStatus(libusb_transfer *xfr, USBEndpoint::TransferStatus status)
{
    BEGIN_LOG("USBEndpoint::changeStatus");
   auto t = getTransfer(xfr);
    if(status == COMPLETED){
        m_status.remove(t);
        m_buffers.remove(t);
        //qDebug() << "status completed" << m_status.size();
        HW_DEBUG("Release transfer {}",(qint64)xfr);
    }
    else
        m_status[t] = status;
}

void USBEndpoint::close()
{
    //TODO
}

bool USBEndpoint::isClosed()
{
    //TODO
    return true;
}

bool USBEndpoint::prepareReceiving(BufferPool<quint8> *pool)
{
    while(m_status.size()<10){
        auto buffer = pool->get();
        if(buffer.isNull())
            return false;
        auto ok = transfer(buffer);
        if(!ok)
            return false;
    }
    return true;
}

USBEndpoint::shared_transfer USBEndpoint::getTransfer(libusb_transfer *xfr)
{
    shared_transfer t;
    for(auto it=m_status.begin();it!=m_status.end();it++){
        if((*it.key().data()) == xfr)
            t = it.key();

    }
    return t;
}

QSharedPointer<Buffer<quint8> > USBEndpoint::getBuffer(USBEndpoint::shared_transfer xfr)
{
    return m_buffers[xfr];
}

bool USBEndpoint::prepareAll()
{
    return true;
}

USBInterface *USBEndpoint::usbInterface()
{
    return m_usbInterface;
}

bool USBEndpoint::isRxEndpoint()
{
    return (m_endpointNumber & 0x80) == 0x80;
}

void USBEndpoint::releaseTransferRessources(libusb_transfer *xfr)
{
    libusb_free_transfer(xfr);
}

int USBEndpoint::endpointNumber() const
{
    return m_endpointNumber;
}
