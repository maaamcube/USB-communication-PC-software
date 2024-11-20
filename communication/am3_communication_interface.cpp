#include "am3_communication_interface.h"
#include "am3_communication_link.h"
#include "am3_log.h"

CommunicationInterface::CommunicationInterface(const CommunicationInterfaceInfos& infos)
    : m_link(nullptr),  m_infos(infos), m_pool(infos.messageSize,25000), m_central(infos.mailVersion,(void*)(this))
{
}

CommunicationInterface::~CommunicationInterface()
{
    m_central.clear();
}

void CommunicationInterface::send(QSharedPointer<Mail> mail)
{
    BEGIN_LOG("CommunicationInterface::send");
    if(m_link && isMailAccepted(mail->type())){
        m_link->transfer(mail,this);
    }
    else{
        HW_ERROR("CommunicationInterface::send - No valid link or mail");
    }

}

void CommunicationInterface::send(const QSharedPointer<Buffer<quint8> > &buffer)
{
    BEGIN_LOG("CommunicationInterface::send");
    if(m_link){
        m_link->transfer(buffer,this);
    }
    else{
        HW_ERROR("CommunicationInterface::send - No valid link or mail");
    }
}



QSharedPointer<Mail> CommunicationInterface::create()
{
    return m_central.create();
}

QSharedPointer<Mail> CommunicationInterface::create(processed_callback proc)
{
    return m_central.create(proc);
}

QSharedPointer<Mail> CommunicationInterface::create(Mail::MailType type)
{
    return m_central.create(type);
}



void CommunicationInterface::processed()
{
    if(m_link)
        m_link->processed(this);
}

void CommunicationInterface::error()
{
    if(m_link)
        m_link->error(this);
}

bool CommunicationInterface::isDown() const
{
    return m_link->isDown();
}


void CommunicationInterface::setLink(CommunicationLink *link)
{
    m_link = link;
}

CommunicationLink *CommunicationInterface::getLink()
{
    return m_link;
}


CommunicationInterfaceInfos CommunicationInterface::infos() const
{
    return m_infos;
}

void CommunicationInterface::setHub(quint8 hub)
{
    m_infos.m_hub = hub;
}

