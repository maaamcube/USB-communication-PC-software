#ifndef AM3_COMMUNICATIONLINK_H
#define AM3_COMMUNICATIONLINK_H

#include <QMap>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QQueue>

#include "am3_communication_interface.h"

class Hub;

class CommunicationLink
{
public:
    CommunicationLink(Hub* hub,
                      CommunicationInterface* a,
                      QList<CommunicationInterface*> b);

    ~CommunicationLink();

    void transfer(QSharedPointer<Mail>, CommunicationInterface* emitter);
    void transfer(const QSharedPointer<Buffer<quint8>>& buffer, CommunicationInterface* emitter);
    void processed(CommunicationInterface *receiver);
    void error(CommunicationInterface *receiver);
    bool isDown() const;

    virtual void run();
    void stop();

    void preStop();
    int declareDeadLink();

private:
    void tryReceipt(CommunicationInterface* );
    void init(CommunicationInterface* interface, bool a);

private:
    Hub* m_hub;
    QMap<CommunicationInterface*,QQueue<QSharedPointer<Mail>>> m_fifos_a;
    QMap<CommunicationInterface*,QQueue<QSharedPointer<Mail>>> m_fifos_b;
    QMap<CommunicationInterface*,unsigned int> m_timeoutCount;
    QMutex m_mutex;
    bool m_deadLink;
    bool m_started;
    int delta = 10;
};
#endif // AM3_COMMUNICATIONLINK_H
