
#include <stdint.h>


//////////////////////////////////////
//              DEFINES             //
//////////////////////////////////////


// --------- FIFO Data Register ---------
#define REG_FIFO            0x00  // FIFO read/write


// ------- Operating Mode Register -------
#define REG_OPMODE          0x01  // Sleep / standby / TX / RX
// Bit 7: SequencerOff - 0 = hardware sequencer runs automatically (handles PLL lock, mode transitions)
//                       1 = manual control of all timing, not recommended
// Bit 6: ListenOn     - 0 = normal operation
//                       1 = low power periodic listen mode, radio sleeps and wakes on a timer
// Bit 5: ListenAbort  - write 1 simultaneously with ListenOn=0 to force exit from listen mode
//                       always reads back as 0, write only trigger
// Bits 4-2: Mode      - 000 = sleep, 001 = standby, 010 = frequency synthesizer
//                       011 = transmit, 100 = receive
// Bits 1-0: reserved, always 0

// For our modes, the sequencer will always run automatically (SequencerOff=0), and we won't be using listen mode (ListenOn=0 and ListenAbort=0)
#define MODE_SLEEP          0x00    // Almost everything off, 0.1uA draw, retains register values
#define MODE_STANDBY        0x04    // Default state, use between packets and during configuration
#define MODE_FREQUENCYSYNTH 0x08    // Frequency synthesizer running, neither TX nor RX active. Used for fast frequency hopping, not needed for this application
#define MODE_TRANSMIT       0x0C    // Transmitting, switch to this mode when sending a packet
#define MODE_RECEIVE        0x10    // Receiving, switch to this mode when listening for packets

// ------- Data Modulation Register -------
#define REG_DATAMODUL       0x02  
// Bits 7: unused, always 0
// Bits 6-5: DataMode          - 00 = packet mode (recommended, hardware handles framing)
//                               10 = continuous mode with bit synchronizer
//                               11 = continuous mode without bit synchronizer
// Bits 4-3: ModulationType    - 00 = FSK (recommended, robust in noisy environments)
//                               01 = OOK (on-off keying, simpler but less noise resistant)
// Bits 2: unused, always 0
// Bits 1-0: ModulationShaping - 00 = no shaping (fine for most applications) (about smoothing frequency transitions, sinse we will be in fsk, but not important for us)
//                               01 = Gaussian BT=1.0 (GFSK, smoother transitions)
//                               10 = Gaussian BT=0.5 (GFSK, tighter spectrum)
//                               11 = Gaussian BT=0.3 (GFSK, tightest spectrum)


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

// Sync Coniguration Register - determining the settings of the sync word
#define REG_SYNC_CONFIG     0x2E
// Bit 7:    SyncOn          - 1 = enable sync word detection
// Bit 6:    FifoFillCond    - 0 = fill FIFO when sync word matched, 1 = always fill
// Bits 5-3: SyncSize        - sync word size in bytes, value = (bytes + 1), e.g. 001 = 2 bytes. 2 bytes is recommended, can do more if in a crowded 915MHz environment but it adds overhead
// Bits 2-0: SyncTol         - number of bit errors allowed in sync word (0-7, recommend 0 or 1)

// Sync Word Registers - registers that hold the sync word bytes
#define REG_SYNCVALUE1      0x2F 
#define REG_SYNCVALUE2      0x30
#define REG_SYNCVALUE3      0x31
#define REG_SYNCVALUE4      0x32

#define REG_PACKETCONFIG1   0x37
#define REG_PAYLOADLENGTH   0x38
#define REG_FIFOTHRESH      0x3C
#define REG_PACKETCONFIG2   0x3D


// ======== SANITY CHECK ========
#define REG_VERSION         0x10  // should read 0x24
