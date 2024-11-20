#ifdef WITH_FTD_32_BIT

#include "am3_ftdi_endpoint.h"
#include "am3_log.h"
#include <QDebug>
#include <QFile>
#include <QTime>
#include <am3_tools.h>
#include "am3_ftdi_interface.h"
#include "am3_buffer_builder.h"
#include "am3_hub.h"

bool FTDIEndpoint::isreceiveFTDIrunning = false;

FTDIEndpoint::FTDIEndpoint(int endpoint,
                         FTDIInterface* interface)
    : m_transfers(1000)
{
    m_ftdiInterface = interface;
    m_endpointNumber = endpoint;
    LastFrameCount = -1;
    LastEllapsedTime = -1;
}

//void FTDIEndpoint::transferCallback(FT_HANDLE xfr)
//{
//    auto endpoint = (FTDIEndpoint*) xfr->user_data;
//    switch(xfr->status){
//        case FT_HANDLE_COMPLETED:

//        if(this->isRxEndpoint()){
//            // RX case, we received data, we send it through interface
//            auto t = this->getTransfer(xfr);
//            auto b = this->getBuffer(t);
//            this->changeStatus(xfr,COMPLETED);
//            this->FTDIInterface()->receiving(endpoint,b);
//        }else{
//            // TX case, data has been transfered, we acknowledge reception
//        }
//        break;

//    case FT_HANDLE_STALL:
//    case FT_HANDLE_NO_DEVICE:
//    case FT_HANDLE_OVERFLOW:
//    case FT_HANDLE_ERROR:
//        this->FTDIInterface()->error();
//        break;
//    case FT_HANDLE_CANCELLED:
//        break;
//    case FT_HANDLE_TIMED_OUT:
//        break;
//    }


//    this->releaseTransferRessources(xfr);
//}

bool FTDIEndpoint::receiveFTDI(QSharedPointer<Buffer<quint8> > buffer)
{
    //done here else in the constructor it is too soon
    QString theSerial = m_ftdiInterface->serial();
    GenericDevice * lastDevice = Hub::instance()->device(theSerial);
    if (lastDevice != nullptr)
    {
        m_height = lastDevice->genericInfos()->m_height;
        m_width = lastDevice->genericInfos()->m_width;
    }
    else
    {
         m_height = 52;
         m_width = 52;
    }



    //qDebug()<<"FTDIEndpoint::receiveFTDI   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;";
    isreceiveFTDIrunning = true;
    bool retour = false;
    //buffer is null mean reception not transmmission
    if ((this->isRxEndpoint()) || (buffer.isNull()))
    {
        FT_STATUS ftStatus;
        FT_HANDLE ftHandle = m_ftdiInterface->handle();

#ifdef Q_OS_MACOS
        if (((long int )ftHandle) != INVALID_HANDLE_VALUE)
#else
        if (ftHandle != INVALID_HANDLE_VALUE)
#endif
        {
            DWORD RxBytes;
            DWORD BytesReceived;
            bool isNewFrame = false;

            ftStatus = FT_GetQueueStatus(ftHandle,&RxBytes);
            while (isreceiveFTDIrunning && (ftStatus == FT_OK) && (RxBytes > 0))
            {
                // tempo to avoid superposition of images in V5
                QThread::msleep(2);

                const DWORD FT_In_Buffer_Size = 65536;
                RxBytes = qMin(RxBytes, FT_In_Buffer_Size);
                char RxBuffer[FT_In_Buffer_Size];

                ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);

                if (ftStatus == FT_OK)
                {
                    retour = true;
                    if (RxBytes == BytesReceived)
                    {
                        for (int c = 0 ; c < BytesReceived ; c++)
                        {
                            dataline.append(RxBuffer[c]);

                            if ((dataline.size() > 4 ) && (RxBuffer[c] == 0))  //************ ROWDATA DETECTION ***********
                            {
                               isNewFrame = computeLine(); //A line has been detected, it is sent to the matrix
                               dataline.clear();
                            }

                            if (isNewFrame)
                            {
                                int cB = 0;
                                while (cB < newBufferFTDI.size())
                                {
                                    QSharedPointer<Buffer<quint8> > b = QSharedPointer<Buffer<quint8> >(BufferBuilder<quint8>::build(QSize(64,65)));
                                    unsigned char *unRxBuffer = new unsigned char[b->size()];
                                    memset(unRxBuffer,0,b->size());
                                    int ib = 0;
                                    // Everything is ok, keep buffer reference
                                    for ( ; (ib < b->size()) && (cB+ib < newBufferFTDI.size()) ; ib++)
                                    {
                                        unRxBuffer[ib] = newBufferFTDI[cB+ib];
                                        //if (pBufferFTDI[cB+ib] != 0) qDebug()<<"unRxBuffer["<<ib<<"] = "<<pBufferFTDI[cB+ib];
                                    }
                                    b->setData(unRxBuffer);
                                    this->changeStatus(ftHandle,COMPLETED);
                                    this->ftdiInterface()->receivingFTDI(this,b);
                                    b.clear();

                                    cB+=ib;
                                    //delete[] unRxBuffer;
                                }
                                isNewFrame = false;
                            }
                        }
                    }
                    else
                    {
                        HW_ERROR("Transfer read failed for endpoint {:x}: {}",
                                  m_endpointNumber,
                                  libFTDI_strerror(static_cast<libFTDI_error>(error)));
                        this->ftdiInterface()->error();
                        retour = false;
                    }
                }
                else
                {
                    retour = false;
                }
                FT_GetQueueStatus(ftHandle,&RxBytes);
            }
        }
    }
    isreceiveFTDIrunning = false;
    return retour;
}


bool FTDIEndpoint::computeLine()
{
  int ofrow = (64 - m_height) / 2;
  int ofcol = (64 - m_width) / 2;

  int EllapsedTime= dataline[0];
  int FrameCount  = dataline[1];
  int Row         = qMax(0,dataline[2] - ofrow);
  int Column      = qMax(0,dataline[3] - ofcol);

  bool  result = false;

  if ((FrameCount != LastFrameCount) || (EllapsedTime != LastEllapsedTime))
  {
      result = true;
      newBufferFTDI = pBufferFTDI;
      pBufferFTDI.clear();
      for (int f = 0 ;  f < (m_height*m_width + 1) ; f++) pBufferFTDI.append(0);
      int FrameTimeStamp = ((EllapsedTime - 1) *1000 + (FrameCount-1) * 1000/100);
      pBufferFTDI[0] = static_cast<quint8>(FrameTimeStamp >> 24);
      pBufferFTDI[1] = static_cast<quint8>(FrameTimeStamp >> 16);
      pBufferFTDI[2] = static_cast<quint8>(FrameTimeStamp >> 8);
      pBufferFTDI[3] = static_cast<quint8>(FrameTimeStamp);
  }
  int i = 0;
  while (((i + 4) < dataline.size()) && (dataline[i+4] != 0))
  {
      pBufferFTDI[4 + Row-1 + (Column+i)*m_width] = dataline[i+4];
      i++;
  }
  LastFrameCount = FrameCount;
  LastEllapsedTime = EllapsedTime;

  return result;
}

bool FTDIEndpoint::transfer(QSharedPointer<Buffer<quint8> > buffer)
{
    FT_HANDLE ftHandle = m_ftdiInterface->handle();

#ifdef Q_OS_MACOS
        if (((long int )ftHandle) != INVALID_HANDLE_VALUE)
#else
        if (ftHandle != INVALID_HANDLE_VALUE)
#endif
    {
        // endpoint is TX type (because of the else)
        // Ensure endpoint is not in use
        if(!m_status.isEmpty())
        {
            auto key = m_status.keys().first();
            changeStatus(*key.data(),COMPLETED);
        }
        else
        {
            //Get transfer container
            auto transfer = m_transfers.get();
            if(!transfer.isNull())
            {
                // Get mail data
                auto data_buffer = buffer->data();

                DWORD dwToWrite = buffer->size();
                DWORD dwWritten;
                OVERLAPPED osWrite = { 0 };
                if (!FT_W32_WriteFile(ftHandle, data_buffer, dwToWrite, &dwWritten, &osWrite))
                {
                    if (FT_W32_GetLastError(ftHandle) == ERROR_IO_PENDING)
                    {
                        // write is delayed so do some other stuff until ...
                        if (!FT_W32_GetOverlappedResult(ftHandle, &osWrite, &dwWritten, FALSE))
                        {
                        // error
                        }
                        else
                        {
                            if (dwToWrite == dwWritten)
                            {
                                // Everything is ok, keep buffer reference
                                m_buffers[transfer] = buffer;
                                m_status[transfer] = WAITING_FOR_COMPLETION;
                                return true;
                            }
                            else
                            {
                                HW_ERROR("Transfer failed for endpoint {:x}: {}",
                                          m_endpointNumber,
                                          libFTDI_strerror(static_cast<libFTDI_error>(error)));
                                this->ftdiInterface()->error();
                                return false;
                            }
                        }
                    }
                }
                else
                {
                    HW_ERROR("Transfer failed for endpoint {:x}: {}",
                              m_endpointNumber,
                              libFTDI_strerror(static_cast<libFTDI_error>(error)));

                    return false;
                }
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}



void FTDIEndpoint::cancelCurrentTransfers()
{

}

void FTDIEndpoint::changeStatus(FT_HANDLE xfr, FTDIEndpoint::TransferStatus status)
{
    BEGIN_LOG("FTDIEndpoint::changeStatus");
    auto t = getTransfer(xfr);
    if(status == COMPLETED){
        m_status.remove(t);
        m_buffers.remove(t);
        HW_DEBUG("Release transfer {}",(qint64)xfr);
    }
    else
        m_status[t] = status;
}

void FTDIEndpoint::close()
{
    //TODO
}

bool FTDIEndpoint::isClosed()
{
    //TODO
    return true;
}

bool FTDIEndpoint::prepareReceiving()//BufferPool<quint8> *pool)
{
    QSharedPointer<Buffer<quint8> > buffer;
    auto ok = receiveFTDI(buffer);
    if(!ok)
        return false;
    return true;
}

FTDIEndpoint::shared_transfer FTDIEndpoint::getTransfer(FT_HANDLE xfr)
{
    shared_transfer t;
    for(auto it=m_status.begin();it!=m_status.end();it++){
        if((*it.key().data()) == xfr)
            t = it.key();

    }
    return t;
}

QSharedPointer<Buffer<quint8> > FTDIEndpoint::getBuffer(FTDIEndpoint::shared_transfer xfr)
{
    return m_buffers[xfr];
}

bool FTDIEndpoint::prepareAll()
{
    return true;
}

FTDIInterface *FTDIEndpoint::ftdiInterface()
{
    return m_ftdiInterface;
}



bool FTDIEndpoint::isRxEndpoint()
{
    return (m_endpointNumber & 0x80) == 0x80;
}

void FTDIEndpoint::releaseTransferRessources(FT_HANDLE xfr)
{
    //libFTDI_free_transfer(xfr);
}

bool FTDIEndpoint::getIsreceiveFTDIrunning()
{
    return isreceiveFTDIrunning;
}

int FTDIEndpoint::endpointNumber() const
{
    return m_endpointNumber;
}

#endif
