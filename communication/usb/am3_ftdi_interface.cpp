#ifdef WITH_FTD_32_BIT

#include "am3_ftdi_interface.h"
#include "am3_devices_defines.h"
#include "am3_log.h"
#include "am3_ftdi_central.h"
#include <am3_hub.h>

#include <QFile>

FTDIInterface::FTDIInterface(const CommunicationInterfaceInfos& infos)
    : HardwareInterface(infos)
{
#ifdef Q_OS_MACOS
    m_handle = (FT_HANDLE) INVALID_HANDLE_VALUE;
#else
    m_handle = INVALID_HANDLE_VALUE;
#endif
    m_serial = infos.serial;

}

void FTDIInterface::init()
{
    for(auto endpoint : m_infos.m_rx){
        m_rx[endpoint] = new FTDIEndpoint(endpoint,this);
        //         for(auto i=0;i<10;i++)
        //            m_rx[endpoint]->prepareReceiving();
    }

    for(auto endpoint : m_infos.m_tx){
        m_tx[endpoint] = new FTDIEndpoint(endpoint,this);
    }

}

void FTDIInterface::loopReceiving()
{
    for(auto endpoint : m_infos.m_rx){
        if (!FTDIEndpoint::getIsreceiveFTDIrunning()) m_rx[endpoint]->prepareReceiving();
    }
}

QString FTDIInterface::serial() const
{
    return m_serial;
}

FTDIInterface::~FTDIInterface()
{
    stopInterface();
    FT_ResetDevice(m_handle);
    FT_Close(m_handle);
}

QString FTDIInterface::getSerialNumber()
{
    FT_STATUS ftStatus;
    FT_HANDLE ftHandle = handle();

#ifdef Q_OS_MACOS
    if (((long int )ftHandle) != INVALID_HANDLE_VALUE)
#else
    if (ftHandle != INVALID_HANDLE_VALUE)
#endif
    {
        DWORD BytesWritten;
        char TxBuffer[2]; // Contains data to write to device

        TxBuffer[0] = 'G';
        TxBuffer[1] = 'S';

        ftStatus =  FT_Purge(ftHandle,FT_PURGE_TX);
        ftStatus = ftStatus  &  FT_Purge(ftHandle,FT_PURGE_RX);

        ftStatus = ftStatus  & FT_Write(ftHandle, TxBuffer, 2, &BytesWritten);
        // sleep answer time
        QThread::msleep(1000);

        DWORD RxBytes = 0;
        ftStatus = FT_GetQueueStatus(ftHandle,&RxBytes);

        if (ftStatus == FT_OK)
        {
            DWORD BytesReceived;
            if ( RxBytes > 0 )
            {
                QString serialNb;
                char *RxBuffer = new char[RxBytes];
                ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
                for (int c = 0 ; c < BytesReceived ; c++)
                {
                    serialNb.append(QChar(RxBuffer[c]));
                }
                delete[] RxBuffer;
                m_serial = serialNb;
                m_infos.serial = serialNb;
            }
        }
        else
        {
        }
        ftStatus =  FT_Purge(ftHandle,FT_PURGE_TX);
        ftStatus = ftStatus  &  FT_Purge(ftHandle,FT_PURGE_RX);
    }
    else
    {
    }

    return m_serial;
}

void FTDIInterface::stopInterface()
{
    FT_STATUS ftStatus;
    FT_HANDLE ftHandle = handle();

#ifdef Q_OS_MACOS
    if (((long int )ftHandle) != INVALID_HANDLE_VALUE)
#else
    if (ftHandle != INVALID_HANDLE_VALUE)
#endif
    {
        const DWORD FT_In_Buffer_Size = 1<<16;
        DWORD BytesWritten;
        char TxBuffer[FT_In_Buffer_Size]; // Contains data to write to device

        TxBuffer[0] = 'B';

        ftStatus =  FT_Purge(ftHandle,FT_PURGE_TX);
        ftStatus = ftStatus  &  FT_Purge(ftHandle,FT_PURGE_RX);

        ftStatus = ftStatus  & FT_Write(ftHandle, TxBuffer, 1, &BytesWritten);

        if (ftStatus == FT_OK)
        {
            FTDIEndpoint::isreceiveFTDIrunning = false;
        }
        else
        {
            //qDebug()<<"stopInterface KO";
        }
        ftStatus =  FT_Purge(ftHandle,FT_PURGE_TX);
        ftStatus = ftStatus  &  FT_Purge(ftHandle,FT_PURGE_RX);
    }
    else
    {
        //qDebug()<<"stopInterface KO 2";
    }
}


void FTDIInterface::rx(QSharedPointer<Mail> mail)
{
    Q_UNUSED(mail)
}

void FTDIInterface::linkDown()
{
    // TODO
}

bool FTDIInterface::isMailAccepted(const Mail::MailType &type) {
    Q_UNUSED(type)
    return true;
}

void FTDIInterface::transfer(const QSharedPointer<Buffer<quint8> > &buffer)
{
    BEGIN_LOG("FTDIInterface::transfer");
    HW_DEBUG("Transmitting on endpoint {}, interface {}", m_tx.first()->endpointNumber(),(qint64)this);

    m_tx.first()->transfer(buffer);
}

void FTDIInterface::receiving(void *channel, const QSharedPointer<Buffer<quint8> > &buffer)
{
    //    auto endpoint = (FTDIEndpoint*) channel;
    //    endpoint->prepareReceiving();//&m_pool);
    HardwareInterface::receiving(channel,buffer);
}

FT_HANDLE FTDIInterface::handle() const
{
    return m_handle;
}

void FTDIInterface::setHandle(const FT_HANDLE &handle)
{
    m_handle = handle;
}

#endif
