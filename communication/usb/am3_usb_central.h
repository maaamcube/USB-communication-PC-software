#ifndef AM3_USBCENTRAL_H
#define AM3_USBCENTRAL_H

#include <QFuture>
#include <QTime>

#include "am3_communication_central.h"
#include "am3_usb_interface.h"

/*!
* \brief The USBCentral class purpose is to listen to events, check the connections of the platform and loop on the usbInterfaces to give the complete list of platform connected
*/
class USBCentral :  public CommunicationCentral
{
public:

    USBCentral();
    virtual ~USBCentral();

    virtual void init(Hub* hub) override;
    virtual void exit() override;
    virtual void waitExit() override;
    virtual void remove(const QString& serial) override;

    static USBCentral* instance();
    static void destroy();

private:
    void events();
    /*!
     * \brief refresh
     */
    void refresh();

    /*!
    * \brief check method loops on the devices list to check if usbInterfaces contains them to connect them to the Hub
    */
    void check();
    void update(qint32 elapsed);

    //QStringList isComplete(USBInterface* newOne);


private:
    bool m_eventRunning;
    bool m_checkRunning;

    QFuture<void> eventsResults;
    QFuture<void> checkResults;

    QTime m_time;

    static USBCentral* m_instance;
    QMap<libusb_device*,USBInterface*> m_usbInterfaces;
};

#endif // AM3_USBCENTRAL_H
