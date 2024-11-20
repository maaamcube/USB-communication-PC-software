#ifdef WITH_FTD_32_BIT

#ifndef FTDIENDPOINT_H
#define FTDIENDPOINT_H

#include <QString>
#include <QHash>
#include <QSharedPointer>
#include "am3_pool.h"
#include "am3_buffer_pool.h"
#include "ftd2xx.h"

#ifdef Q_OS_MACOS
#define ERROR_IO_PENDING 997L   //dderror
#endif

class FTDIInterface;
class Mail;
class FTDIEndpoint
{
    typedef enum {
        WAITING_FOR_COMPLETION,
        COMPLETED
    } TransferStatus;

    typedef QSharedPointer<FT_HANDLE> shared_transfer;
public:
    FTDIEndpoint(int endpoint, FTDIInterface* interface);

    bool transfer(QSharedPointer<Buffer<quint8>> buffer);
    bool receiveFTDI(QSharedPointer<Buffer<quint8> > buffer);
    bool transfer(QSharedPointer<Mail> mail);
    QSharedPointer<Buffer<quint8>> nextPart();
    void cancelCurrentTransfers();
    void changeStatus(FT_HANDLE xfr, TransferStatus status);
    void close();
    bool isClosed();
    bool prepareReceiving();//BufferPool<quint8>* pool);
    bool prepareAll();


    shared_transfer getTransfer(FT_HANDLE xfr);
    QSharedPointer<Buffer<quint8>> getBuffer(shared_transfer xfr);
    int endpointNumber() const;

    bool computeLine();

    static bool getIsreceiveFTDIrunning();

protected:

    FTDIInterface* ftdiInterface();
    shared_transfer find(FT_HANDLE);

private:
    bool isRxEndpoint();
    void releaseTransferRessources(FT_HANDLE);

private:
    Pool<FT_HANDLE> m_transfers;
    FTDIInterface* m_ftdiInterface;
    int m_endpointNumber;
    QHash<shared_transfer,TransferStatus> m_status;
    QHash<shared_transfer,QSharedPointer<Buffer<quint8>>> m_buffers;

    int LastFrameCount, LastEllapsedTime;
    QList<quint8> dataline;
    QList<quint8> pBufferFTDI, newBufferFTDI;
    int m_height, m_width;
public:
    static bool isreceiveFTDIrunning;
};

#endif // FTDIENDPOINT_H

#endif
