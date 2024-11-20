#include "am3_usb_builder.h"
#include "am3_devices_defines.h"
#include "libusb.h"

USBBuilder::USBBuilder()
{
    for (int i = 0 ; i < 100 ; i++)
    {
        m_templates[QString::number(fw_usb_pid)     +"00000000000"+QString::number(65000+i)]   =
        {"", fw_id, "", "", "", "",  0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
        m_templates[QString::number(fw_usb_pid)     +"00000001000"+QString::number(64000+i)]   =
        {"", fw_id, "", "", "", "",  0x1040,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
        m_templates[QString::number(fwp_usb_pid)     +"00000001000"+QString::number(64000+i)]   =
        {"", fw_id, "", "", "", "",  0x1040,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
        m_templates[QString::number(fwp_usb_pid)    +"00000000000"+QString::number(65000+i)]   =
        {"", fsr_s_id, "", "", "", "", 0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
        m_templates[QString::number(fsr_s_usb_pid)  +"00000000000"+QString::number(65000+i)]   =
        {"", fsr_s_id, "", "", "", "", 0,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
        m_templates[QString::number(fsr_m_usb_pid)  +"00000000000"+QString::number(65000+i)]   =
        {"", fsr_m_id, "", "", "", "", 0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,2,0,""};
        m_templates[QString::number(fsr_l_usb_pid)  +"00000000000"+QString::number(65000+i)]   =
        {"", fsr_l_id, "", "", "", "", 0x440,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,3,0,""};

        m_templates[QString::number(fsr_s_usb_v8_pid)  +"00000000000"+QString::number(65000+i)]   =
        {"", fsr_s_id, "", "", "", "", 0x1040,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
        m_templates[QString::number(fsr_m_usb_v8_pid)  +"00000000000"+QString::number(65000+i)]   =
        {"", fsr_m_id, "", "", "", "", 0x1040,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,2,0,""};
        m_templates[QString::number(fsr_l_usb_v8_pid)  +"00000000000"+QString::number(65000+i)]   =
        {"", fsr_l_id, "", "", "", "", 0x1040,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,3,0,""};

        m_templates[QString::number(fsr_s_usb_v8_pid)  +"00000000000"+QString::number(82800+i)]   =
        {"", fsr_s_id, "", "", "", "", 0x1040,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,1,0,""};
        m_templates[QString::number(fsr_m_usb_v8_pid)  +"00000000000"+QString::number(82800+i)]   =
        {"", fsr_m_id, "", "", "", "", 0x1040,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,2,0,""};
        m_templates[QString::number(fsr_l_usb_v8_pid)  +"00000000000"+QString::number(82800+i)]   =
        {"", fsr_l_id, "", "", "", "", 0x1040,0,QList<quint32>() << 0x81 << 0x83 << 0x85,QList<quint32>() << 0x02,3,0,""};
   }
}

USBInterface* USBBuilder::create(libusb_device* device)
{
    struct libusb_device_descriptor desc;
    CommunicationInterfaceInfos infos;
    if(libusb_get_device_descriptor(device, &desc) == 0){
        if(desc.idVendor == usb_vid){

            libusb_device_handle* handle;
            if(libusb_open(device,&handle) == 0){
                unsigned char s[100], p[20], version[20];

                uint8_t ports[7];
                auto size = libusb_get_port_numbers (device, ports, 7);

                libusb_get_string_descriptor_ascii(handle,desc.iSerialNumber,s,100);
                QString serial  = QString((char*)s);

                libusb_get_string_descriptor_ascii(handle,desc.iProduct,p,20);
                QString product = QString((char*)p);

                libusb_get_string_descriptor_ascii(handle,usb_index_hardware_version,version,20);
                QString hardware_version = QString((char*)version);

                libusb_get_string_descriptor_ascii(handle,usb_index_cpld_version,version,20);
                QString cpld_version = QString((char*)version);

                libusb_get_string_descriptor_ascii(handle,usb_index_microcontroller_version,version,20);
                QString microcontroller_version = QString((char*)version);

                libusb_get_string_descriptor_ascii(handle,usb_index_bootloader_version,version,20);
                QString bootloader_version = QString((char*)version);



                QString key = QString::number(desc.idProduct)
                                +hardware_version
                                +microcontroller_version;
                if(!serial.isEmpty() && m_templates.contains(key)){
                    infos = m_templates[key];
                    if(infos.m_devices_expected == 1)
                        infos.m_hub = 3;
                    else
                    {
                        infos.m_hub = ports[size-1];
                    }

                    for(auto i=0;i<size-1;i++)
                    {
                        infos.m_path += QString::number(ports[i]);
                    }
                    infos.serial = serial;
                    infos.product = product;
                    infos.hardware_version = hardware_version;
                    infos.cpld_version = cpld_version;
                    infos.microcontroller_version = microcontroller_version;
                    infos.bootloader_version = bootloader_version;
//                    for (int p = 0 ; p < 7 ; p++) qDebug()<<p<< "  port   "<<ports[p];
                    qDebug()<<"infos = m_templates["<<key<<"];  "<<size-1
                          <<" m_hub="<<infos.m_hub <<"\r\n"
                          <<" m_path="<<infos.m_path <<"\r\n"
                          <<" serial="<<infos.serial <<"\r\n"
                          <<" product="<<infos.product <<"\r\n"
                          <<" hardware_version="<<infos.hardware_version <<"\r\n"
                          <<" m_devices_expected="<<infos.m_devices_expected <<"\r\n"
                          <<" cpld_version="<<infos.cpld_version <<"\r\n"
                          <<" bootloader_version="<<infos.bootloader_version <<"\r\n"
                          <<" microcontroller_version="<<infos.microcontroller_version<<"\r\n"
                          <<" m_path="<<infos.m_path<<"\r\n"
                          <<" mailVersion="<<infos.mailVersion<<"\r\n"
                         <<" hub="<<infos.m_hub ;

                return new USBInterface(handle,infos);

                }
                libusb_close(handle);
            }
        }
    }
    return nullptr;
}
