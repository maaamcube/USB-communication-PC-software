#ifndef AM3_MAIL_H
#define AM3_MAIL_H

#include <QSharedPointer>

class Firmware;
class Calibration;
class CalibrationFromFile;
class CommunicationInterface;
class GenericDevice;

typedef void (*processed_callback)(void* arg, void* data);

/*!
/*! * \brief The Mail class define the message to be send or received, its type, its content, its status and if it has been received.
/*! */
class Mail
{
    friend class CommunicationInterface;
 public:
    typedef enum {
        UNKNOWN = 0,
        RECEIVING_MAIL,
        BOOTLOADER_START,
        BOOTLOADER_CPLD_PROGRAM,
        BOOTLOADER_CPLD_CHECK,
        BOOTLOADER_CPLD_ERASE,
        BOOTLOADER_ACTIVATE,
        BOOTLOADER_RESERVED1,
        BOOTLOADER_RESERVED2,
        BOOTLOADER_RESERVED3,
        BOOTLOADER_RESERVED4,
        DEVICE_START_ACQUISITION,
        DEVICE_STOP_ACQUISITION,
        DEVICE_CALIBRATE,
        DEVICE_GET_CALIBRATION,
        DEVICE_RESET,
        DEVICE_GET_SENSORS,
        DEVICE_GET_BIAS,
        DEVICE_SET_BIAS,
        DEVICE_RESERVED2,
        DEVICE_RESERVED3,
        DEVICE_RESERVED4,
        DEVICE_RESERVED5,
        DEVICE_RESERVED6,
        DEVICE_RESERVED7,
        DEVICE_RESERVED8,
        DEVICE_START_ACQUISITION_V8,
        DEVICE_START_ACQUISITION_V8_ZIP,
    }MailType;


    typedef enum {
        WAITING_FOR_RECEPTION = 1,
        RECEIVING,
        RECEIVED,
        RECEPTION_TIMEOUT,
        RECEPTION_ERROR
    }MailStatus;


    Mail(QSharedPointer<Calibration> calibration, quint8 mailVersion, quint32 id, void *device);
    Mail(Firmware* firmware, quint8 mailVersion, quint32 id, void *device);
    Mail(MailType type, quint8 mailVersion, quint32 id, void *device);
    Mail(quint8 mailVersion, quint32 id, void *device);
    Mail(quint8 mailVersion, quint32 id, processed_callback callback, void *device);
    MailStatus status() const { return m_status; }

    quint32 partCount(quint32 packet) const;
    void copy(quint32 part, quint32 packet, quint8* buffer);
    void from(quint8* buffer, quint32 size);
    quint32 id() const;
    QString name() const;

    void changeStatus(MailStatus status, bool ignore = false);

    void addDelta(quint32 ms );
    QByteArray * tx();
    QByteArray * rx();
    QByteArray rxConst() const;
    MailType type() const;
    void resetTimeout();

    bool isAck(quint8 rxType);

    /*!
    /*! * \brief fill is the method to build the Mail and define all its characteristics.
    /*! * \param type give the MailType for instance DEVICE_GET_CALIBRATION.
    /*! * \param size give the size of the transmitted data.
    /*! */
    void fill(MailType type, quint32 size);

    bool isProcessed();
    friend void operator <<(QSharedPointer<Mail> &mail, const Firmware &mat);
    friend void operator >>(Mail &mail, Firmware &mat);
    friend void operator <<(QSharedPointer<Mail> &mail, const Calibration &calibration);
    friend void operator <<(QSharedPointer<Mail> &mail, const CalibrationFromFile &calibration);
    friend void operator >>(Mail &mail, Calibration &calibration);
    friend void operator >>(Mail &mail, CalibrationFromFile &calibration);

    GenericDevice *device() const;

    quint8 version() const;

private:
    quint32 m_id;
    quint8 m_txCodeType;
    QByteArray m_txData;
    quint8 m_rxCodeType;
    QByteArray m_rxData;
    quint32 m_offsetRx;
    quint32 m_currentRxIndex;
    unsigned int m_timeout;
    unsigned int m_currentTimeout;
    MailType m_type;
    quint8 m_version;
    MailStatus m_status;
    QString m_name;
    QSharedPointer<Calibration> m_calibration;
    Firmware* m_firmware;
    GenericDevice* m_device;
    processed_callback f_processed;

public:
    int compteur;
};


#endif // AM3_MAIL_H
