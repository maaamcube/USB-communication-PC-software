#ifndef AM3_COMMUNICATION_CENTRAL_H
#define AM3_COMMUNICATION_CENTRAL_H

#include "am3_singleton.h"
#include "am3_communication_interface.h"

#include <QMap>

class Hub;

class CommunicationCentral
{
public:
    CommunicationCentral();

    virtual void init(Hub* hub) = 0;
    virtual void waitExit() = 0;
    virtual void exit() = 0;
    virtual void remove(const QString& serial) = 0;

    virtual CommunicationInterface* get(const QString& serial);

    QList<CommunicationInterface *> listInterfacesConnected();
    void initCheck();
protected:
    Hub* m_hub;
    QMap<QString,CommunicationInterface*> m_interfaces;

    QString m_path_new;

};


#endif // AM3_COMMUNICATION_CENTRAL_H
