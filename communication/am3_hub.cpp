#include "am3_hub.h"
#include "am3_usb_central.h"
#include "am3_device_builder.h"
#include "am3_communication_wrapper.h"
#include <QtConcurrent/QtConcurrent>
#include <am3_ftdi_central.h>
#include <am3_tools.h>
Hub* Hub::m_instance = nullptr;
Hub::Hub()
{
    f_autoOffset     = nullptr;
    f_connected     = nullptr;
    f_disconnected  = nullptr;
    f_status        = nullptr;
    f_acquisition   = nullptr;
    workingDevice   = nullptr;
    workingInterface = nullptr;
    f_acquisition_map   = nullptr;

    QThreadPool::globalInstance()->setMaxThreadCount(8);

    m_bias = 777777;
    m_isSpecial = false;
}

Hub::~Hub()
{
}

Hub *Hub::instance()
{
    if(!m_instance)
        m_instance = new Hub();
    return m_instance;
}

void Hub::destroy()
{
    delete m_instance;
    m_instance = nullptr;
}

void Hub::start()
{
    //qDebug() << QTime::currentTime().toString("[hh:mm:ss:zzz] ") << "Start";
    addCentral(USBCentral::instance());
#ifdef WITH_FTD_32_BIT
    addCentralFTDI(FTDICentral::instance());
#endif


}

void Hub::stop()
{
    //qDebug() << QTime::currentTime().toString("[hh:mm:ss:zzz] ") << "Stop";
    for(auto l : m_links){
        l->stop();
    }

    for(auto central : m_centrals)
        central->exit();

    QThread::msleep(50);

    for(auto central : m_centrals)
        central->waitExit();

    QThread::msleep(50);

#ifdef WITH_FTD_32_BIT

    for(auto central : m_centralsFTDI)
        central->exit();

    QThread::msleep(50);

    for(auto central : m_centralsFTDI)
        central->waitExit();

#endif
    for(auto l : m_links){
        delete l;
    }

// this make MacOs exit in error so commented
//    for(auto d : m_devices){
//        delete d;
//    }

    for(auto c : m_centrals){
        auto usb = (USBCentral*) c;
        usb->destroy();
    }

#ifdef WITH_FTD_32_BIT
    for(auto c : m_centralsFTDI){
        auto ftdi = (FTDICentral*) c;
        ftdi->destroy();
    }
#endif
    m_links.clear();
    m_devices.clear();
    m_centrals.clear();

    m_devicesSpecial.clear();
    m_linksSpecial.clear();
    m_mapSpecialSerial.clear();
    m_savedDevices.clear();

    #ifdef WITH_FTD_32_BIT
    m_centralsFTDI.clear();
    #endif
}

void Hub::stopPartial()
{
    for(auto l : m_links){
        //l->exitLink();  ne ferme plus si on ne fait pas le stop a la place
        l->stop();
    }

    QThread::msleep(500);
    for(auto l : m_links){
        delete l;
    }

    for(auto d : m_devices){
        //delete d;
    }

    m_links.clear();
    m_devices.clear();

}

void Hub::restartWithAutoOffset()
{
    qDebug()<<"restartWithAutoOffset begin";
    Hub::instance()->stop();
    qDebug()<<"stop() Tools::AUTO_OFFSET_TIME="<<Tools::AUTO_OFFSET_TIME;
    QThread::sleep(1);
    // Forcer l'autoOffset si c'est le bouton Lancer qui est activé
    if (Tools::AUTO_OFFSET_TIME<0)
        Tools::AUTO_OFFSET_TIME = -Tools::AUTO_OFFSET_TIME;

    qDebug()<<"sss Hub::instance()->start()";
    Hub::instance()->start();
}

GenericDevice * Hub::restoreDevices()
{
    GenericDevice * retour = nullptr;
    QList<CommunicationInterface*> interfaces;

    foreach (CommunicationCentral * cc, Hub::instance()->getCentrals())
    {
        foreach (QString d, m_savedDevices.keys())
        {
            interfaces << cc->get(d);
        }

        if ((!m_savedDevices.isEmpty()) && (!interfaces.isEmpty()))
        {
            QString firstSerial = m_savedDevices.firstKey();

            auto dev_interface = DeviceBuilder().create(interfaces.first()->infos());
            m_devices[firstSerial] = dev_interface;
            m_devices[firstSerial]->registerAvailableCallback(Hub::s_acqAvailable);
            m_devices[firstSerial]->registerAvailableCallbackMap(Hub::s_acqAvailable_map);
            m_devices[firstSerial]->registerStatusCallback(Hub::s_statusUpdate);
            m_devices[firstSerial]->registerConnectedCallback(f_connected);
            m_links[firstSerial] = new CommunicationLink(this,dev_interface,interfaces);
            m_devices[firstSerial]->loadCalibration();
            m_devices[firstSerial]->changeStatus(GenericDevice::CONNECTED);
            if(f_connected )
                f_connected(firstSerial);

            retour = m_devices[firstSerial];
        }

    }

    return retour;
}

void Hub::setSavedDevices(QStringList serials)
{
    if (!serials.isEmpty())
    {
        //m_savedDevices.clear();
        foreach(QString s, serials)
        {
            m_savedDevices[s] = true;
        }
    }
}

/*!
 * \brief Hub::connectDevicesForSpecialCommunication
 * Specially connects cards to make special communication like retrieving or recording the bias.
 * \param cardNumber : integer that represent the number of card (1,2,3)
 * \return integer that represents the index of the card in the hub
 */
GenericDevice *Hub::connectDevicesForSpecialCommunication(int cardNumber)
{
    auto hub = cardNumber + 3;
    foreach (CommunicationCentral * cc, getCentrals())
    {
        auto interfaces = cc->listInterfacesConnected();
        foreach(CommunicationInterface * inter, interfaces)
        {
            if (inter->infos().m_hub == hub)
            {
                QList<CommunicationInterface*> b;
                b.append(inter);
                auto dev_interface = DeviceBuilder().create(inter->infos());
                new CommunicationLink(this, dev_interface, b);
                dev_interface->changeStatus(GenericDevice::CONNECTED);
                return dev_interface;
            }
        }
    }
    return nullptr;
}

unsigned int Hub::get_bias_information(int cardNumber)
{
    unsigned int retour = 0;
    auto  dev_interface = connectDevicesForSpecialCommunication(cardNumber);

    if (dev_interface != nullptr){
        dev_interface->get_bias_informations(&retour);

        auto delta = dev_interface->getLink()->declareDeadLink() * 2;
        //il faut attendre sinon le lien de communication ne se rétablis pas
        QThread::msleep(delta);
        // remettre le lien de communication d'origine
        auto interfaces = getCentrals().first()->listInterfacesConnected();
        foreach(CommunicationInterface * inter, interfaces)
        {
            inter->setLink(m_links.first());
        }
    }
    return retour;
}

bool Hub::set_bias_information(unsigned int bias, int cardNumber)
{
    bool retour = false;
    auto  dev_interface = connectDevicesForSpecialCommunication(cardNumber);

    if (dev_interface != nullptr){
        retour = dev_interface->set_bias_informations(bias);
        auto delta = dev_interface->getLink()->declareDeadLink() * 2;
        //il faut attendre sinon le lien de communication ne se rétablis pas
        QThread::msleep(delta);
        // remettre le lien de communication d'origine
        auto interfaces = getCentrals().first()->listInterfacesConnected();
        foreach(CommunicationInterface * inter, interfaces)
        {
            inter->setLink(m_links.first());
        }
    }
    return retour;
}

void Hub::fireConnected(QString serial)
{
    if(f_connected )
    {
        f_connected(serial);
    }
}

void Hub::signalAutoOffset()
{
     m_mutexAutoOffset = true;
}

void Hub::addCentral(CommunicationCentral *central)
{
    if(central){
        m_centrals << central;
        central->init(this);
    }
}

#ifdef WITH_FTD_32_BIT
void Hub::addCentralFTDI(FTDICentral *central)
{
    if(central){
        m_centralsFTDI << central;
        central->init(this);
    }
}
#endif

QMap<QString, GenericDevice *> Hub::getDevices() const
{
    return m_devices;
}

QList<CommunicationCentral *> Hub::getCentrals() const
{
    return m_centrals;
}

void Hub::deadLink(CommunicationLink *link)
{
    Q_UNUSED(link)
}

void Hub::connect(CommunicationCentral* central,const QStringList &serials)
{
    if (m_isSpecial)
    {
        connectSpecial(central, m_serial);
        m_isSpecial = false;

    }
    else
    {
        if (serials.size() > 0)
        {
            QList<CommunicationInterface*> interfaces;

            for(auto serial : serials)
            {
                interfaces << central->get(serial);
            }

            auto serialFirst = serials.first();
            qDebug()<<"----------- PF serial "<<serialFirst<<" connected ";
            auto dev_interface = DeviceBuilder().create(interfaces.first()->infos());

            m_devices[serialFirst] = dev_interface;
            m_devices[serialFirst]->registerAvailableCallback(Hub::s_acqAvailable);
            m_devices[serialFirst]->registerAvailableCallbackMap(Hub::s_acqAvailable_map);
            m_devices[serialFirst]->registerStatusCallback(Hub::s_statusUpdate);
            m_devices[serialFirst]->registerConnectedCallback(f_connected);
            m_links[serialFirst] = new CommunicationLink(this,dev_interface,interfaces);
            m_devices[serialFirst]->loadCalibration();
            // Wait for loading calibration
            QThread::msleep(200);

            if (Tools::AUTO_OFFSET_TIME>0)
            {
                qDebug() << "----------------------------------- auto offset on -----";
                autoOffset(serialFirst);
            }
            else
            {
                qDebug() << "----------------------------------- auto offset off";
                fireConnected(serials.first());
            }

        }
    }
}

void Hub::autoOffset(const QString &serialFirst)
{
    if(f_autoOffset != nullptr)
    {
        m_mutexAutoOffset = false;
        auto result = f_autoOffset(serialFirst);
        while (!m_mutexAutoOffset) {
            QThread::msleep(100);
        }

        qDebug()<<"Auto Offset authorization Ok with result="<<result;
    }

    m_devices[serialFirst]->setHub(this);
    m_devices[serialFirst]->setSerialRef(serialFirst);
    m_devices[serialFirst]->autoCalibration();
}


void Hub::reConnectSpecial(const QString &serial)
{
        QList<CommunicationInterface*> interfaces;

        foreach (CommunicationCentral * cc, Hub::instance()->getCentrals())
        {
            interfaces << cc->get(serial);
        }

        m_links[serial] = new CommunicationLink(this,m_devices[serial],interfaces);

}


void Hub::connectSpecial(CommunicationCentral* central,const QString serial)
{
    QList<CommunicationInterface*> interfaces;

    interfaces << central->get(serial);

    QString specialSerial = serial + "_Special";
    auto dev_interface = DeviceBuilder().create(interfaces.first()->infos());
    m_devicesSpecial[specialSerial] = dev_interface;
//    m_devicesSpecial[specialSerial]->registerAvailableCallback(Hub::s_acqAvailable);
//    m_devices[serials.first()]->registerAvailableCallbackMap(Hub::s_acqAvailable_map);
//    m_devicesSpecial[specialSerial]->registerStatusCallback(Hub::s_statusUpdate);
//    m_devicesSpecial[specialSerial]->registerConnectedCallback(f_connected);
    m_linksSpecial[specialSerial] = new CommunicationLink(this,dev_interface,interfaces);
//    m_devicesSpecial[specialSerial]->loadCalibration();
    m_devicesSpecial[specialSerial]->changeStatus(GenericDevice::CONNECTED);
    // do not send the signal connected because it is special and the plate is already connected
//    if(f_connected )
//        f_connected(serial);
}

void Hub::disconnectSpecial(CommunicationCentral *central, const QString &serial)
{
    Q_UNUSED(central)

    for(auto l : m_linksSpecial){
        //l->exitLink();  ne ferme plus si on ne fait pas le stop a la place
        l->stop();
        l->preStop();
    }
    for(auto l : m_linksSpecial){
        delete l;
    }

    for(auto d : m_devicesSpecial){
        delete d;
    }

    m_linksSpecial.clear();
    m_devicesSpecial.clear();

   // m_centrals.clear();

    //    stop();
}

void Hub::disconnect(CommunicationCentral *central, const QString &serial)
{
    if(m_devices.contains(serial)){
        if(f_disconnected)
            f_disconnected(serial);
    }
    central->initCheck();
}

GenericDevice *Hub::device(const QString &serial)
{
    if(m_devices.contains(serial))
        return m_devices[serial];
    return nullptr;
}

void Hub::registerAutoOffset(autoOffset_cbk cbk){
    f_autoOffset = cbk;
}

void Hub::registerConnect(connected_cbk cbk)
{
    f_connected = cbk;
}

void Hub::registerDisconnect(disconnected_cbk cbk)
{
    f_disconnected = cbk;
}

void Hub::registerStatus(status_update_cbk cbk)
{
    f_status = cbk;
}

void Hub::registerAcq(acquisition_available_cbk cbk)
{
    f_acquisition = cbk;
}

void Hub::registerAcqMap(acquisition_available_map cbk)
{
    f_acquisition_map = cbk;
}

void Hub::s_acqAvailable(GenericDevice *device, QList<quint32> ms, QList<Sparse> sparses)
{
    Hub::instance()->acqAvailable(device,ms,sparses);
}

void Hub::s_acqAvailable_map(GenericDevice *device, SensorsValues values, SensorsValues brutes)
{
    //qDebug() << "Available " << ms;
    Hub::instance()->acqAvailableMap(device,values,brutes);
    //qDebug() << "4c";
}

void Hub::s_statusUpdate(GenericDevice *device)
{
    Hub::instance()->statusUpdate(device);
}

void Hub::statusUpdate(GenericDevice *device)
{
    if(f_status)
        f_status(device->infos().communicationInfos.serial,
                 device->status());
}

quint32 Hub::get_older_frame_timestamp(GenericDevice *device)
{
    if(device->availables().size())
        return device->availables().first();
    return 0;
}

QList<GenericDevice *> Hub::number_connected()
{
    return m_devices.values();
}

void Hub::acqAvailable(GenericDevice *device, QList<quint32>ms, QList<Sparse> sparses)
{
    HW_INFO("Acqs availables");
    if(f_acquisition){
        f_acquisition(device->infos().communicationInfos.serial,
                      ms,
                      sparses);
    }
}

void Hub::acqAvailableMap(GenericDevice *device, SensorsValues values, SensorsValues brutes)
{
    HW_INFO("Acqs MAP availables");
    if(f_acquisition_map){
        f_acquisition_map(device->infos().communicationInfos.serial,
                      values,
                      brutes);
    }
}

void Hub::prepare_bias_informations(int nbCartes)
{
    for (int inb = 0 ; inb < nbCartes; inb++)
    {
        CommunicationInterface * interface = nullptr;
        foreach (CommunicationCentral * cc, Hub::instance()->getCentrals())
        {
            foreach(CommunicationInterface * inter, cc->listInterfacesConnected())
            {
               if (inter->infos().m_hub == (inb+3))
               {
                   interface = inter;
                   break;
               }
            }
        }
        if (interface != nullptr)
        {
            m_mapSpecialSerial[inb] = interface->infos().serial;
        }
    }
}

unsigned int Hub::get_bias_informations(int numCarte, int nbCarte)
{
        unsigned int retour = 0;

        m_isSpecial = true;
        m_serial = m_mapSpecialSerial[numCarte];

        stopPartial();

        QThread::msleep(100);


        foreach (CommunicationCentral * cc, Hub::instance()->getCentrals())
        {
            foreach(CommunicationInterface * inter, cc->listInterfacesConnected())
            {
               if (inter->infos().m_hub == (numCarte+nbCarte))//1))
               {
                   workingInterface = inter;
                   break;
               }
            }
        }
        if (workingInterface != nullptr)
        {

            QString specialSerial = workingInterface->infos().serial + "_Special";

            if (!m_devicesSpecial.contains(specialSerial)) connectSpecial(USBCentral::instance(),workingInterface->infos().serial);

            if (m_devicesSpecial.contains(specialSerial))
            {
                workingDevice = m_devicesSpecial[specialSerial];

                auto mail = workingInterface->create(processedBias);
                mail->fill(Mail::DEVICE_GET_BIAS,100);
                mail >> (*workingDevice);
                int cpt = 0;
                while ((cpt < 100) && (!mail->isProcessed()))
                {
                    QThread::msleep(10);
                    cpt++;
                }
//                device->get_bias_informations(&m_bias);
                retour = m_bias;
//                disconnectSpecial(USBCentral::instance(),interface->infos().serial);
            }
        }
        m_isSpecial = false;

        QThread::msleep(100);

    return retour;
}

// must be done AFTER get_bias_informations
bool Hub::set_bias_informations(unsigned int bias, int numCarte, int nbCarte)
{
    m_isSpecial = true;

    bool retour = false;
    if (workingDevice != nullptr)
    {
//    CommunicationCentral * ccChecked = nullptr;

//        CommunicationInterface * interface = nullptr;
//        foreach (CommunicationCentral * cc, Hub::instance()->getCentrals())
//        {
//            foreach(CommunicationInterface * inter, cc->listInterfacesConnected())
//            {
//                qDebug()<<"set if ("<<inter->infos().m_hub<<" == ("<<numCarte<<"+"<<nbCarte<<"))";
//               if (inter->infos().m_hub == (numCarte+nbCarte)) //(numCarte+1))
//               {
//                   qDebug()<<"set OK if ("<<inter->infos().m_hub<<" == ("<<numCarte<<"+"<<nbCarte<<"))";

//                   interface = inter;
//                   ccChecked = cc;
//                   break;
//               }
//            }
//        }
//        if (interface != nullptr)
//        {
//            qDebug()<<"if apres";
//            fflush(stdout);

//            QString specialSerial = interface->infos().serial + "_Special";
//            if (!m_devicesSpecial.contains(specialSerial)) connectSpecial(USBCentral::instance(),interface->infos().serial);

//            qDebug()<<"connectSpecial apres";
//            fflush(stdout);
//            if (m_devicesSpecial.contains(specialSerial))
//            {
//                GenericDevice *device = m_devicesSpecial[specialSerial];
//                //retour = device->set_bias_informations(bias);
                auto mail = workingInterface->create();
                mail->fill(Mail::DEVICE_SET_BIAS,100);

                QDataStream stream(mail->tx(),QIODevice::ReadWrite);
                stream << bias;
                mail >> (*workingDevice);

                int cpt = 0;
                while ((cpt < 500) && (!mail->isProcessed()))
                {
                    QThread::msleep(10);
                    cpt++;
                }

                retour = true;

//                auto mailStop = workingInterface->create(GenericDevice::processedStopMail);
//                mail->fill(Mail::DEVICE_STOP_ACQUISITION,100);
//                QDataStream streamStop(mail->tx(),QIODevice::ReadWrite);
//                int code = 0xA5A5A5A5;
//                streamStop << code;
//                streamStop << code;
//                qDebug() << "mailStop";
//                mailStop >> (*workingDevice);
                //workingDevice->stop();
                // connected normally so stay connected

               // QThread::msleep(100);
                disconnectSpecial(USBCentral::instance(),workingInterface->infos().serial);
               // QThread::msleep(100);
    }

//            }
//        }

    workingInterface = nullptr;
    workingDevice = nullptr;

    m_isSpecial = false;

    QThread::msleep(500);

    return retour;
}

void Hub::finalise_bias_informations()
{
    m_isSpecial = false;
    //stop();
    CommunicationWrapper::resetInstance();
}


void Hub::processedBias(void *device, void *data)
{
    quint8* buffer = (quint8*)data;
    Hub::instance()->changeBias(Tools::bigEndianToInt(buffer));
}

void Hub::changeBias(unsigned int bias)
{
    m_bias = bias;
}
