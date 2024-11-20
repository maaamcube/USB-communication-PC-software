#include <QtConcurrent/QtConcurrent>
#include <QDebug>
#include "am3_usb_builder.h"
#include "am3_usb_central.h"
#include "am3_hub.h"
#include "am3_log.h"
#include "libusb.h"
#include <iostream>
#include "tools/am3_tools.h"

USBCentral* USBCentral::m_instance = nullptr;

USBCentral::USBCentral()
    : CommunicationCentral(),m_eventRunning(false),m_checkRunning(false)

{

    m_path_new = "123456789101112131415";
}

USBCentral::~USBCentral()
{
}

void USBCentral::init(Hub *hub)
{
    m_hub = hub;
    INIT_HW_LOG();
    INIT_FW_LOG();
    auto version = libusb_get_version ();
    qDebug() << "Version " << version->major << version->minor << version->micro << version->nano;
    libusb_init(nullptr);

    QThreadPool::globalInstance()->setMaxThreadCount(8);

    //libusb_set_option(NULL,LIBUSB_OPTION_LOG_LEVEL,LIBUSB_LOG_LEVEL_DEBUG  );
    eventsResults   = QtConcurrent::run(this,&USBCentral::events);
    checkResults    = QtConcurrent::run(this,&USBCentral::refresh);
    m_time.start();
}

void USBCentral::exit()
{
    //qDebug() << "Exit 1";
    m_eventRunning = false;
    m_checkRunning = false;
}

void USBCentral::waitExit()
{
    eventsResults.cancel();
    checkResults.cancel();
    eventsResults.waitForFinished();
    checkResults.waitForFinished();
    for(auto it=m_usbInterfaces.begin();it!=m_usbInterfaces.end();it++){
        delete it.value();
    }
    libusb_exit(nullptr);
}

void USBCentral::remove(const QString &serial)
{
    Q_UNUSED(serial)
}

USBCentral *USBCentral::instance()
{
    if(!m_instance)
        m_instance = new USBCentral();
    return m_instance;
}

void USBCentral::destroy()
{
    if (m_instance!=nullptr)
    {
        delete m_instance;
        m_instance = nullptr;
    }
}

void USBCentral::events()
{
    m_eventRunning = true;
    struct timeval tv = { 0, 2000 };
    while(m_eventRunning){
        libusb_handle_events_timeout_completed(nullptr,&tv,nullptr);
    }
}

void USBCentral::check()
{
    libusb_device **device_list;
    int deviceCount = libusb_get_device_list(nullptr, &device_list);
    auto connected = m_usbInterfaces.keys();

    libusb_device *dev;
    QMap<libusb_device*,USBInterface*> m_usbInterfacesTemp;
    for (int i = 0; i < deviceCount; i++)
    {
        dev = device_list[i];
        if(m_usbInterfaces.contains(dev))
        {
            connected.removeOne(dev);
        }
        else{
            auto interface = USBBuilder().create(dev);
            if(interface)
            {
                m_usbInterfaces[dev] = interface;
                m_usbInterfacesTemp[dev] = interface;
            }
        }
    }

    bool isSameHub = true;
    foreach(USBInterface* it, m_usbInterfacesTemp){
        auto path = it->infos().m_path;
        if (path.size() != m_path_new.size()) isSameHub = false;
        if (path.size() < m_path_new.size()) m_path_new = path;
    }

    QStringList serials;

    foreach(USBInterface* interface, m_usbInterfacesTemp)
    {
        if (isSameHub) m_path_new = interface->infos().m_path;

        if(interface->infos().m_devices_expected == 1){
            serials << interface->infos().serial;
        }else{
            QStringList temps;
            foreach(USBInterface* it, m_usbInterfaces)
            {
                QString serial = it->infos().serial;
                if(!m_interfaces.contains(serial)
                       && it->infos().m_path.contains(m_path_new))
                {
                    if (isSameHub)
                    {
                        qDebug() <<" not0"<<" "<<QTime::currentTime().toString("hh:mm:ss.zzz");
                        temps << QString("%1%2").arg(it->infos().m_hub).arg(serial);
                    }
                    else
                    {
                        qDebug() <<" not1"<<" "<<QTime::currentTime().toString("hh:mm:ss.zzz");
                        int orderPath = it->infos().m_path.size() - m_path_new.size();
                        qDebug()<<orderPath<<"  it "<<it->infos().m_hub;
                        if (it->infos().m_hub == 1) it->setHub(3+orderPath);
                        temps.prepend(QString::number(it->infos().m_hub) + serial);
                    }
                }
                else
                {
                    qDebug() << "notnot"<<" "<<QTime::currentTime().toString("hh:mm:ss.zzz");
                }
            }
            qSort(temps);
            if(temps.size() == interface->infos().m_devices_expected)
            {
                for(auto t : temps){
                    serials << t.remove(0,1);
                }
            }
        }

        if(!serials.isEmpty()){
            HW_INFO("USB device with serial {} has connected",serials.first().toUtf8().data() );
            for(auto serial : serials){
                for(auto it = m_usbInterfaces.begin();it!=m_usbInterfaces.end();it++){
                    if(it.value()->infos().serial == serial)
                        m_interfaces[serial] = it.value();
                }
            }

            //m_hub->connect(this,serials);
            m_hub->setSavedDevices(serials);
            connected.removeOne(dev);

            // stop
            if (Tools::CALIB_MODE)
                m_checkRunning = false;
        }
    }

    for(auto d : connected){
        qDebug() << "Remove from m_usbInterfaces" << m_usbInterfaces[d]->infos().serial;
        m_hub->disconnect(this,m_usbInterfaces[d]->infos().serial);
        m_interfaces.remove(m_usbInterfaces[d]->infos().serial);
        m_usbInterfaces.remove(d);
    }

    if (serials.size() > 0)
        m_hub->connect(this,serials);

    libusb_free_device_list(device_list, 1);
}

void USBCentral::update(qint32 elapsed)
{
    Q_UNUSED(elapsed)
}

void USBCentral::refresh()
{
    m_checkRunning = true;

    if (Tools::CALIB_MODE)
    {
        while(m_checkRunning){
            check();
            QThread::msleep(50);
        }
        return;
    }

    while(m_checkRunning){
        check();
        update(m_time.restart());
        for(auto i=0;i<10;i++){
            if(!m_checkRunning)
                return;
            QThread::msleep(50);
        }
    }
    //qDebug() << "Refresh finished";
}
