#ifdef WITH_FTD_32_BIT

#include <QtConcurrent/QtConcurrent>
#include "am3_ftdi_builder.h"
#include "am3_ftdi_central.h"
#include "am3_hub.h"
#include "ftd2xx.h"

FTDICentral* FTDICentral::m_instance = nullptr;

FTDICentral::FTDICentral()
    : CommunicationCentral(),m_checkRunning(false)
{
    SavedLowVal = 0 ;
    SavedLowDir = 0 ;
    SavedHighVal = 0 ;
    SavedHighDir = 0 ;
    current_state = undefined_jtag_state;
    m_started = false;
}

FTDICentral::~FTDICentral()
{
}

void FTDICentral::init(Hub *hub)
{
    m_hub = hub;
    QThreadPool::globalInstance()->setMaxThreadCount(8);

    eventsResults   = QtConcurrent::run(this,&FTDICentral::events);
    m_checkResults    = QtConcurrent::run(this,&FTDICentral::refresh);
    m_time.start();
}

void FTDICentral::exit()
{
    m_eventRunning = false;
    m_checkRunning = false;
}

void FTDICentral::waitExit()
{
    m_checkResults.cancel();
    eventsResults.cancel();
    m_checkResults.waitForFinished();
    eventsResults.waitForFinished();
    for(auto it=m_ftdiInterfaces.begin();it!=m_ftdiInterfaces.end();it++){
        delete it.value();
    }
//    libusb_exit(nullptr);
}

void FTDICentral::remove(const QString &serial)
{
    Q_UNUSED(serial)
//    m_hub->disconnect(this,m_ftdiInterfaces[serial]->infos().serial);
//    m_interfaces.remove(m_ftdiInterfaces[serial]->infos().serial);
//    m_ftdiInterfaces.remove(serial);
}

FTDICentral *FTDICentral::instance()
{
    if(m_instance == nullptr)
    {
        m_instance = new FTDICentral();
    }
    return m_instance;
}

void FTDICentral::destroy()
{
    if (m_instance!=nullptr)
    {
        delete m_instance;
        m_instance = nullptr;
    }
}

void FTDICentral::initFT_properties(FT_HANDLE ftHandle, bool latency)
{
    UCHAR LatencyTimer = 16;
    UCHAR Mask = 0;// sets the corresponding pin to an input
    DWORD cUSBWriteTO=0;
    DWORD cUSBReadTO=10;
    FT_ResetDevice(ftHandle);
    if (!latency)
    {
        FT_SetTimeouts(ftHandle,cUSBReadTO,cUSBWriteTO);
    }
    if (latency)
    {
        FT_SetLatencyTimer(ftHandle,LatencyTimer);
        UCHAR Mode = 0x0; //Reset
        FT_SetBitMode(ftHandle,Mask,Mode);
        Mode = 0x2; //0x2 = MPSSE (FT2232, FT2232H, FT4232H and FT232H devices only)
        FT_SetBitMode(ftHandle,Mask,Mode);
    }
}


void FTDICentral::check()
{
    FT_STATUS ftStatus;
    DWORD numDevs = 0;
    FT_HANDLE ftHandleTemp;
    char Buffer[64]; // more than enough room!
    char Description[64];
    QString serialNb;
    bool FT_failed = false;

    // create the device information list
    //ftStatus = FT_CreateDeviceInfoList(&numDevs);
    ftStatus = FT_ListDevices(&numDevs,nullptr,FT_LIST_NUMBER_ONLY);

    QList<QString> connected = m_ftdiInterfaces.keys();

    for (int iDev = 0 ;  iDev < numDevs; iDev++)
    {
        ftStatus = FT_ListDevices((PVOID)iDev,Description,FT_LIST_BY_INDEX|FT_OPEN_BY_DESCRIPTION);
        QString product = QString(Description);
        if (ftStatus == FT_OK)
        {
            ftStatus = FT_ListDevices((PVOID)iDev,Buffer,FT_LIST_BY_INDEX|FT_OPEN_BY_SERIAL_NUMBER);
            if (ftStatus == FT_OK)
            {
                serialNb = "0";
                for (int c = 0 ;  c < 16 ; c++)
                {
                    if (QChar('\0') == QChar(Buffer[c]))
                        c=16;
                    else
                        serialNb.append(QChar(Buffer[c]));
                }
                if(product.endsWith("B"))
                {
                    if (!m_ftdiInterfaces.contains(serialNb))
                    {
                        FTDIBuilder* builder = new FTDIBuilder();
                        FTDIInterface *interface = builder->create(iDev);
                        if (interface != nullptr)
                        {

                            m_ftdiInterfaces[serialNb] = interface;

                            for(auto it = m_ftdiInterfaces.begin();it!=m_ftdiInterfaces.end();it++)
                            {
                                if(it.value()->infos().serial == serialNb)
                                {
                                    m_interfaces[serialNb] = it.value();
                                }
                            }

                            ftStatus = FT_OpenEx(Description,FT_OPEN_BY_DESCRIPTION,&ftHandleTemp);
                            initFT_properties(ftHandleTemp, false);

                            if (ftStatus == FT_OK)
                            {
                                connected.removeAll(serialNb);

                                m_ftdiInterfaces[serialNb]->setHandle(ftHandleTemp);
                                auto serialNN = m_ftdiInterfaces[serialNb]->getSerialNumber();
                                qDebug()<<"serialN FROM V5 card ="<<serialNN;

                                m_ftdiInterfaces[serialNN] = m_ftdiInterfaces[serialNb];
                                m_interfaces[serialNN] = m_interfaces[serialNb];

                                m_ftdiInterfaces.remove(serialNb);
                                m_interfaces.remove(serialNb);

                                m_hub->connect(this,QStringList(serialNN));
                            }
                            else
                            {
                                qDebug()<<"failed to open interface for "<<serialNb;
                            }
                        }
                        else
                        {
                            qDebug()<<"failed to create interface for "<<serialNb;
                        }
                    }
                    else
                    {
                        connected.removeAll(serialNb);
                    }
                }
                else //A
                {
                    initProductA(Description);
                    connected.removeAll(serialNb);
                }
            }
            else
            {
                FT_failed = true;
            }
        }
        else
        {
            FT_failed = true;
        }
    }
    if ((!connected.isEmpty()) && (!FT_failed))
    {
        foreach (QString d, connected)
        {
            m_hub->disconnect(this,m_ftdiInterfaces[d]->infos().serial);
            m_interfaces.remove(m_ftdiInterfaces[d]->infos().serial);
            m_ftdiInterfaces.remove(d);
        }
    }
}

void FTDICentral::update(qint32 elapsed)
{
    Q_UNUSED(elapsed)
}

QMap<QString, FTDIInterface *> FTDICentral::ftdiInterfaces() const
{
    return m_ftdiInterfaces;
}

void FTDICentral::setStarted(bool started)
{
    m_started = started;
}

void FTDICentral::refresh()
{

    m_checkRunning = true;
    while(m_checkRunning){
        check();
        update(m_time.restart());
        for(auto i=0;i<10;i++){
            if(!m_checkRunning)
                return;
            QThread::msleep(50);
        }
    }
}

void FTDICentral::events()
{
    m_eventRunning = true;
    while(m_eventRunning)
    {
        if (m_started)
        {
            foreach (FTDIInterface * interFace, m_ftdiInterfaces)
            {
                interFace->loopReceiving();
            }
        }
        QThread::msleep(5);//200Hz enough because never above 100Hz
    }
}

void FTDICentral::initProductA(char Description[64])
{
    FT_STATUS ftStatus;
    FT_HANDLE ftHandleTemp;

    int speed= 0;

    ftStatus = FT_OpenEx(Description,FT_OPEN_BY_DESCRIPTION,&ftHandleTemp);
    initFT_properties(ftHandleTemp, true);

    if (ftStatus == FT_OK)
        {
        PurgeChip(ftHandleTemp);
        OutIndex= 0;

        QThread::msleep(20); // wait for all the USB stuff to complete

        AddToBuffer(0x80); // set I/O low bits all out except TDO
        SavedLowVal = SavedLowVal & 0xF0;
        SavedLowVal = SavedLowVal | 0x08; // TDI,TCK start low
        AddToBuffer(SavedLowVal);
        SavedLowDir= SavedLowDir & 0xF0;
        SavedLowDir = SavedLowDir | 0x0B;
        AddToBuffer(SavedLowDir);
        AddToBuffer(0x82); // output on GPIOH 1-4
        AddToBuffer(SavedHighVal);
        AddToBuffer(SavedHighDir);
        AddToBuffer(0x86); // set clk divisor
        AddToBuffer(speed & 0xFF);
        AddToBuffer(speed >>= 8);
        // turn off loop back
        AddToBuffer(0x85);
        SendBytes(ftHandleTemp, OutIndex); // send off the command

        Rst_tap(ftHandleTemp, run_test_idle); // reset TAP controller and put it in
        // run test idle
        //m_openPort= tOpenJTAG;
        INITMIC(ftHandleTemp);
        //****** fin init jtag
    }
}

void FTDICentral::PurgeChip(FT_HANDLE ftHandle)
{
    DWORD RxBytes = 0;
    DWORD BytesReceived;
    FT_STATUS ftStatus;

    FT_ResetDevice(ftHandle);
    // check for old data
    ftStatus = FT_GetQueueStatus(ftHandle,&RxBytes);
    if ( RxBytes > 0 )
    {
        char *RxBuffer = new char[RxBytes];
        ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
        delete[] RxBuffer;
    }

}

void FTDICentral::AddToBuffer(int i)
{
    FT_Out_Buffer[OutIndex] = i & 0xFF;
    OutIndex++;
}

void FTDICentral::SendBytes(FT_HANDLE ftHandle, int numBytes)
{
    DWORD Write_Result;
    FT_Write(ftHandle,&FT_Out_Buffer,numBytes,&Write_Result);
    OutIndex -= Write_Result;
}

void FTDICentral::Rst_tap(FT_HANDLE ftHandle, jtag_state state)
{
  bool Do_Read;
  int TMS_Clks;

  QThread::msleep(20); // wait for all the USB stuff to complete
  PurgeChip(ftHandle);
  QThread::msleep(20); // wait for all the USB stuff to complete
  OutIndex = 0;
  AddToBuffer(0x80); // set I/O low bits all out except TDO
  SavedLowVal = SavedLowVal & 0xF0;
  SavedLowVal = SavedLowVal | 0x08; // TDI,TCK start low
  AddToBuffer(SavedLowVal);
  SavedLowDir = SavedLowDir & 0xF0;
  SavedLowDir = SavedLowDir | 0x0B;
  AddToBuffer(SavedLowDir);
  current_state = undefined_jtag_state;
  Do_Read = false;
  TMS_Clks = wiggler_move_state(test_logic_reset,1,Do_Read);
  TMS_Clks = wiggler_move_state(state,0,Do_Read);
  SendBytes(ftHandle, OutIndex); // send off the command
  PurgeChip(ftHandle);

}


int FTDICentral::wiggler_move_state(jtag_state new_state, int last_data, bool Do_Read)
{
  int result = 0;
  if (current_state == undefined_jtag_state)
  {
    wiggler_do_tms(0x7F,7,false);
    current_state= test_logic_reset;
  }

  result = 0;

  switch (current_state)
  {
  case test_logic_reset:
      wiggler_do_tms(from_test_logic_reset[(quint8)(new_state)] | (last_data <<= 7),
      from_test_logic_resetc[(quint8)(new_state)],Do_Read);
      result = from_test_logic_resetc[(quint8)(new_state)];
      break;
  case run_test_idle:
      wiggler_do_tms(from_run_test_idle[(quint8)(new_state)] | (last_data <<= 7),
      from_run_test_idlec[(quint8)(new_state)],Do_Read);
      result = from_run_test_idlec[(quint8)(new_state)];
      break;
  case pause_dr:
      wiggler_do_tms(from_pause_dr[(quint8)(new_state)] | (last_data <<= 7),
      from_pause_drc[(quint8)(new_state)],Do_Read);
      result = from_pause_drc[(quint8)(new_state)];
      break;
  case pause_ir:
      wiggler_do_tms(from_pause_ir[(quint8)(new_state)] | (last_data <<= 7),
      from_pause_irc[(quint8)(new_state)],Do_Read);
      result = from_pause_irc[(quint8)(new_state)];
      break;
  case shift_dr:
      wiggler_do_tms(from_shift_dr[(quint8)(new_state)] | (last_data <<= 7),
      from_shift_drc[(quint8)(new_state)],Do_Read);
      result = from_shift_drc[(quint8)(new_state)];
      break;
  case shift_ir:
      wiggler_do_tms(from_shift_ir[(quint8)(new_state)] | (last_data <<= 7),
      from_shift_irc[(quint8)(new_state)],Do_Read);
      result = from_shift_irc[(quint8)(new_state)];
      break;
  default:
      break;
 }
  current_state= new_state;

  return result;
}

void FTDICentral::wiggler_do_tms(quint8 tmsval, quint8 count, bool Do_Read)
{
  if (count == 0) return;
  if (count > 7) return;
  if (Do_Read) AddToBuffer(0x6B); else AddToBuffer(0x4B);
  AddToBuffer(count-1);
  AddToBuffer(tmsval);
}

MicroTYPE FTDICentral::INITMIC(FT_HANDLE ftHandle)
{
  MicroTYPE Microloc;
  QString IDCODE;

  Rst_tap(ftHandle, run_test_idle);
  Microloc = none;
  Scan_Out_DATA(ftHandle, instruction_register, 16, IDCODE_IR);
  Scan_IN_DATA(ftHandle, data_register, IDCODE_LEN);
  IDCODE = QString::number(Array_BYTE_SCAN_IN[3])
         + QString::number(Array_BYTE_SCAN_IN[2])
         + QString::number(Array_BYTE_SCAN_IN[1])
         + QString::number(Array_BYTE_SCAN_IN[0]);

  if (IDCODE == IDCODE_FOR_F060)
  {
    Microloc             = C8051F060;
    FLASHCON             = 0x4082;   // 16 bits
    FLASHDAT             = 0x4083;   // 16 bits
    FLASHADRESS          = 0x4084;   // 16 bits
    FLASHSCALE           = 0x4085;   // 16 bits

    FLASHADRESS_LENGTH   = 16;
    FLCN_LEN             = 8;
    FLD_WRLEN            = 8;
    FLD_RDLEN            = 10;
    FLC_WRLEN            = 8;
    FLSC_LEN             = 8;
  }

  if (IDCODE == IDCODE_FOR_F120)
  {
    Microloc             = C8051F120;
    INITC8051F120(ftHandle);
    FLASHCON             = 0x3082;  // 16 bits
    FLASHDAT             = 0x3083;  // 16 bits
    FLASHADRESS          = 0x03084; // 17 bits
    FLASHSCALE           = 0x3085;  // 16 bits

    FLASHADRESS_LENGTH   = 17;
    FLCN_LEN             = 8;
    FLD_WRLEN            = 8;
    FLD_RDLEN            = 10;
    FLC_WRLEN            = 8;
    FLSC_LEN             = 8;
  }

  if ((IDCODE != IDCODE_FOR_F120) && (IDCODE != IDCODE_FOR_F060))
  {
    Microloc  = none;
    //ERROR_Status.Status  = true;
    //ERROR_Status.errordescription  = 'Bad IDCODE...';
  }

  //m_microType = Microloc;
  return Microloc;
}

void FTDICentral::INITC8051F120(FT_HANDLE ftHandle)
{
  quint8 OUTDATA_ARRAY[2];

  //   JTAG_Reset ();
  //   JTAG_IR_Scan (0x1801);        // abort POR (TCK)
  OUTDATA_ARRAY[0] = 0x01;
  OUTDATA_ARRAY[1] = 0x18;
  Scan_Out_DATA(ftHandle, instruction_register, 16, OUTDATA_ARRAY);

   //   JTAG_IR_Scan (0x1804);        // IDCODE (TCK)
  OUTDATA_ARRAY[0] = 0x04;
  OUTDATA_ARRAY[1] = 0x18;
  Scan_Out_DATA(ftHandle, instruction_register, 16, OUTDATA_ARRAY);

   //   T3_Waitus (65000);            // wait for /RST to rise
   QThread::msleep(7);
   //   JTAG_IR_Scan (0x2804);        // SYSRST (TCK)
   OUTDATA_ARRAY[0] = 0x04;
   OUTDATA_ARRAY[1] = 0x28;
   Scan_Out_DATA(ftHandle, instruction_register, 16, OUTDATA_ARRAY);

   //   JTAG_IR_Scan (0x2004);        // SYSRST (intosc)
  OUTDATA_ARRAY[0] = 0x04;
  OUTDATA_ARRAY[1] = 0x20;
  Scan_Out_DATA(ftHandle, instruction_register, 16, OUTDATA_ARRAY);

   //   JTAG_IR_Scan (0x1004);        // HALT (intosc)
  OUTDATA_ARRAY[0] = 0x04;
  OUTDATA_ARRAY[1] = 0x10;
  Scan_Out_DATA(ftHandle, instruction_register, 16, OUTDATA_ARRAY);

   //   JTAG_IR_Scan (0x4004);        // SUSPEND (intosc)
  OUTDATA_ARRAY[0] = 0x04;
  OUTDATA_ARRAY[1] = 0x40;
  Scan_Out_DATA(ftHandle, instruction_register, 16, OUTDATA_ARRAY);

  //    JTAG_IR_Scan (RESET);// RESET 2F FF
  OUTDATA_ARRAY[0] = 0xFF;
  OUTDATA_ARRAY[1] = 0x2F;
  Scan_Out_DATA(ftHandle, instruction_register, 16, OUTDATA_ARRAY);

  //    JTAG_IR_Scan (0x30c1);
  OUTDATA_ARRAY[0] = 0xC1;
  OUTDATA_ARRAY[1] = 0x30;
  Scan_Out_DATA(ftHandle, instruction_register, 16, OUTDATA_ARRAY);

  //    Set FLRD timing to 100MHz (required to access FLASH through
  //    debug port
  //    JTAG_IWrite (ireg:0x30c1, dat:0x8400L, 17);
  Array_BYTE_SCAN_OUT[0] =0x00;
  Array_BYTE_SCAN_OUT[1] =0x84;
  INDIRECT_WRITE_(ftHandle, 0x30c1,17,Array_BYTE_SCAN_OUT);

  //    JTAG_IWrite (0x30c1, 0xb731L, 17);
  Array_BYTE_SCAN_OUT[0] =0x31;
  Array_BYTE_SCAN_OUT[1] =0xB7;
  INDIRECT_WRITE_(ftHandle, 0x30c1,17,Array_BYTE_SCAN_OUT);

  //    Switch to internal osc divide by 1
  //    JTAG_IWrite (0x30c1, 0x840FL, 17);
  Array_BYTE_SCAN_OUT[0] =0x0F;
  Array_BYTE_SCAN_OUT[1] =0x84;
  INDIRECT_WRITE_(ftHandle, 0x30c1,17,Array_BYTE_SCAN_OUT);

  //    JTAG_IWrite (0x30c1, 0x8a83L, 17);
  Array_BYTE_SCAN_OUT[0] =0x83;
  Array_BYTE_SCAN_OUT[1] =0x8A;
  INDIRECT_WRITE_(ftHandle, 0x30c1,17,Array_BYTE_SCAN_OUT);

  //    JTAG_IWrite (0x30c1, 0x8400L, 17);
  Array_BYTE_SCAN_OUT[0] =0x00;
  Array_BYTE_SCAN_OUT[1] =0x84;
  INDIRECT_WRITE_(ftHandle, 0x30c1,17,Array_BYTE_SCAN_OUT);

  //    JTAG_IR_Scan (IDCODE);
  //    JTAG_IR_Scan (FLASHADR | STATCTL);
}

void FTDICentral::Scan_Out_DATA(FT_HANDLE ftHandle, shift_register IRREGISTER, WORD Numofbits, quint8 out_data[])
{
  jtag_state state;
  state = run_test_idle;
  s_out(ftHandle, IRREGISTER, Numofbits, &out_data[0], state);
}

void FTDICentral::Scan_IN_DATA(FT_HANDLE ftHandle, shift_register IRREGISTER, WORD Numofbits)
{
  jtag_state state;
  quint8 TDILevel;
  bool SendImmediate;

  state = run_test_idle;
  TDILevel = 0;
  SendImmediate = true;
  s_in(ftHandle, IRREGISTER,Numofbits,Array_BYTE_SCAN_IN,state,TDILevel,SendImmediate);
}


//**********************************************************************************************************************
void FTDICentral::INDIRECT_WRITE_(FT_HANDLE ftHandle, WORD Addresse_instruction, quint8 Length_of_Adress_instruction, quint8 data_to_write[])
{
  quint8 OUTDATA_ARRAY[5];
  WORD tempAdr;
  // Load IR with register to be read
  OUTDATA_ARRAY[0] = Addresse_instruction & 0x00FF ;
  tempAdr = (Addresse_instruction & 0xFF00);
  OUTDATA_ARRAY[1] = tempAdr >>= 8 ;
  Scan_Out_DATA(ftHandle, instruction_register, 16, OUTDATA_ARRAY);

  OUTDATA_ARRAY[0] = 0;
  OUTDATA_ARRAY[1] = 0;
  OUTDATA_ARRAY[2] = 0;

  //Load DR with 'Write' IndOpcode code
  if (Length_of_Adress_instruction  == 0 )
  {
    OUTDATA_ARRAY[0] = 0x3;
  }

  if (Length_of_Adress_instruction  == 8)
  {
    // Put WRITE OPCODE ( 11b ) into  OUTDATA_ARRAY[0]
    OUTDATA_ARRAY[1] = 0x3;
    OUTDATA_ARRAY[0] = OUTDATA_ARRAY[0] | data_to_write[0];
  }

  if (Length_of_Adress_instruction ==  17)
  {
    // Put WRITE OPCODE ( 11b ) into  OUTDATA_ARRAY[0]
    OUTDATA_ARRAY[0] =  OUTDATA_ARRAY[0] | data_to_write[0];
    OUTDATA_ARRAY[1] =  OUTDATA_ARRAY[1] | data_to_write[1];
    OUTDATA_ARRAY[2] = 0x6 | data_to_write[2];
  }

  if (Length_of_Adress_instruction ==  16)
  {
    // Put WRITE OPCODE ( 11b ) into  OUTDATA_ARRAY[0]
    OUTDATA_ARRAY[0] =   OUTDATA_ARRAY[0] | data_to_write[0];
    OUTDATA_ARRAY[1] =   OUTDATA_ARRAY[1] | data_to_write[1];
    OUTDATA_ARRAY[2] =   0x3 | data_to_write[2];
  }

  Scan_Out_DATA(ftHandle, data_register,Length_of_Adress_instruction+2,OUTDATA_ARRAY);

  POLLING_BUSY(ftHandle);
}




//**********************************************************************************************************************
void FTDICentral::POLLING_BUSY(FT_HANDLE ftHandle)
{
  quint8 OUTPUSH_1[2];
  unsigned int INPULL_1[2] ;
  quint8 TIMEOUT;
  quint32 TestPoll;

  //Polling busy
  INPULL_1[0] = 0x0;
  OUTPUSH_1[0]= 0x0;
  TIMEOUT     = 0;
  do
  {
    TestPoll = Scan_IN_Out_DATA(ftHandle, data_register, 1, OUTPUSH_1[0] ,INPULL_1[0]);
    TestPoll = TestPoll & 1;
    TIMEOUT++;
    //sleep(1);
  }
  while (!((TestPoll == 0) || (TIMEOUT == 50)));

  if (TIMEOUT == 50)
  {
  ////            ERROR_Status.Status = true;
  ////            ERROR_Status.errordescription = ' TIME OUT, polling_busy';
  }
}




//**********************************************************************************************************************
quint32 FTDICentral::Scan_IN_Out_DATA(FT_HANDLE ftHandle, shift_register IRREGISTER, WORD Numofbits, quint32 DATAWORD_OUT, quint32 DATAWORD_IN)
{
  jtag_state state;
  bool SendImmediate;
  unsigned int INSCANDATA[51];
  quint8 OUTSCANDATA[51];
  quint32 tempData;

  //state           = run_test_idle;
  SendImmediate   = true;
  //Rst_tap(run_test_idle);

  OUTSCANDATA[0] = DATAWORD_OUT & 0x00FF;
  tempData = (DATAWORD_OUT & 0xFF00) ;
  OUTSCANDATA[1] = tempData >>= 8;

  INSCANDATA[0] = 0;
  INSCANDATA[1] = 0;


  s_io (ftHandle, IRREGISTER,Numofbits,OUTSCANDATA,INSCANDATA,run_test_idle,true);
  return INSCANDATA[0];//(INSCANDATA[0]) OR (INSCANDATA[1] shl  8);
}


//**********************************************************************************************************************
// JTAG Scan in and out
void FTDICentral::s_io(FT_HANDLE ftHandle, shift_register jtag_register, WORD bit_length, quint8 out_data[], unsigned int in_data[], jtag_state state, bool SendImmediate)
{
  int i,j,TMS_Clks;
  quint8 LastBit;
  WORD Mod_bit_length;
  bool Do_Read,passed;

  OutIndex = 0;
  j = 0;
  Mod_bit_length = bit_length - 1;
  Do_Read = false;
  if (jtag_register == instruction_register)
    TMS_Clks = wiggler_move_state(shift_ir,0,Do_Read);
  else
    TMS_Clks = wiggler_move_state(shift_dr,0,Do_Read);

  if ((Mod_bit_length / 8) > 0)
  { // do whole bytes
    i = (Mod_bit_length / 8) - 1;
    AddToBuffer(0x39); // clk data bytes out on -ve in +ve clk LSB
    AddToBuffer(i & 0xFF);
    AddToBuffer((i / 256) & 0xFF);
    // now add the data bytes to go out
      do
      {
          AddToBuffer(out_data[j]);
          j = j + 1;
      }
      while (j <= i);
  }
  if ((Mod_bit_length % 8) > 0)
  { // do remaining bits
    i = (Mod_bit_length % 8) - 1;
    AddToBuffer(0x3B); // clk data bits out on -ve in +ve clk LSB
    AddToBuffer(i & 0xFF);
    // now add the data bits to go out
    AddToBuffer(out_data[j]);
  }
  // get LastBit  modified

  LastBit = out_data[j];
  //
  j = bit_length % 8;
   if ((bit_length % 8) == 0)    // mod : reste de la division
   {
      LastBit = LastBit >>= (8-j-1); // True if only lenghtbit /4 = integer
   }
   if ((bit_length % 8) > 0)   // mod : reste de la division
   {
       LastBit = LastBit >>= (j-1);  // True if only lenghtbit /4 <> integer
   }
   //end of modification

  Do_Read = true;
  TMS_Clks = wiggler_move_state(state,LastBit,Do_Read); // end it in state passed in
  if (SendImmediate) AddToBuffer(0x87);
  SendBytes(ftHandle, OutIndex); // send off the command
  // now wait for data
  passed = Get_Data(ftHandle, in_data,bit_length,TMS_Clks);
  //
}


//**********************************************************************************************************************
// Routines to Control MPSSE hardware
// This will work out the number of whole bytes to read and adjust for the TMS read
bool FTDICentral::Get_Data(FT_HANDLE ftHandle, unsigned int in_data[], WORD Bit_Length, int TMS_Clks)
{
  FT_STATUS ftStatus;
  DWORD RxBytes, BytesReceived;
  int NoBytes,i,j;
  int BitShift,Mod_Bit_Length;
  quint8 Last_Bit;
  quint8 Temp_Buffer[64001];
  int TotalBytes;
  QDateTime timeout_start;
  QDateTime timeout_end;
  bool timeout;

  bool result = false;
  i = 0;
  Mod_Bit_Length = Bit_Length - 1; // adjust for bit count of 1 less than no of bits
  NoBytes = Mod_Bit_Length / 8;  // get whole bytes
  BitShift = Mod_Bit_Length % 8; // get remaining bits
  if (BitShift > 0) NoBytes = NoBytes + 1; // bump whole bytes if bits left over
  BitShift = 8 - BitShift; // adjust for SHR of incoming byte
  NoBytes = NoBytes + 1; // add 1 for TMS read byte
  i = 0;
  TotalBytes = 0;
  do
  {
    timeout_start = QDateTime::currentDateTime();
    do
    {
      ftStatus = FT_GetQueueStatus(ftHandle,&RxBytes);
      if ( RxBytes == 0)  QThread::msleep(0); // give up timeslice
      timeout_end = QDateTime::currentDateTime();
      timeout = (timeout_start.msecsTo(timeout_end)>=500);//    modified used to be 5000
    }
    while (!((RxBytes > 0) || (ftStatus != FT_OK) || timeout));

    if (RxBytes > 0)
    {
      quint8 *RxBuffer = new quint8[RxBytes];
      ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
      for (int i = 0 ; i < BytesReceived ; i++)
      {
        Temp_Buffer[TotalBytes] = RxBuffer[i];
        TotalBytes++;
      }
      delete[] RxBuffer;
    }
  }
  while (!((TotalBytes >= NoBytes) || (ftStatus != FT_OK) || timeout));

  if ((!timeout) && (ftStatus == FT_OK))
  {
    result = true;
    //adjust last 2 bytes
    if (BitShift < 8 )
    {
      Temp_Buffer[NoBytes-2] = Temp_Buffer[NoBytes-2] >>= BitShift;
      Last_Bit = Temp_Buffer[NoBytes-1] <<= (TMS_Clks-1);
      Last_Bit = Last_Bit & 0x80; // strip the rest
      Temp_Buffer[NoBytes-2] = Temp_Buffer[NoBytes-2] | (Last_Bit >>= (BitShift-1));
      for (int j = 0 ; j < (NoBytes-2); j++)
      {
        in_data[j] = Temp_Buffer[j];
      }
    }
    else // case for 0 bit shift in data + TMS read bit
    {
      Last_Bit = Temp_Buffer[NoBytes-1] <<= (TMS_Clks-1);
      Last_Bit = Last_Bit >>= 7; // strip the rest
      Temp_Buffer[NoBytes-1] = Last_Bit;
      for (int j = 0 ; j < (NoBytes-1) ; j++)
      {
        in_data[j] = Temp_Buffer[j];
      }
    }
  }
  return result;
}

//**********************************************************************************************************************
void FTDICentral::s_out(FT_HANDLE ftHandle, shift_register jtag_register, WORD bit_length, quint8 out_data[], jtag_state state)
{
  int i,j,TMS_Clks;
  quint8 LastBit;
  WORD Mod_bit_length;
  bool Do_Read;

  OutIndex = 0;
  j = 0;
  Mod_bit_length = bit_length - 1;
  Do_Read = false;

  if (jtag_register == instruction_register)
    TMS_Clks = wiggler_move_state(shift_ir,0,Do_Read);
  else
    TMS_Clks = wiggler_move_state(shift_dr,0,Do_Read);

  if ((Mod_bit_length / 8) > 0 )   // division
  { // do whole bytes
    i = (Mod_bit_length / 8) - 1;
    AddToBuffer(0x19); // clk data bytes out on -ve clk LSB
    AddToBuffer(i & 0xFF);
    AddToBuffer((i / 256) & 0xFF);
    // now add the data bytes to go out
    do
    {
      AddToBuffer(out_data[j]);
      j = j + 1;
    }
    while (j <= i);
  }

  if ((Mod_bit_length % 8) > 0)   // mod : reste de la division
  { // do remaining bits
    i = (Mod_bit_length % 8) - 1;
    AddToBuffer(0x1B); // clk data bits out on -ve clk LSB
    AddToBuffer(i & 0xFF);
    // now add the data bits to go out
    AddToBuffer(out_data[j]);
  }
  // get LastBit modified
  LastBit = out_data[j];
  //
  j = bit_length % 8;
   if ((bit_length % 8) == 0)   // mod : reste de la division
   {
    LastBit = LastBit >>= (8-j-1); // True if only lenghtbit /4 = integer
   }
  if ((bit_length % 8) > 0)   // mod : reste de la division
  {
     LastBit = LastBit >>= (j-1);  // True if only lenghtbit /4 <> integer
  }
   // end of modification
  TMS_Clks = wiggler_move_state(state,LastBit,Do_Read); // end it in state passed in
  SendBytes(ftHandle, OutIndex); // send off the command

}


//**********************************************************************************************************************
// JTAG Scan in
void FTDICentral::s_in(FT_HANDLE ftHandle, shift_register jtag_register, WORD bit_length, unsigned int in_data[], jtag_state state, quint8 TDILevel, bool SendImmediate)
{
  int i,TMS_Clks;
  WORD Mod_bit_length;
  bool Do_Read;
  bool Passed;

  OutIndex = 0;
  Mod_bit_length = bit_length - 1;
  Do_Read = false;

  if (jtag_register == instruction_register)
  {
    TMS_Clks = wiggler_move_state(shift_ir,TDILevel,Do_Read);
  }
  else
  {
    TMS_Clks = wiggler_move_state(shift_dr,TDILevel,Do_Read);
  }

  if ((Mod_bit_length / 8) > 0)
  { // do whole bytes
    i = (Mod_bit_length / 8) - 1;
    AddToBuffer(0x28); // clk data bytes in LSB +ve clk
    AddToBuffer(i & 0xFF);
    AddToBuffer((i / 256) & 0xFF);
  }

  if ((Mod_bit_length % 8) > 0)
  { // do remaining bits
    i = (Mod_bit_length % 8) - 1;
    AddToBuffer(0x2A); // clk data bits in LSB +ve clk
    AddToBuffer(i & 0xFF);
  }
  Do_Read = true;
  TMS_Clks = wiggler_move_state(state,TDILevel,Do_Read); // end it in state passed in

  if (SendImmediate) AddToBuffer(0x87);

  SendBytes(ftHandle, OutIndex); // send off the command
  // now wait for data
  Passed = Get_Data(ftHandle, in_data,bit_length,TMS_Clks);
}

#endif
