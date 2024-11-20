#ifndef AM3_MAIL_CENTRAL_H
#define AM3_MAIL_CENTRAL_H

#include <QMap>

#include "am3_mail.h"

class MailCentral
{
public:
    MailCentral(quint8 mailVersion,void* device);

    QSharedPointer<Mail> create();
    QSharedPointer<Mail> create(processed_callback callback);
    QSharedPointer<Mail> create(Mail::MailType type);
    QSharedPointer<Mail> create(quint8* buffer);
    QSharedPointer<Mail> create(QSharedPointer<Calibration> calibration);
    QSharedPointer<Mail> create(Firmware* firmware);
    void clear();

    void remove(QSharedPointer<Mail> mail);


private:
    QMap<quint32,QSharedPointer<Mail>> m_mails;
    quint8 m_version;
    quint32 m_uniqueId;
    void* m_device;
};

#endif // AM3_MAIL_CENTRAL_H
