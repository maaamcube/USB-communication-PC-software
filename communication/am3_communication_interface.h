#ifndef AM3_COMMUNICATIONINTERFACE_H
#define AM3_COMMUNICATIONINTERFACE_H

#include <QQueue>
#include "am3_mail_central.h"
#include "am3_buffer_pool.h"



class CommunicationLink;

struct CommunicationInterfaceInfos
{
    QString serial;
    QString product;
    QString hardware_version;
    QString cpld_version;
    QString microcontroller_version;
    QString bootloader_version;
    quint32 messageSize;
    quint8 mailVersion;
    QList<quint32> m_rx;
    QList<quint32> m_tx;
    quint8 m_devices_expected;
    quint8 m_hub;
    QString m_path;
};


class CommunicationInterface
{
    friend class CommunicationLink;
public:
    CommunicationInterface(const CommunicationInterfaceInfos& infos);
    ~CommunicationInterface();

    void send(QSharedPointer<Mail> mail);
    void send(const QSharedPointer<Buffer<quint8> > &buffer);
    QSharedPointer<Mail> create();
    QSharedPointer<Mail> create(processed_callback proc);
    QSharedPointer<Mail> create(Mail::MailType type);
    QSharedPointer<Mail> create(quint8* buffer);

    void processed();
    void error();
    bool isDown() const;

    CommunicationInterfaceInfos infos() const;

    void setHub(quint8 hub);

    void setLink(CommunicationLink* link);
    CommunicationLink* getLink();

protected:
    virtual void receive(QSharedPointer<Mail> mail) = 0;
    virtual void rawReceive(const QSharedPointer<Buffer<quint8> > &buffer,quint8 hub) = 0;
    virtual bool isMailAccepted(const Mail::MailType& type) = 0;
    virtual void linkDown() = 0;

protected:
    CommunicationLink* m_link;
    QQueue<QSharedPointer<Mail>> m_queue;
    CommunicationInterfaceInfos m_infos;
    BufferPool<quint8> m_pool;
    MailCentral m_central;

};

#endif // AM3_COMMUNICATIONINTERFACE_H
