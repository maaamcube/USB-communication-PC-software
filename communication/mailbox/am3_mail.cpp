#include <QFile>
#include <QtGlobal>

#include "am3_calibration.h"
#include "am3_firmware.h"
#include "am3_mail.h"
#include "am3_tools.h"
#include "am3_log.h"
#include "am3_generic_device.h"


Mail::Mail(quint8 mailVersion, quint32 id,processed_callback callback, void*  device)
{
    m_type = Mail::UNKNOWN;
    m_currentTimeout = 0;
    m_status = WAITING_FOR_RECEPTION;
    m_timeout = 700;
    m_version = mailVersion;
    m_id = id;
    m_offsetRx = 4u;
    m_calibration = nullptr;
    m_firmware = nullptr;
    f_processed = callback;
    m_device = (GenericDevice*)device;
}

Mail::Mail(QSharedPointer<Calibration> calibration, quint8 mailVersion, quint32 id, void *device)
{
    m_version = mailVersion;
    m_id = id;
    m_calibration = calibration;
    m_firmware = nullptr;
    f_processed = nullptr;
    m_device = (GenericDevice*)device;
    fill(Mail::DEVICE_GET_CALIBRATION,calibration->size());
}

Mail::Mail(Firmware *firmware, quint8 mailVersion, quint32 id, void *device)
{
    m_version = mailVersion;
    m_id = id;
    m_firmware = firmware;
    m_calibration = nullptr;
    f_processed = nullptr;
    m_device = (GenericDevice*)device;
    fill(Mail::BOOTLOADER_CPLD_CHECK,firmware->byteSize());
}

Mail::Mail(Mail::MailType type, quint8 mailVersion, quint32 id, void *device)
{
    m_currentTimeout = 0;
    m_type = type;
    m_status = WAITING_FOR_RECEPTION;
    m_timeout = 700;
    m_version = mailVersion;
    m_id = id;
    m_offsetRx = 4u;
    m_calibration = nullptr;
    m_firmware = nullptr;
    f_processed = nullptr;
    m_device = (GenericDevice*)device;
}

Mail::Mail(quint8 mailVersion, quint32 id, void *device)
{
    m_type = Mail::UNKNOWN;
    m_currentTimeout = 0;
    m_status = WAITING_FOR_RECEPTION;
    m_timeout = 700;
    m_version = mailVersion;
    m_id = id;
    m_offsetRx = 4u;
    m_calibration = nullptr;
    m_firmware = nullptr;
    f_processed = nullptr;
    m_device = (GenericDevice*)device;
}

quint32 Mail::partCount(quint32 packet) const
{
    BEGIN_LOG("Mail::partCount");
    HW_DEBUG("txData size {}, packet {}, result {}",m_txData.size(),packet,(static_cast<quint32>(m_txData.size())/(packet-8)) + 1);
    //qDebug()<<"txData size {}, packet {}, result {}   "<<m_txData.size()<<"  "<<packet<<"  "<<(static_cast<quint32>(m_txData.size())/(packet-8)) + 1;
    return (static_cast<quint32>(m_txData.size())/(packet-8)) + 1;
}

void Mail::copy(quint32 part, quint32 packet, quint8 *buffer)
{
    BEGIN_LOG("Mail::copy");
    HW_DEBUG("Copying data from mail {} to buffer {:x}, part {}",m_name.toUtf8().data(),reinterpret_cast<quint64>(buffer),part);
    quint32 code = 0x32ab <<16;

    switch((int)m_version){
    case 0:{
            code |= part << 8;
            code |= m_txCodeType;
            auto index = 0u;
            index += Tools::intToBigEndian(buffer,code);
            if(this->partCount(packet) > 1)
                index += Tools::intToBigEndian(&buffer[index],part);
            quint8* data = reinterpret_cast<quint8*>(&m_txData.data()[part*(packet-8)]);
            quint16 size = qMin(packet,(m_txData.size() - part*(packet-8))+8);
            for(auto i=0u;index<size;i++,index++){
                buffer[index] = data[i];
            }
        }
        break;

    case 1:{
            code |= m_txCodeType;
            auto index = 0u;
            index += Tools::intToBigEndian(buffer,code);
            if(this->partCount(packet) > 1)
                index += Tools::intToBigEndian(&buffer[index],part);
            quint8* data = reinterpret_cast<quint8*>(&m_txData.data()[part*(packet-8)]);
            quint16 size = qMin(packet,(m_txData.size() - part*(packet-8))+8);
            for(auto i=0u;index<size;i++,index++){
                buffer[index] = data[i];
            }
        }
        break;
    }
}

void Mail::from(quint8 *buffer, quint32 size)
{

////debug code
//    ////qDebug()<<size<<"    0000  if((type() == Mail::DEVICE_CALIBRATE) || (type() == Mail::DEVICE_GET_CALIBRATION))     "<<((type() == Mail::DEVICE_CALIBRATE) || (type() == Mail::DEVICE_GET_CALIBRATION));
//        if (type() == Mail::DEVICE_CALIBRATE)// || (type() == Mail::DEVICE_GET_CALIBRATION))
//        {
//            QString fichier;
//            if (type() == Mail::DEVICE_GET_CALIBRATION)   fichier = "C:\\BUFFER\\received_DEVICE_GET_CALIBRATION.txt";
//            if (type() == Mail::DEVICE_CALIBRATE)  fichier = "C:\\BUFFER\\received_DEVICE_CALIBRATE.txt";
//    ////qDebug()<<"ouveture deeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee "<<fichier;
//            QFile file(fichier); // Appel du constructeur de la classe QFile

//            file.open(QIODevice::WriteOnly | QIODevice::Append);
//            // Si l'ouverture du fichier en écriture à réussie

//            // écrire dans le fichier en utilisant un flux :
//            QTextStream out(&file);

//            out<<"\n"<<compteur<< " size     "<<size<<"    m_offsetRx  "<<m_offsetRx<<"\n";
//            compteur++;


//            for(auto i=m_offsetRx;i<size;i++){
//                m_rxData.append(static_cast<char>(buffer[i]));
//                out<<"      "<<(buffer[i]);
//                if (((i-m_offsetRx+1)%10) == 0) out<<"\n";
//            }

//            out<<"\n\n\n";
//            file.close();
//        }
//        else
//        {


//            for(auto i=m_offsetRx;i<size;i++){
//                m_rxData.append(static_cast<char>(buffer[i]));
//            }




//        }

//original code
    BEGIN_LOG("Mail::from");
    HW_DEBUG("Copying data from buffer {:x} to mail {} ",reinterpret_cast<quint64>(buffer),m_name.toUtf8().data());
    for(auto i=m_offsetRx;i<size;i++){
        m_rxData.append(static_cast<char>(buffer[i]));
    }
}


quint32 Mail::id() const
{
    return m_id;
}

QString Mail::name() const
{
    return m_name;
}

void Mail::changeStatus(MailStatus status, bool ignore)
{

    BEGIN_LOG("Mail::changeStatus");
    if(m_calibration && status == Mail::RECEIVED && !ignore){
        HW_DEBUG("Calibrationb received {}",this->id());
        (*this) >> m_calibration;
        m_device->changeStatus(GenericDevice::CONNECTED);
        m_device->connect();
    }



    if(m_firmware && status == Mail::RECEIVED)
        (*this) >> (*m_firmware);

    m_status = status;
    //qDebug()<<"   f_processed ************************************************** "<<f_processed<<"      "<<(int)this<<"    m_status = "<<status;
    if(f_processed &&  status == Mail::RECEIVED)
        f_processed((void*)m_device,this->m_rxData.data());


}

void Mail::addDelta(quint32 ms)
{
    BEGIN_LOG("Mail::addDelta");
    m_currentTimeout += ms;
    if( m_currentTimeout >= m_timeout){
        m_status = RECEPTION_TIMEOUT;
        //qDebug() << "Timeout mail {}" << this->id();
        HW_DEBUG("Timeout mail {}",this->id());
    }
}

QByteArray *Mail::tx()
{
    return &m_txData;
}

QByteArray *Mail::rx()
{
    return &m_rxData;
}

QByteArray Mail::rxConst() const
{
    return m_rxData;
}

Mail::MailType Mail::type() const
{
    return m_type;
}

void Mail::resetTimeout()
{
    m_currentTimeout = 0;
}

bool Mail::isAck(quint8 rxType)
{
    return (m_rxCodeType == rxType);
}


void Mail::fill(Mail::MailType type,quint32 size)
{
    //qDebug() << "Fill " << type;
    m_status = WAITING_FOR_RECEPTION;
    m_currentTimeout = 0;
    m_timeout = 5000;
    m_currentRxIndex = 0;
    switch(type){
    case Mail::DEVICE_GET_CALIBRATION:
        m_type = Mail::DEVICE_GET_CALIBRATION;
        m_rxCodeType = 0x01;
        m_txCodeType = 0x01;
        m_txData.resize(static_cast<int>(size));
        Tools::intToBigEndian((quint8*)&m_txData.data()[0],0);
        m_name = "DEVICE_GET_CALIBRATION";
        m_offsetRx = 8u;
        break;

    case Mail::DEVICE_CALIBRATE:
        m_type = Mail::DEVICE_CALIBRATE;
        m_rxCodeType = 0x02;
        m_txCodeType = 0x02;
        m_timeout = 200000;
        m_name = "DEVICE_CALIBRATE";
        break;

    case Mail::DEVICE_START_ACQUISITION:
        m_type = Mail::DEVICE_START_ACQUISITION;
        m_rxCodeType = 0x04;
        m_txCodeType = 0x03;
        m_timeout = 5000;
        m_name = "DEVICE_START";
        break;

    case Mail::DEVICE_START_ACQUISITION_V8:
        m_type = Mail::DEVICE_START_ACQUISITION;
        m_rxCodeType = 0x20;
        m_txCodeType = 0x1E;
        m_timeout = 5000;
        m_name = "DEVICE_START_V8";
        break;

    case Mail::DEVICE_START_ACQUISITION_V8_ZIP:
        m_type = Mail::DEVICE_START_ACQUISITION;
        m_rxCodeType = 0x21;
        m_txCodeType = 0x1F;
        m_timeout = 5000;
        m_name = "DEVICE_START_V8_ZIP";
        break;

    case Mail::DEVICE_STOP_ACQUISITION:
        m_type = Mail::DEVICE_STOP_ACQUISITION;
        m_rxCodeType = 0x05;
        m_txCodeType = 0x04;
        m_timeout = 5000;
        m_name = "DEVICE_STOP";
        break;

    case Mail::BOOTLOADER_CPLD_PROGRAM:
        m_type = Mail::BOOTLOADER_CPLD_PROGRAM;
        m_rxCodeType = 0x15;
        m_txCodeType = 0x13;
        m_timeout = 5000;
        m_name = "CPLD_PROGRAM";
        break;

    case Mail::DEVICE_GET_SENSORS:
        m_type = Mail::DEVICE_GET_SENSORS;
        m_rxCodeType = 0x1D;
        m_txCodeType = 0x1B;
        m_timeout = 5000;
        m_name = "GET_SENSORS_INFOS";
        break;

    case Mail::DEVICE_GET_BIAS:
            m_type = Mail::DEVICE_GET_BIAS;
            m_rxCodeType = 0x1E;
            m_txCodeType = 0x1C;
            m_timeout = 5000;
            m_name = "DEVICE_GET_BIAS";
            break;

    case Mail::DEVICE_SET_BIAS:
        m_type = Mail::DEVICE_SET_BIAS;
        m_rxCodeType = 0x1F;
        m_txCodeType = 0x1D;
        m_timeout = 5000;
        m_name = "DEVICE_SET_BIAS";
        break;

    case Mail::BOOTLOADER_CPLD_CHECK:{
        m_type = Mail::BOOTLOADER_CPLD_CHECK;
        m_rxCodeType = 0x16;
        m_txCodeType = 0x14;
        m_timeout = 5000;
        m_offsetRx = 8u;
        auto parts = (size/(m_device->infos().communicationInfos.messageSize - 20 ));
        m_txData.resize(static_cast<int>(parts * m_device->infos().communicationInfos.messageSize)+20);
        for(auto i=0;i<parts+1;i++){
            auto address = m_firmware->address() + (i*(m_device->infos().communicationInfos.messageSize - 20 ));
            auto size = m_device->infos().communicationInfos.messageSize - 20 ;
            auto module = 1;
            auto offset = i*(m_device->infos().communicationInfos.messageSize - 8) ;

            Tools::intToBigEndian((quint8*)&m_txData.data()[offset],address);
            Tools::intToBigEndian((quint8*)&m_txData.data()[offset+4],size);
            Tools::intToBigEndian((quint8*)&m_txData.data()[offset+8],module);
        }
        m_name = "CPLD_CHECK";
    }
        break;
    }
}

bool Mail::isProcessed()
{
    return m_status == Mail::RECEIVED;
}

GenericDevice *Mail::device() const
{
    return m_device;
}

quint8 Mail::version() const
{
    return m_version;
}

