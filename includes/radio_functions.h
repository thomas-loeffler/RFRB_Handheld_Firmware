
//////////////////////////////////////
//             INCLUDES             //
//////////////////////////////////////

#include <stdint.h>


//////////////////////////////////////
//              DEFINES             //
//////////////////////////////////////

// Maybe put these in a separate header?
// ======== CORE CONTROL ========
#define REG_FIFO            0x00  // FIFO read/write
#define REG_OPMODE          0x01  // Sleep / standby / TX / RX
#define REG_DATAMODUL       0x02  // Packet mode, FSK, shaping


// === DATA RATE / MODULATION ===
#define REG_BITRATEMSB      0x03
#define REG_BITRATELSB      0x04
#define REG_FDEVMSB         0x05
#define REG_FDEVLSB         0x06


// ======== RF FREQUENCY ========
#define REG_FRFMSB          0x07
#define REG_FRFMID          0x08
#define REG_FRFLSB          0x09


// ====== POWER / RECEIVER ======
#define REG_PALEVEL         0x11  // TX power
#define REG_LNA             0x18  // RX gain
#define REG_RXBW            0x19  // RX bandwidth


// ==== INTERRUPTS / STATUS ====
#define REG_DIOMAPPING1     0x25  // Map IRQ to DIO0
#define REG_IRQFLAGS1       0x27  // Mode ready etc
#define REG_IRQFLAGS2       0x28  // Packet sent / payload ready


// ======= PACKET ENGINE =======
#define REG_PREAMBLEMSB     0x2C
#define REG_PREAMBLELSB     0x2D

#define REG_SYNCCONFIG      0x2E
#define REG_SYNCVALUE1      0x2F   // usually only 1–2 used

#define REG_PACKETCONFIG1   0x37
#define REG_PAYLOADLENGTH   0x38
#define REG_FIFOTHRESH      0x3C
#define REG_PACKETCONFIG2   0x3D


// ======== SANITY CHECK ========
#define REG_VERSION         0x10  // should read 0x24




//////////////////////////////////////
//       FUNCTION PROTOTYPES        //
//////////////////////////////////////
