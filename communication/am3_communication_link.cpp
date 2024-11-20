#include "am3_communication_link.h"
#include "am3_hub.h"
#include "am3_tools.h"
#include <QTime>
#include <QtConcurrent/QtConcurrent>

CommunicationLink::CommunicationLink(Hub *hub,
                                     CommunicationInterface *a,
                                     QList<CommunicationInterface *> bs)

{
    m_hub = hub;
    m_started = false;

    init(a,true);
    for(auto b : bs)
        init(b,false);

    m_deadLink = false;
    QtConcurrent::run(this,&CommunicationLink::run);
    //start();
}

CommunicationLink::~CommunicationLink()
{
    //qDebug() << "~CommunicationLink " << m_started;

   /* while(m_started){

    }
    for(auto receiver : m_fifos.keys()) {
        receiver->setLink(nullptr);
    }*/
}

void CommunicationLink::transfer(QSharedPointer<Mail> mail,
                                 CommunicationInterface *emitter)
{
    BEGIN_LOG(" CommunicationLink::transfer");
    auto& fifo = (emitter == m_fifos_a.firstKey()) ? m_fifos_b : m_fifos_a;

    auto origin = QSharedPointer<Mail>(new Mail(*mail.data()));
    //qDebug() << "Transfer " << mail->name() << QTime::currentTime().toString("hh:mm:ss.zzz");
    QMutexLocker lock(&m_mutex);
    bool need_copy = false;

    for(auto receiver : fifo.keys()) {
        if((mail->type() == Mail::DEVICE_START_ACQUISITION) || (mail->type() == Mail::DEVICE_START_ACQUISITION_V8) || (mail->type() == Mail::DEVICE_START_ACQUISITION_V8_ZIP))
        {
            Tools::intToBigEndian((quint8*)&mail->tx()->data()[8],receiver->infos().m_hub-3);
        }
        if(need_copy){
            auto copy = QSharedPointer<Mail>(new Mail(*origin.data()));
            if((copy->type() == Mail::DEVICE_START_ACQUISITION)||(copy->type() == Mail::DEVICE_START_ACQUISITION_V8)||(copy->type() == Mail::DEVICE_START_ACQUISITION_V8_ZIP)){// TODO
                Tools::intToBigEndian((quint8*)&copy->tx()->data()[8],receiver->infos().m_hub-3);
            }
            fifo[receiver].enqueue(copy);
        }
        else
        {
            fifo[receiver].enqueue(mail);
        }

        //qDebug()<<"000tryReceipt("<<(int)receiver<<");" << QTime::currentTime().toString("hh:mm:ss.zzz");
        tryReceipt(receiver);
        need_copy = true;

        if(mail->type() == Mail::DEVICE_GET_CALIBRATION)
            break;
    }
}

void CommunicationLink::transfer(const QSharedPointer<Buffer<quint8> > &buffer,
                                 CommunicationInterface *emitter)
{
    auto& fifo = (emitter == m_fifos_a.firstKey()) ? m_fifos_b : m_fifos_a;
    //qDebug() << "transfer   fifo.keys().size : " << fifo.keys().size() << QTime::currentTime().toString("hh:mm:ss.zzz");
    //QMutexLocker lock(&m_mutex);
    if(m_fifos_b[emitter].size() &&
       ((m_fifos_b[emitter].first()->type() == Mail::DEVICE_START_ACQUISITION)||(m_fifos_b[emitter].first()->type() == Mail::DEVICE_START_ACQUISITION_V8)||(m_fifos_b[emitter].first()->type() == Mail::DEVICE_START_ACQUISITION_V8_ZIP)))
    {
           m_fifos_b[emitter].first()->changeStatus(Mail::RECEIVED);
           m_fifos_b[emitter].dequeue();
    }
    //qDebug()<<(emitter == m_fifos_a.firstKey())<<"   "<<fifo.keys().size();

    for(auto receiver : fifo.keys()) {
        receiver->rawReceive(buffer,emitter->infos().m_hub);
    }
}

void CommunicationLink::processed(CommunicationInterface *receiver)
{
    BEGIN_LOG("CommunicationLink::processed");

    if (!m_fifos_a.isEmpty())
    {
        auto& fifo = (receiver == m_fifos_a.firstKey()) ? m_fifos_a : m_fifos_b;
        if(fifo[receiver].size()){

            //QMutexLocker lock(&m_mutex);
            HW_DEBUG("Mail {} dequeue, receiver {}", fifo[receiver].first()->id(), (quint64)receiver);
            // MODIFS_L fifo[receiver].first()->changeStatus(Mail::RECEIVED, (receiver->infos().m_hub !=3));
            fifo[receiver].first()->changeStatus(Mail::RECEIVED);
            fifo[receiver].dequeue();
            m_timeoutCount[receiver] = 0u;
            tryReceipt(receiver);
        }
        else{
            HW_ERROR("Mail fifo empty, receiver {}", (quint64)receiver);
        }
    }
}

void CommunicationLink::error(CommunicationInterface *receiver)
{
    Q_UNUSED(receiver);
    qDebug() <<"CommunicationLink::error hub="<<receiver->infos().m_hub << " N°serie=" << receiver->m_infos.serial;
    declareDeadLink();
}

bool CommunicationLink::isDown() const
{
    return m_deadLink;
}

void CommunicationLink::run()
{
    m_started = true;
    while(!m_deadLink){

        //qDebug() << "run communication sleep" << QTime::currentTime().toString("hh:mm:ss.zzz") << m_fifos_a.size()
          //       << m_fifos_b.size();
        QThread::msleep(delta);

            auto list = QList<CommunicationInterface *>() << m_fifos_a.keys()<< m_fifos_b.keys();
            m_mutex.lock();
            for(auto receiver : list) {

                auto& fifo = (receiver == m_fifos_a.firstKey()) ? m_fifos_a : m_fifos_b;
                //qDebug() <<"fifo[receiver].size()  "<<fifo[receiver].size();
                if(fifo[receiver].size()){
                    //qDebug() << "run communication link" << QTime::currentTime().toString("hh:mm:ss.zzz")
                          //   << fifo[receiver].first()->status() << fifo[receiver].first()->type();
                    fifo[receiver].first()->addDelta(delta);
                    //qDebug() << "run communication delta" << QTime::currentTime().toString("hh:mm:ss.zzz");
                    if(fifo[receiver].first()->status() == Mail::RECEPTION_TIMEOUT){
                        //qDebug() << "run communication timeout" << QTime::currentTime().toString("hh:mm:ss.zzz");
                        m_timeoutCount[receiver]++;
                        if(m_timeoutCount[receiver]>2){
                            //qDebug() << "run communication dead" << QTime::currentTime().toString("hh:mm:ss.zzz");
                            //qDebug() <<"run communication dead for hub="<< receiver->infos().m_hub;
                            declareDeadLink();
                            BEGIN_LOG("CommunicationLink::run()")
                            HW_DEBUG("Mail {} dequeue, receiver {}", fifo[receiver].first()->id(), (quint64)receiver);
                            fifo[receiver].dequeue();
                        }
                        else {
                           fifo[receiver].first()->changeStatus(Mail::WAITING_FOR_RECEPTION);
                            tryReceipt(receiver);
                        }
                    }

                }
            }
            m_mutex.unlock();
    }
    qDebug() <<"Deadlink";
    m_started =false;
}

void CommunicationLink::stop()
{
    m_fifos_a.clear();
    m_fifos_b.clear();

    qDebug() <<"CommunicationLink::stop";
    declareDeadLink();
}

void CommunicationLink::preStop()
{
    for(auto receiver : m_fifos_a.keys()) {
        receiver->setLink(nullptr);
    }
    for(auto receiver : m_fifos_b.keys()) {
        receiver->setLink(nullptr);
    }
}

int CommunicationLink::declareDeadLink()
{
    auto list = QList<CommunicationInterface *>()   << m_fifos_a.keys()
                                                    << m_fifos_b.keys();

    m_deadLink = true;
    for(auto receiver : list) {
        receiver->linkDown();
    }
    m_hub->deadLink(this);

    return delta;
}


void CommunicationLink::tryReceipt(CommunicationInterface *receiver)
{
    auto& fifo = (receiver == m_fifos_a.firstKey()) ? m_fifos_a : m_fifos_b;

    if(fifo[receiver].size()){
     if(fifo[receiver].first()->status() == Mail::WAITING_FOR_RECEPTION){

            fifo[receiver].first()->changeStatus(Mail::RECEIVING);
            receiver->receive(fifo[receiver].first());
        }
    }
}

void CommunicationLink::init(CommunicationInterface *interface,bool a)
{
    auto& fifo = (a) ? m_fifos_a : m_fifos_b;
    fifo[interface] = QQueue<QSharedPointer<Mail>>();
    m_timeoutCount[interface] = 0u;
    interface->setLink(this);
}

