#ifdef WITH_FTD_32_BIT

#ifndef AM3_FTDI_CENTRAL_H
#define AM3_FTDI_CENTRAL_H

#include <QFuture>
#include "am3_singleton.h"
#include "am3_communication_central.h"
#include "am3_ftdi_interface.h"


typedef enum {
    test_logic_reset,
    run_test_idle,
    pause_dr,
    pause_ir,
    shift_dr,
    shift_ir,
    undefined_jtag_state,
}jtag_state;

typedef enum {
    C8051F120,
    C8051F060,
    C8051F132,
    none,
}MicroTYPE;

typedef enum {
    instruction_register,
    data_register,
}shift_register;

class FTDICentral : public CommunicationCentral
{
public:
    FTDICentral();
    virtual ~FTDICentral();

    virtual void init(Hub* hub) override;
    virtual void exit() override;
    virtual void waitExit() override;
    virtual void remove(const QString& serial) override;

    //static FTDICentral* instance();
    //static void destroy();

    static FTDICentral* instance();
    static void destroy();

    QMap<QString, FTDIInterface *> ftdiInterfaces() const;

    void initFT_properties(FT_HANDLE ftHandle, bool latency);
    void initProductA(char Description[64]);
    void PurgeChip(FT_HANDLE ftHandle);
    void AddToBuffer(int i);
    void SendBytes(FT_HANDLE ftHandle, int numBytes);
    void Rst_tap(FT_HANDLE ftHandle, jtag_state state);
    int wiggler_move_state(jtag_state new_state, int last_data, bool Do_Read);
    void wiggler_do_tms(quint8 tmsval, quint8 count, bool Do_Read);
    MicroTYPE INITMIC(FT_HANDLE ftHandle);
    void Scan_Out_DATA(FT_HANDLE ftHandle, shift_register IRREGISTER, WORD Numofbits, quint8 out_data[]);
    void Scan_IN_DATA(FT_HANDLE ftHandle, shift_register IRREGISTER, WORD Numofbits);
    void INITC8051F120(FT_HANDLE ftHandle);
    void INDIRECT_WRITE_(FT_HANDLE ftHandle, WORD Addresse_instruction, quint8 Length_of_Adress_instruction, quint8 data_to_write[]);
    void POLLING_BUSY(FT_HANDLE ftHandle);
    quint32 Scan_IN_Out_DATA(FT_HANDLE ftHandle, shift_register IRREGISTER, WORD Numofbits, quint32 DATAWORD_OUT, quint32 DATAWORD_IN);
    void s_io(FT_HANDLE ftHandle, shift_register jtag_register, WORD bit_length, quint8 out_data[], unsigned int in_data[], jtag_state state, bool SendImmediate);
    bool Get_Data(FT_HANDLE ftHandle, unsigned int in_data[], WORD Bit_Length, int TMS_Clks);
    void s_out(FT_HANDLE ftHandle, shift_register jtag_register, WORD bit_length, quint8 out_data[], jtag_state state);
    void s_in(FT_HANDLE ftHandle, shift_register jtag_register, WORD bit_length, unsigned int in_data[], jtag_state state, quint8 TDILevel, bool SendImmediate);
    void setStarted(bool started);
private:
    void events();
    void refresh();

    /*!
    /*! * \brief check method loops on the devices list to check if usbInterfaces contains them to connect them to the Hub
    /*! */
    void check();
    void update(qint32 elapsed);

private:
    bool m_eventRunning;
    bool m_checkRunning;

    QFuture<void> eventsResults;
    QFuture<void> m_checkResults;

    bool m_started;

    QTime m_time;

    static FTDICentral* m_instance;

    QMap<QString, FTDIInterface*> m_ftdiInterfaces;

    int OutIndex;
    char FT_Out_Buffer[65536];
    int SavedLowVal;
    int SavedLowDir;
    int SavedHighVal;
    int SavedHighDir;
    jtag_state current_state;


    //======================================================================
    // go to ->                                    tlr rti pdr pir sdr sir
    //======================================================================
    const quint8 from_test_logic_reset[6] = {0x01,0x00,0x0a,0x16,0x02,0x06};
    const quint8 from_run_test_idle[6]    = {0x07,0x00,0x05,0x0b,0x01,0x03};
    const quint8 from_pause_dr[6]         = {0x1f,0x03,0x17,0x2f,0x01,0x0f};
    const quint8 from_pause_ir[6]         = {0x1f,0x03,0x17,0x2f,0x07,0x01};
    const quint8 from_shift_dr[6]         = {0x1f,0x03,0x01,0x2f,0x00,0x00};
    const quint8 from_shift_ir[6]         = {0x1f,0x03,0x17,0x01,0x00,0x00};
    //======================================================================
    // with this number of clocks
    //======================================================================
    const quint8 from_test_logic_resetc[6] = {1,1,5,6,4,5};
    const quint8 from_run_test_idlec[6]    = {3,5,4,5,3,4};
    const quint8 from_pause_drc[6]         = {5,3,6,7,2,6};
    const quint8 from_pause_irc[6]         = {5,3,6,7,5,2};
    const quint8 from_shift_drc[6]         = {5,3,2,7,0,0};
    const quint8 from_shift_irc[6]         = {5,4,6,2,0,0};

    quint8 IDCODE_IR[2] = {0x04,0x00};
    const quint8 IDCODE_LEN = 32;
    // ID CODE Values
    const QString  IDCODE_FOR_F120     = "10007243";
    const QString  IDCODE_FOR_F060     = "00006243";

    unsigned int Array_BYTE_SCAN_IN[0xFFFFFF];
    quint8 Array_BYTE_SCAN_OUT[0xFFFFFF];

    DWORD FLASHCON;     // 16 bits
    DWORD FLASHDAT;     // 16 bits
    DWORD FLASHADRESS;     // 17 bits or 16 bits
    DWORD FLASHSCALE;     // 16 bits

    quint8 FLASHADRESS_LENGTH;
    quint8 FLCN_LEN          ;
    quint8 FLD_WRLEN         ;
    quint8 FLD_RDLEN         ;
    quint8 FLC_WRLEN         ;
    quint8 FLSC_LEN          ;

};

#endif // AM3_FTDI_CENTRAL_H
#endif
