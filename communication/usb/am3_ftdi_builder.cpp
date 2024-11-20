#ifdef WITH_FTD_32_BIT

#include "am3_ftdi_builder.h"
#include "am3_devices_defines.h"
#include "ftd2xx.h"

FTDIBuilder::FTDIBuilder()
{
//    m_templates[QString::number(fw_usb_pid)     +"0000000000065000"]   = {"", fw_id, "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
//    m_templates[QString::number(fw_usb_pid)     +"0000000100064000"]   =
//    {"", fw_id, "", "", "", "",  0x1040,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
//    m_templates[QString::number(fwp_usb_pid)     +"0000000100064000"]   =
//    {"", fw_id, "", "", "", "",  0x1040,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};


       m_templates[QString("Dual Device A")]   = {"","Dual Device A" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("Dual Device B")]   = {"","Dual Device B" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("IST1 A")]   = {"","IST1 A" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("IST1 B")]   = {"","IST1 B" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("AMCube1 A")]   = {"","AMCube1 A" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("AMCube1 B")]   = {"","AMCube1 B" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("IST2 A")]   = {"","IST2 A" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("IST2 B")]   = {"","IST2 B" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("AMC2 A")]   = {"","AMC2 A" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("AMC2 B")]   = {"","AMC2 B" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};

       m_templates[QString("IST3 A")]   = {"","IST3 A" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("IST3 B")]   = {"","IST3 B" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("AMC3 A")]   = {"","AMC3 A" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("AMC3 B")]   = {"","AMC3 B" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};

       m_templates[QString("IST6 A")]   = {"","IST6 A" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("IST6 B")]   = {"","IST6 B" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("AMC6 A")]   = {"","AMC6 A" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("AMC6 B")]   = {"","AMC6 B" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};

       m_templates[QString("A AMCUBE A")]   = {"","A AMCUBE A" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("A AMCUBE B")]   = {"","A AMCUBE B" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("B AMCUBE A")]   = {"","B AMCUBE A" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("B AMCUBE B")]   = {"","B AMCUBE B" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("C AMCUBE A")]   = {"","C AMCUBE A" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("C AMCUBE B")]   = {"","C AMCUBE B" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("D AMCUBE A")]   = {"","D AMCUBE A" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("D AMCUBE B")]   = {"","D AMCUBE B" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};

       m_templates[QString("A AMC6 A")]   = {"","A AMC6 A" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("A AMC6 B")]   = {"","A AMC6 B" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("B AMC6 A")]   = {"","B AMC6 A" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("B AMC6 B")]   = {"","B AMC6 B" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("C AMC6 A")]   = {"","C AMC6 A" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("C AMC6 B")]   = {"","C AMC6 B" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("D AMC6 A")]   = {"","D AMC6 A" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
       m_templates[QString("D AMC6 B")]   = {"","D AMC6 B" , "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
}

FTDIInterface* FTDIBuilder::create(int numDev)
{
    FTDIInterface *returnedInterFace = nullptr;

    FT_STATUS ftStatus, ftStatus2;
    DWORD numDevs;
    DWORD aNumDev = numDev;
    char Buffer[64]; // more than enough room!
    char Description[64];
    QString serialNb;


    // create the device information list
    ftStatus = FT_CreateDeviceInfoList(&numDevs);
    if ((ftStatus == FT_OK) && (aNumDev < numDevs))
    {
        ftStatus = FT_ListDevices((PVOID)aNumDev,Buffer,FT_LIST_BY_INDEX|FT_OPEN_BY_SERIAL_NUMBER);
        ftStatus2 = FT_ListDevices((PVOID)aNumDev,Description,FT_LIST_BY_INDEX|FT_OPEN_BY_DESCRIPTION);
    }
    if ((ftStatus == FT_OK) && (ftStatus2 == FT_OK) && (aNumDev < numDevs))
    {
        serialNb = "0";
        for (int c = 0 ;  c < 16 ; c++)
        {
            if (QChar('\0') == QChar(Buffer[c]))
                c=16;
            else
                serialNb.append(QChar(Buffer[c]));
        }

        CommunicationInterfaceInfos infos;

        QString serial  = serialNb;

        QString product = QString(Description);

        //libFTDI_get_string_descriptor_ascii(handle,FTDI_index_hardware_version,version,20);
        QString hardware_version = QString("FTDI hardware");

        //libFTDI_get_string_descriptor_ascii(handle,FTDI_index_cpld_version,version,20);
        QString cpld_version = QString("FTDI cpld");

        //libFTDI_get_string_descriptor_ascii(handle,FTDI_index_microcontroller_version,version,20);
        QString microcontroller_version_V1_2 = QString("FTDI microcontroller V1_2");
        QString microcontroller_version_V4_5 = QString("FTDI microcontroller V4_5");

        //libFTDI_get_string_descriptor_ascii(handle,FTDI_index_bootloader_version,version,20);
        QString bootloader_version = QString("FTDI bootloader");

        if(!serial.isEmpty())
        {
            QString aKey("");
            foreach(QString key, m_templates.keys())
            {
                if (product.contains(key))
                {
                    aKey = key;
                    break;
                }
            }
            if(aKey != "")
            {
                infos = m_templates[aKey];
                infos.m_hub = 3;
                infos.m_path = QString("");
                infos.serial = serial;
                infos.product = product;
                infos.hardware_version = hardware_version;
                infos.cpld_version = cpld_version;
                if (product.contains("V1") || product.contains("V2")) infos.microcontroller_version = microcontroller_version_V1_2;
                else infos.microcontroller_version = microcontroller_version_V4_5;
                infos.bootloader_version = bootloader_version;

                qDebug()<<"infos = m_templates["<<aKey<<")];    "<<infos.serial<<"   "<<infos.product<<"   "<<infos.hardware_version<<"   "<<infos.m_devices_expected ;

                returnedInterFace = new FTDIInterface(infos);
                returnedInterFace->init();

            }
        }
    }
    return returnedInterFace;
}

#endif
