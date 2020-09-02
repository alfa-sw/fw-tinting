/* 
 * File:   serialCom_GUI.h
 * Author: michele.abelli
 *
 * Created on 14 maggio 2019, 14.49
 */

#ifndef _SERIAL_COM_GUI_H_
#define _SERIAL_COM_GUI_H_

#define  MAX_GUI_RETRY  3

#define IS_FROM_GUI(id) \
  ((id) == 201)

#define MAB_DEVICE_ID 200
#define GUI_DEVICE_ID 201

#define STORE_BYTE(buf, c)                         \
  do {                                             \
    (buf).buffer[(buf).index ++ ] = (c);           \
    if ((buf).index >= BUFFER_SIZE)                \
    {                                              \
      SIGNAL_ERROR(buf);                           \
    }                                              \
  } while(0)


/**
 * @brief Resets the receiver
 */
#define RESET_RECEIVER(buf)                     \
  do {                                          \
    initBuffer(&buf);                           \
  } while(0)

/**
 * @brief Last char was an escape?
 */
#define IS_ESCAPE(buf)                          \
  (buf.escape != FALSE)

/**
 * @brief Signal last char was an escape
 */
#define SIGNAL_ESCAPE(buf)                      \
  do {                                          \
    buf.escape = TRUE;                          \
  } while (0)

/**
 * @brief Signal last char was not an escape
 */
#define CLEAR_ESCAPE(buf)                       \
  do {                                          \
    buf.escape = FALSE;                         \
  } while (0)

/**
 * @brief Serial error?
 */
#define IS_ERROR(buf)                           \
  (buf.bufferFlags.serialError != FALSE)


/**
 * @brief Signal serial error
 */
#define SIGNAL_ERROR(buf)                       \
  do {                                          \
    buf.bufferFlags.serialError = TRUE;         \
  } while (0)

#define IS_FROM_DISPLAY(id)                     \
  ((id) < 100)

#define IS_FROM_SLAVE(id)                       \
  ((id) > 100)

/**
 * @brief The device ID for MASTER -> SLAVE msgs
 */
#define MASTER_DEVICE_ID(id)                    \
  (id)

/**
 * @brief True iff ID is a valid query or reply ID
 */
#define IS_VALID_GUI_ID(id) \
  ((id == 200) || (id == 201))

/**
 * @brief All fixed position values are transferred with
 * a positive offset to avoid clashes with reserved bytes.
 */
#define ADD_OFFSET(c)       \
  ((c) + 0x20)
#define REMOVE_OFFSET(c)    \
  ((c) - 0x20)

// just an alias for code uniformity
#define STUFF_BYTE(buf, ndx, c)     \
  stuff_byte((buf), &(ndx), (c))

/**
 * @brief low-level write c into buf and increment index.
 * @param buf, the buffer to write to
 * @param ndx, the index to write c at
 * @param c, the character to be written
 */
#define WRITE_BYTE(buf, ndx, c)                   \
  do {                                            \
    *((buf) + (ndx)) = (c);                       \
    ++ (ndx);                                     \
  } while (0)

/**
 * @brief Frame initialization
 */
#define FRAME_BEGIN(txb, idx, id)                                     \
  do {                                                                \
    WRITE_BYTE((txb)->buffer, (idx), ASCII_STX);                      \
    WRITE_BYTE((txb)->buffer, (idx), ADD_OFFSET( (id)));              \
    ++ (idx); /* reserved for pktlen */                               \
  } while (0)

#define FRAME_END(txb, idx)                                             \
  do {                                                                  \
    unionWord_t crc;                                                    \
                                                                        \
    /* fix pkt len */                                                   \
    (txb)->buffer [ FRAME_LENGTH_BYTE_POS ] =                           \
      ADD_OFFSET( FRAME_END_OVERHEAD + (idx));                          \
    (txb)->length = ( FRAME_END_OVERHEAD + (idx));                      \
                                                                        \
    /* crc16, sent one nibble at the time, w/ offset, big-endian */     \
    crc.uword = CRCarea((txb)->buffer, (idx), NULL);                    \
    WRITE_BYTE((txb)->buffer, (idx), ADD_OFFSET( MSN( crc.byte[1])));   \
    WRITE_BYTE((txb)->buffer, (idx), ADD_OFFSET( LSN( crc.byte[1])));   \
    WRITE_BYTE((txb)->buffer, (idx), ADD_OFFSET( MSN( crc.byte[0])));   \
    WRITE_BYTE((txb)->buffer, (idx), ADD_OFFSET( LSN( crc.byte[0])));   \
                                                                        \
    /* ETX = frame end */                                               \
    WRITE_BYTE((txb)->buffer, (idx), ASCII_ETX);                        \
  } while (0)


/**
 * -- Typedefs  -----------------------------------------------------------------
 */
enum {
  // machine presence, sent when no other activity is required 
  CONTROLLO_PRESENZA_MACCHINA = 100,

  // state transitioning actions
  RESET_MACCHINA = 101,
  STANDBY_MACCHINA = 102,
  ALLARME_MACCHINA = 103,

  // paint dispensation 
  DISPENSAZIONE_COLORE_MACCHINA = 104,

  // Collect sw version info for all components
  MACHINE_INFO = 105,

  // enter DIAGNOSTICS mode
  DIAGNOSTICA_MACCHINA = 106,

  // terminates immediately any running process 
  ABORT = 107,
  
  // setup commands 
  PAR_CURVA_CALIBRAZIONE_MACCHINA = 108,
  PAR_CIRCUITO_COLORANTE_MACCHINA = 109,
  PAR_SLAVES_CONFIGURATION = 110,
  CAN_LIFTER_MOVEMENT=111,
  ROTATING_TABLE_FIND_CIRCUITS_POSITION = 112,
  FW_VERSIONS = 113,
  UPDATE_TINTING_TABLE_SETTINGS = 114,
  RESET_CAN_LIFTER = 115,
  UPDATE_TINTING_PUMP_SETTINGS = 116,
  START_FORMULA = 117,    
  READ_SLAVES_CONFIGURATION = 118,
  BOOT_VERSIONS = 119, 
  CAN_MOVEMENT  = 120,
  
  // diag commands  
  DIAG_POS_STAZIONE_PRELIEVO = 150,
  DIAG_POS_STAZIONE_RIEMPIMENTO = 151,
  DIAG_POS_STAZIONE_TAPPATURA = 152,
  DIAG_POS_STAZIONE_SCARICO = 153,
  DIAG_TAPPATURA_COPERCHIO = 154,
  DIAG_POS_TAPPATURA_COPERCHIO = 155,
  DIAG_PRELIEVO_CONTENITORE = 156,
  DIAG_SCARICO_CONTENITORE = 157,
  DIAG_ATTIVA_AGITAZIONE_CIRCUITI =158,
  DIAG_ATTIVA_RICIRCOLO_CIRCUITI  = 159,
  DIAG_ATTIVA_AGITAZIONE_RICIRCOLO_CIRCUITI = 160,
  DIAG_IMPOSTA_DISPENSAZIONE_CIRCUITI  = 161,
  DIAG_IMPOSTA_RICIRCOLO_CIRCUITI = 162,
  DIAG_ATTIVA_DISPENSAZIONE_CIRCUITI = 163,
  DIAG_MOVIMENTAZIONE_ASSE_START = 164,
  DIAG_MOVIMENTAZIONE_ASSE_STOP = 165,
  DIAG_POS_HOME_DA_SCARICO = 166,
  DIAG_POS_SEARCH_HOMING = 167,
  DIAG_CALIB_SINGLE_MOVEMENT = 168,
  DIAG_ATTIVA_RESET_CIRCUITI = 169,
  DIAG_MOVIMENTAZIONE_AUTOCAP = 170,
  DIAG_DISPENSATION_VOL = 171,
  DIAG_READ_LAST_DISPENSATION_PARAM = 172,
  DIAG_ATTIVA_EV_DISPENSAZIONE = 173,
  IMPOSTA_CAN_LIFTER=174,
  DIAG_RESET_EEPROM=175,
  DIAG_MOVE_CAN_LIFTER=176,
  DIAG_EN_DEBOUNCE_MICRO_RESERVE = 177,
  DIAG_SET_HUMIDIFIER_PERIPHERALS = 178,
  DIAG_SETUP_HUMIDIFIER_TEMPERATURE_PROCESSES = 179,
  DIAG_SETUP_HUMIDIFIER_10_PARAMETERS = 180,
  DIAG_EEPROM_MANAGEMENT = 181,
  DIAG_SETUP_PUMP_TYPE = 182,
  DIAG_SETUP_TIMERS = 183,
  DIAG_JUMP_TO_BOOT = 184,
  DIAG_ROTATING_TABLE_POSITIONING = 185,
  DIAG_ROTATING_TABLE_SEARCH_POSITION_REFERENCE = 186,
  UPDATE_TINTING_CLEANING_SETTINGS = 187,
  DIAG_COLORANT_ACTIVATION_CLEANING  = 188,
  DIAG_SET_TINTING_PERIPHERALS = 189,
  DIAG_ROTATING_TABLE_TEST = 190,
  DIAG_ROTATING_TABLE_STEPS_POSITIONING = 191,
  DIAG_SETUP_SENSORS = 192,  
  DIAG_AUTOTEST_SETTINGS = 193,
  DIAG_SETUP_MIXER_PARAMETERS = 194,
  DIAG_SET_MIXER_PERIPHERALS = 195,
  DIAG_DESK_LAB_CT30_AUTOTEST_SETTINGS = 196,  
  
  // STATUS request
  STATO_MACCHINA = 200,
  
  // DEBUG only
  DIAG_DATA_LAST_DISPENSATION_PARAM = 201,
  MACHINE_FW_VERSIONS = 213,
  MACHINE_BOOT_VERSIONS = 214,  
  GET_SLAVES_CONFIGURATION = 215,  
}; // enum 

typedef struct
{
  unsigned char numRetry;
  unsigned char procMsg;
} serialGUI_t;

extern serialGUI_t serialGUI;

/**
 * -- Macro-llike functions  ----------------------------------------------------
 */
#define isNewProcessingMsg()                                            \
  (serialGUI.procMsg)

#define resetNewProcessingMsg()                                         \
  (serialGUI.procMsg = FALSE)

#define setNewProcessingMsg()                                           \
  (serialGUI.procMsg = TRUE)

#define isGUIStatusCmd()                                                \
  (procGUI.typeMessage == CONTROLLO_PRESENZA_MACCHINA &&                \
   isNewProcessingMsg())

#define isGUIInfoCmd()                                                  \
  (procGUI.typeMessage == MACHINE_INFO &&                               \
   isNewProcessingMsg())
   
#define isGUIStandbyCmd()                                               \
  (procGUI.typeMessage == STANDBY_MACCHINA &&                           \
   isNewProcessingMsg())

#define isGUISupplyCmd()                                                \
  (procGUI.typeMessage == DISPENSAZIONE_COLORE_MACCHINA &&              \
   isNewProcessingMsg())

#define isGUIResetCmd()                                                 \
  (procGUI.typeMessage == RESET_MACCHINA &&                             \
   isNewProcessingMsg())

#define isGUIDiagnosticCmd()                                            \
  (procGUI.typeMessage == DIAGNOSTICA_MACCHINA &&                       \
   isNewProcessingMsg())

#define isGUIParamUploadCmd()                                           \
  (procGUI.typeMessage == PAR_CURVA_CALIBRAZIONE_MACCHINA ||            \
   procGUI.typeMessage == PAR_CIRCUITO_COLORANTE_MACCHINA)

#define isGUICalibCurveUploadCmd()                                      \
  (procGUI.typeMessage == PAR_CURVA_CALIBRAZIONE_MACCHINA &&            \
   isNewProcessingMsg())

#define isGUIColorCircuitUploadCmd()                                    \
  (procGUI.typeMessage == PAR_CIRCUITO_COLORANTE_MACCHINA &&            \
   isNewProcessingMsg())

#define isGUIAbortCmd() 		                                        \
  (procGUI.typeMessage == ABORT &&                       				\
   isNewProcessingMsg())
   
#define isGUIFWVersionsCmd()                                            \
  (procGUI.typeMessage == MACHINE_FW_VERSIONS &&                        \
   isNewProcessingMsg())

#define isGUIBOOTVersionsCmd()                                          \
  (procGUI.typeMessage == MACHINE_BOOT_VERSIONS &&                      \
   isNewProcessingMsg())

#define isGUIGetSlavesCfgCmd()                                          \
  (procGUI.typeMessage == GET_SLAVES_CONFIGURATION &&                   \
   isNewProcessingMsg())

/**
 * -- Exported functions  -------------------------------------------------------
 */
extern void initSerialCom_GUI(void);

/**
 * -- Sequencers ----------------------------------------------------------------
 */
extern void serialCommManager_GUI(void);
extern void set_slave_status(unsigned short index,unsigned char value);
extern void clear_slave_comm(void);
extern void U2TX_InterruptHandler(void);
extern void U2RX_InterruptHandler(void);

#endif /* _SERIAL_COM_GUI_H_ */
