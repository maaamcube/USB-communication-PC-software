#include "am3_communication_central.h"


CommunicationCentral::CommunicationCentral()
{
    m_hub = nullptr;
    m_path_new = "123456789101112131415";
}

CommunicationInterface *CommunicationCentral::get(const QString &serial)
{
    if(m_interfaces.contains(serial))
        return m_interfaces[serial];
    return nullptr;
}

QList<CommunicationInterface *> CommunicationCentral::listInterfacesConnected()
{
    return m_interfaces.values();
}

void CommunicationCentral::initCheck()
{
    m_path_new = "123456789101112131415";
}

