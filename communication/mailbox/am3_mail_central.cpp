#include "am3_mail_central.h"

MailCentral::MailCentral(quint8 mailVersion, void *device)
{
    m_version = mailVersion;
    m_uniqueId = 1;
    m_device = device;
}

QSharedPointer<Mail> MailCentral::create()
{
    auto mail = QSharedPointer<Mail>(new Mail(m_version,m_uniqueId,m_device));
    m_mails[m_uniqueId++] = mail;
    return mail;
}

QSharedPointer<Mail> MailCentral::create(processed_callback callback)
{
    auto mail = QSharedPointer<Mail>(new Mail(m_version,m_uniqueId,callback,m_device));
    m_mails[m_uniqueId++] = mail;
    return mail;
}

QSharedPointer<Mail> MailCentral::create(Mail::MailType type)
{
    auto mail = QSharedPointer<Mail>(new Mail(type,m_version,m_uniqueId,m_device));
    m_mails[m_uniqueId++] = mail;
    return mail;
}

QSharedPointer<Mail> MailCentral::create(quint8 *buffer)
{
    Q_UNUSED(buffer);
    return QSharedPointer<Mail>(nullptr);
}

QSharedPointer<Mail> MailCentral::create(QSharedPointer<Calibration> calibration)
{
    auto mail = QSharedPointer<Mail>(new Mail(calibration,m_version,m_uniqueId,m_device));
    m_mails[m_uniqueId++] = mail;
    return mail;

}

QSharedPointer<Mail> MailCentral::create(Firmware *firmware)
{
    auto mail = QSharedPointer<Mail>(new Mail(firmware,m_version,m_uniqueId,m_device));
    m_mails[m_uniqueId++] = mail;
    return mail;
}

void MailCentral::clear()
{
    m_mails.clear();
}

void MailCentral::remove(QSharedPointer<Mail> mail)
{
    m_mails.remove(mail->id());
}

