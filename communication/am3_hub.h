#ifndef AM3_HUB_H
#define AM3_HUB_H

#include "am3_singleton.h"
#include "am3_generic_device.h"
#include "am3_communication_central.h"
#include "am3_communication_link.h"

#include <QString>
#include <am3_ftdi_central.h>
#include "am3_sensors_values.h"

typedef void (*connected_cbk)(QString serial);
typedef bool (*autoOffset_cbk)(QString serial);
typedef void (*disconnected_cbk)(QString serial);
typedef void (*status_update_cbk)(QString serial,GenericDevice::Status status);
typedef void (*acquisition_available_cbk)(QString serial,const QList<quint32>& ms,const QList<Sparse>& sparse);
typedef void (*acquisition_available_map)(QString serial, SensorsValues values, SensorsValues brutes);

/*!
* \brief The Hub class is central to the library because every connection to a device and every acquisition will go through a Hub.
*/
class Hub
{
public:
    Hub();
    virtual ~Hub();

    static Hub* instance();
    static void destroy();

    void start();
    void stop();
    void stopPartial();
    void restartWithAutoOffset();

    void deadLink(CommunicationLink* link);

    /*!
    * \brief connect establish a connection with the first device that can be identified from the list of serials with the central communication.
     * It build the device, setup the callbacks and load the calibration
    * \param central a CentralCommunication to get the serials infos
    * \param serials the QStringList of CommunicationInterface names (serial numbers) available from the USB interfaces
    */
    void connect(CommunicationCentral* central, const QStringList &serials);
    void disconnect(CommunicationCentral* central,const QString& serial);
    void connectSpecial(CommunicationCentral *central, const QString serial);
    void disconnectSpecial(CommunicationCentral *central, const QString &serial);

    void autoOffset(const QString& serial);


    QStringList connected() const { return m_links.keys(); }

    GenericDevice* device(const QString& serial);

    void registerAutoOffset(autoOffset_cbk cbk);
    void registerConnect(connected_cbk cbk);
    void registerDisconnect(disconnected_cbk cbk);
    void registerStatus(status_update_cbk cbk);
    void registerAcq(acquisition_available_cbk cbk);
    void registerAcqMap(acquisition_available_map cbk);


    static void s_acqAvailable(GenericDevice* device, QList<quint32> ms, QList<Sparse> sparses);
    void acqAvailable(GenericDevice* device, QList<quint32> ms, QList<Sparse> sparses);

    static void s_acqAvailable_map(GenericDevice *device, SensorsValues values, SensorsValues brutes);
    void acqAvailableMap(GenericDevice* device, SensorsValues values, SensorsValues brutes);

    static void s_statusUpdate(GenericDevice* device);
    void statusUpdate(GenericDevice* device);

    quint32 get_older_frame_timestamp(GenericDevice* device);

    QList<GenericDevice*> number_connected();

    QList<CommunicationCentral *> getCentrals() const;

    QMap<QString, GenericDevice *> getDevices() const;

    virtual unsigned int get_bias_informations(int numCarte, int nbCarte = 3);
    virtual bool set_bias_informations(unsigned int bias, int numCarte, int nbCarte = 3);
    static void processedBias(void *device, void *data);
    void changeBias(unsigned int bias);


    void finalise_bias_informations();
    void prepare_bias_informations(int nbCartes);
    void reConnectSpecial(const QString &serial);
    GenericDevice * restoreDevices();

    void setSavedDevices(QStringList serials);

    // Necessaire pour outil de calibration
    unsigned int get_bias_information(int cardNumber);
    bool set_bias_information(unsigned int bias, int cardNumber);

    void fireConnected(QString serial);


    void signalAutoOffset();


private:

    bool m_mutexAutoOffset = false;


    void addCentral(CommunicationCentral* central);
#ifdef WITH_FTD_32_BIT
    void addCentralFTDI(FTDICentral* central);
    QList<FTDICentral*>                m_centralsFTDI;
#endif
    GenericDevice *connectDevicesForSpecialCommunication(int cardNumber);

    QList<CommunicationCentral*>                m_centrals;

    QMap<QString,GenericDevice*>                m_devices;
    QMap<QString,CommunicationLink*>            m_links;
    QMap<QString,GenericDevice*>                m_devicesSpecial;
    QMap<QString,CommunicationLink*>            m_linksSpecial;


    autoOffset_cbk              f_autoOffset;
    connected_cbk               f_connected;
    disconnected_cbk            f_disconnected;
    status_update_cbk           f_status;
    acquisition_available_cbk   f_acquisition;
    acquisition_available_map   f_acquisition_map;

    static Hub* m_instance;

    unsigned int m_bias;
    bool m_isSpecial;
    QString m_serial;
    QMap<int, QString> m_mapSpecialSerial;
    QMap<QString, bool> m_savedDevices;

    GenericDevice *workingDevice;
    CommunicationInterface * workingInterface;

};

#endif // AM3_HUB_H
