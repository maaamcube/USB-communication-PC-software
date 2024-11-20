#ifdef WITH_FTD_32_BIT

#ifndef AM3_FTDIINTERFACE_H
#define AM3_FTDIINTERFACE_H

#include "am3_hardware_interface.h"
#include "am3_ftdi_endpoint.h"


class FTDIInterface : public HardwareInterface
{
public:
        FTDIInterface(const CommunicationInterfaceInfos &infos);
        virtual ~FTDIInterface();
        void init();
        virtual void rx(QSharedPointer<Mail> mail);
        virtual void linkDown();
        virtual bool isMailAccepted(const Mail::MailType& type);

        virtual void transfer(const QSharedPointer<Buffer<quint8>>& buffer);
        void receiving(void *channel, const QSharedPointer<Buffer<quint8>>& buffer);

        FT_HANDLE handle() const;
        void setHandle(const FT_HANDLE &handle);

        void stopInterface();
        void loopReceiving();
        QString serial() const;
        // envoyer command GS à la plateforme de type V5.201 pour connaitre le vrai n°serie dans le firmwaire
        QString getSerialNumber();


private:
        FT_HANDLE m_handle;
        QMap<quint32,FTDIEndpoint*> m_rx;
        QMap<quint32,FTDIEndpoint*> m_tx;

        QString m_serial;

};

#endif // AM3_FTDIINTERFACE_H
#endif
