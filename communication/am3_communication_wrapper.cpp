#include "am3_communication_wrapper.h"
#include "tools/am3_tools.h"

CommunicationWrapper*CommunicationWrapper::m_instance = nullptr;

CommunicationWrapper::CommunicationWrapper(QObject *parent) : QObject(parent)
{

    qRegisterMetaType<IdList>("IdList");
    qRegisterMetaType<SparseList>("SparseList");
    qRegisterMetaType<SensorsValues>("SensorsValues");
    m_central = Hub::instance();

    m_central->registerAutoOffset(CommunicationWrapper::static_waitForAutoOffset);
    m_central->registerConnect(CommunicationWrapper::static_connected);
    m_central->registerAcq(CommunicationWrapper::static_available);
    m_central->registerAcqMap(CommunicationWrapper::static_availableMap);
    m_central->registerDisconnect(CommunicationWrapper::static_disconnected);
    m_central->start();
}

void CommunicationWrapper::stop()
{
    m_central->stop();
}

CommunicationWrapper *CommunicationWrapper::instance()
{
    if(m_instance == nullptr)
        m_instance = new CommunicationWrapper();
    return m_instance;
}

void CommunicationWrapper::resetInstance()
{
    //m_instance->m_central = Hub::instance();
//    m_instance->m_central->registerConnect(CommunicationWrapper::static_connected);
//    m_instance->m_central->registerAcq(CommunicationWrapper::static_available);
//    m_instance->m_central->registerDisconnect(CommunicationWrapper::static_disconnected);
//    m_instance->m_central->start();
    //destroy();
    //m_instance = new CommunicationWrapper();
}

void CommunicationWrapper::destroy()
{
    if(m_instance != nullptr)
    {
        instance()->stop();
        delete m_instance;
    }
    m_instance= nullptr;
}

void CommunicationWrapper::signalAutoOffset()
{
    m_central->signalAutoOffset();
}

bool CommunicationWrapper::static_waitForAutoOffset(QString serial)
{
    return instance()->waitForAutoOffset(serial);
}

bool CommunicationWrapper::waitForAutoOffset(QString serial)
{
    auto device = m_central->device(serial);
    return emit devicewaitForAutoOffset(device);
}

void CommunicationWrapper::static_connected(QString serial)
{
    instance()->connected(serial);
}


void CommunicationWrapper::connected(QString serial)
{
    auto device = m_central->device(serial);
    emit deviceConnected(device);
}

void CommunicationWrapper::static_disconnected(QString serial)
{
    instance()->disconnected(serial);
}

void CommunicationWrapper::disconnected(QString serial)
{
    emit deviceDisconnected(serial);
}

void CommunicationWrapper::static_available(QString serial, const QList<quint32> &ms, const QList<Sparse> &sparse)
{
    instance()->available(serial,ms,sparse);
}

void CommunicationWrapper::static_availableMap(QString serial, SensorsValues values, SensorsValues brutes)
{
    instance()->availableMap(serial,values,brutes);
}

void CommunicationWrapper::availableMap(QString serial, SensorsValues values, SensorsValues brutes)
{
    emit deviceAcqAvailableMap(values,brutes);
}

void CommunicationWrapper::available(QString serial, const QList<quint32> &ms, const QList<Sparse> &sparse)
{
    emit deviceAcqAvailable(serial,ms,sparse);
    emit deviceAcqAvailableOrdered(serial,ms,sparse);
}

