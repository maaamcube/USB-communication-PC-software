#include "am3_hardware_interface.h"
#include "am3_tools.h"
#include "am3_log.h"
#include <QDebug>
#include <QFile>
#include <QTime>

HardwareInterface::HardwareInterface(const CommunicationInterfaceInfos& infos)
    : CommunicationInterface(infos)
{

}

HardwareInterface::~HardwareInterface()
{

}

void HardwareInterface::receive(QSharedPointer<Mail> mail)
{
    m_current = mail;
    const auto size = m_current->partCount(m_infos.messageSize);

    for(auto i=0u;i<size;i++){
        m_parts[i] = m_pool.get();
        m_current->copy(i,m_infos.messageSize,m_parts[i]->data());
    }
    transfer(m_parts.first());
}

void HardwareInterface::rawReceive(const QSharedPointer<Buffer<quint8> > &buffer, quint8 hub)
{
    Q_UNUSED(buffer);
    Q_UNUSED(hub);
}


QSharedPointer<Buffer<quint8>> HardwareInterface::nextPart()
{
    m_parts.remove(m_parts.firstKey());
    if(m_parts.size())
        return m_parts.first();
    else
        return QSharedPointer<Buffer<quint8>>(nullptr);
}

void HardwareInterface::receiving(void* channel, const QSharedPointer<Buffer<quint8> > &buffer)
{
    //qDebug() << "receiving " << Tools::bigEndianToInt(buffer.data()->data()) << Tools::bigEndianToInt(&buffer.data()->data()[4]);
    BEGIN_LOG("HardwareInterface::receiving");
    Q_UNUSED(channel);
    if(m_current.isNull()){
        //TODO
       //qDebug() << "isNull";
    }else{
        //qDebug() << "isNotNull"<< QTime::currentTime().toString("hh:mm:ss.zzz");
        if((m_current->type() == Mail::DEVICE_START_ACQUISITION)||(m_current->type() == Mail::DEVICE_START_ACQUISITION_V8)||(m_current->type() == Mail::DEVICE_START_ACQUISITION_V8_ZIP))
        {
            m_current->resetTimeout();
            send(buffer);
            //qDebug() << "isAcq"<< QTime::currentTime().toString("hh:mm:ss.zzz");
        }
        else if(m_current->isAck(static_cast<quint8>(Tools::bigEndianToInt(buffer->data())))){
            HW_DEBUG("Acknowledge received hardware {}",(quint64)this);

            m_current->from(buffer->data(),m_infos.messageSize);
            auto part = nextPart();
            //qDebug() << "Part" << (ulong)part.data();
            if(part){
                //qDebug() << "Transfer" << (ulong)part.data();
                HW_DEBUG("New part available {}/{} hardware {}",m_parts.firstKey(),m_parts.lastKey(),(quint64)this);
                m_current->resetTimeout();
                transfer(part);
            }
            else{
                //qDebug() << "Processed" << m_infos.m_hub;//(ulong)part.data();
                HW_DEBUG("Mail has been transferred entirely hardware {}",(quint64)this);

                // seif optimize (appel
                /*
                if ((m_current->type() == Mail::DEVICE_GET_BIAS) || (m_current->type() == Mail::DEVICE_SET_BIAS))
                    m_current->changeStatus(Mail::RECEIVED,m_infos.m_hub!=3);
                else
                    m_current->changeStatus(Mail::RECEIVED,m_infos.m_hub!=3);
                **/
                m_current = QSharedPointer<Mail>(nullptr);
                processed();
            }
        }
        //qDebug() << "isNotNull end"<< QTime::currentTime().toString("hh:mm:ss.zzz");
    }
}

void HardwareInterface::receivingFTDI(void* channel, const QSharedPointer<Buffer<quint8> > &buffer)
{
    //qDebug() << "receiving " << Tools::bigEndianToInt(buffer.data()->data()) << Tools::bigEndianToInt(&buffer.data()->data()[4]);
    BEGIN_LOG("HardwareInterface::receiving");
    Q_UNUSED(channel);
    m_current->resetTimeout();
    send(buffer);
}
