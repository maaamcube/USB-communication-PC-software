#ifndef USBENDPOINT_H
#define USBENDPOINT_H

#include <QString>
#include <QHash>
#include <QSharedPointer>
#include "am3_pool.h"
#include "am3_buffer_pool.h"
#include "libusb.h"


class USBInterface;
class Mail;
class USBEndpoint
{
    typedef enum {
        WAITING_FOR_COMPLETION,
        COMPLETED
    } TransferStatus;

    typedef QSharedPointer<libusb_transfer*> shared_transfer;
public:
    USBEndpoint(int endpoint,
                USBInterface* interface);

    static void LIBUSB_CALL transferCallback(libusb_transfer *xfr);

    bool transfer(QSharedPointer<Buffer<quint8>> buffer);
    bool transfer(QSharedPointer<Mail> mail);
    QSharedPointer<Buffer<quint8>> nextPart();
    void cancelCurrentTransfers();
    void changeStatus(libusb_transfer* xfr, TransferStatus status);
    void close();
    bool isClosed();
    bool prepareReceiving(BufferPool<quint8>* pool);
    bool prepareAll();


    shared_transfer getTransfer(libusb_transfer* xfr);
    QSharedPointer<Buffer<quint8>> getBuffer(shared_transfer xfr);
    int endpointNumber() const;

protected:

    USBInterface* usbInterface();
    shared_transfer find(libusb_transfer*);

private:
    bool isRxEndpoint();
    void releaseTransferRessources(libusb_transfer*);

private:
    Pool<libusb_transfer*> m_transfers;
    USBInterface* m_usbInterface;
    int m_endpointNumber;
    QHash<shared_transfer,TransferStatus> m_status;
    QHash<shared_transfer,QSharedPointer<Buffer<quint8>>> m_buffers;

    static bool isStart;
};

#endif // USBENDPOINT_H
