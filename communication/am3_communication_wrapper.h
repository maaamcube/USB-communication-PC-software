#ifndef AM3_COMMUNICATION_WRAPPER_H
#define AM3_COMMUNICATION_WRAPPER_H

#include <QObject>
#include "am3_hub.h"
#include "am3_sensors_values.h"

typedef QList<quint32> IdList;
typedef QList<Sparse> SparseList;
class CommunicationWrapper : public QObject
{
    Q_OBJECT
public:
    static CommunicationWrapper* instance();
    static void destroy();

    void signalAutoOffset();

    static bool static_waitForAutoOffset(QString serial);
    bool waitForAutoOffset(QString serial);

    static void static_connected(QString serial);
    void connected(QString serial);

    static void static_disconnected(QString serial);
    void disconnected(QString serial);

    static void static_available(QString serial,const QList<quint32>& ms,const QList<Sparse>& sparse);
    void available(QString serial,const QList<quint32>& ms,const QList<Sparse>& sparse);

    static void static_availableMap(QString serial, SensorsValues values, SensorsValues brutes);
    void availableMap(QString serial, SensorsValues values, SensorsValues brutes);

    static void resetInstance();
signals:
    bool devicewaitForAutoOffset(GenericDevice* device);
    void deviceConnected(GenericDevice* device);
    void deviceDisconnected(const QString serial);
    void deviceAcqAvailable(const QString& serial,IdList ms,SparseList sparse);
    void deviceAcqAvailableMap(SensorsValues values, SensorsValues brutes);

    /*!
     * \brief deviceAcqAvailableOrdered : signal to tell that data (Sparses) are available from the plaform
     * \param serial : the serial number of the platform
     * \param ms : the list of times corresponding to the list of sparses
     * \param sparse : the list of Sparses (often there is only one Sparse unless we are late)
     */
    void deviceAcqAvailableOrdered(const QString& serial,IdList ms,SparseList sparse);

public slots:

private:
    explicit CommunicationWrapper(QObject *parent = nullptr);
    void stop();
private:
    Hub* m_central;
    static CommunicationWrapper* m_instance;

};


Q_DECLARE_METATYPE(quint32);
Q_DECLARE_METATYPE(IdList);
Q_DECLARE_METATYPE(SparseList);
#endif // AM3_COMMUNICATION_WRAPPER_H
