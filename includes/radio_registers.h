
#include <stdint.h>


//////////////////////////////////////
//              DEFINES             //
//////////////////////////////////////


// ---------- FIFO Data Register ----------
#define REG_FIFO            0x00  // FIFO read/write




// ---------- Operating Mode Register ----------
#define REG_OPMODE          0x01 
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
#define MODE_TX             0x0C    // Transmitting, switch to this mode when sending a packet
#define MODE_RX             0x10    // Receiving, switch to this mode when listening for packets




// ---------- Data Modulation Register ----------
#define REG_DATAMODUL       0x02  
// Bit 7: unused, always 0
// Bits 6-5: DataMode          - 00 = packet mode (recommended, hardware handles framing)
//                               10 = continuous mode with bit synchronizer
//                               11 = continuous mode without bit synchronizer
// Bits 4-3: ModulationType    - 00 = FSK (recommended, robust in noisy environments)
//                               01 = OOK (on-off keying, simpler but less noise resistant)
// Bit 2: unused, always 0
// Bits 1-0: ModulationShaping - 00 = no shaping (fine for most applications) (about smoothing frequency transitions, since we will be in fsk, but not important for us)
//                               01 = Gaussian BT=1.0 (GFSK, smoother transitions)
//                               10 = Gaussian BT=0.5 (GFSK, tighter spectrum)
//                               11 = Gaussian BT=0.3 (GFSK, tightest spectrum)
#define MY_DATAMODUL 0x00 // Packet mode, FSK, no shaping




// ---------- Bit Rate Registers ----------
#define REG_BITRATEMSB      0x03
#define REG_BITRATELSB      0x04
// Sets the bit rate of the radio, split across two bytes
// Formula: BitRate = F_osc / BR_value
// F_osc = crystal frequency = 32MHz
// MSB 2^8 + LSB 2^6 = 320 -> 32M / 320 = 100,000 bps
#define BR_100kb_MSB        0x01    // 100kbps bitrate MSB value
#define BR_100kb_LSB        0x40    // 100kbps bitrate LSB value
// Higher bitrate = wider RX bandwidth needed = more noise susceptibility
// Lower bitrate  = narrower RX bandwidth = more noise rejection, better range




// ---------- Frequency Deviation Registers ----------
#define REG_FDEVMSB         0x05
#define REG_FDEVLSB         0x06
// Sets the FSK frequency deviation - the distance the carrier frequency shifts
// above and below center to represent a 1 bit and a 0 bit respectively
// Ex. at 915MHz with 50kHz deviation:
//   1 bit = 915.050 MHz
//   0 bit = 914.950 MHz
// F_step = smallest possible frequency step (the RFM69 has a 19-bit frequency synthesizer)
// F_step = F_osc / 2^19 = 32,000,000 / 524,288 = 61.035 Hz
// Formula: Fdev_actual = F_step * Fdev_reg
// Fdev_reg = desired_Fdev / 61.035
//
// Modulation Index = Fdev / (Bitrate / 2)      target 1.0 for best performance
// Deviation MUST be updated if bitrate changes - they are linked:
#define FDEV_50k_MSB         0x03    // 50kHz deviation (819*61.035 ~= 50,000)
#define FDEV_50k_LSB         0x33    
// Too low:  frequencies too close together, noise causes bit errors
// Too high: wastes bandwidth, RX filter must be wider letting in more noise




// ---------- RF FREQUENCY ----------
#define REG_FRFMSB          0x07
#define REG_FRFMID          0x08
#define REG_FRFLSB          0x09
// Sets the carrier frequency - the center frequency the radio transmits and receives on
// F_step = F_osc / 2^19 = 32,000,000 / 524,288 = 61.035 Hz
// Formula: Frf = F_step * Frf_value
// Frf_value = desired_frequency / 61.035
// Split across 3 bytes, frequency only takes effect when LSB is written
// so always write in order MSB -> Mid -> LSB
//   915 MHz: 0xE4, 0xC0, 0x00 = 14,991,360 * 61.035 = 914,997,658 Hz ~ 915 MHz
#define FRF_915_MSB          0xE4
#define FRF_915_MID          0xC0
#define FRF_915_LSB          0x00




// ---------- Version Register ----------
#define REG_VERSION         0x10  // should read 0x24
// Read-only register that contains the version number of the RFM69




// ---------- TX Power Register ----------
#define REG_PALEVEL         0x11
// Controls which power amplifiers are active and the output power level
// The RFM69HCW has 3 power amplifiers across 2 output pins:
//   PA0 → RFIO pin    (NOT connected on Adafruit breakout, unusable)
//   PA1 → PA_BOOST pin (our amplifier)
//   PA2 → PA_BOOST pin (combines with PA1 for higher power, adds mode switching complexity)
//
// Bit 7:    Pa0_On        - 0 = off, RFIO pin not connected on Adafruit breakout
// Bit 6:    Pa1_On        - 1 = on,  PA1 on PA_BOOST pin
// Bit 5:    Pa2_On        - 0 = off, not needed, +13dBm is sufficient for 200ft
// Bits 4-0: OutputPower   - power level 0-31
//                           Pout formula depends on PA combination:
//                           PA1 only:          Pout = -18 + OutputPower (-2  to +13 dBm)
//                           PA1 + PA2:         Pout = -14 + OutputPower (+2  to +17 dBm)
//                           PA1 + PA2 + high:  Pout = -11 + OutputPower (+5  to +20 dBm)
//
// Selected: PA1 only, OutputPower = 31 → -18 + 31 = +13dBm
// +13dBm chosen over +20dBm because:
//   - +20dBm requires toggling test registers 0x5A and 0x5C on every TX/RX switch
//   - forgetting to reset test registers before RX can damage the chip
//   - +20dBm only adds 7dB over +13dBm, negligible at these distances
#define MY_TX_POWER 0x5F // PA1 on, PA2 off, OutputPower = 31 = +13dBm




// ---------- Power Amplifier Ramp Register ----------
#define REG_PARAMP           0x12  // PA ramp up time
// Controls the rise and fall time of the power amplifier when turning on and off
// Slower ramp = cleaner spectrum, less splatter into adjacent channels
// Faster ramp = quicker PA switching, lower latency between packets
// Must be considered alongside InterPacketRxDelay in REG_PACKETCONFIG2
// The RX delay should be >= the ramp time to avoid the receiver mistaking PA ramp-down for a new packet
//
// Bits 7-4: reserved, always 0
// Bits 3-0: PaRamp
// default is 0000 1001 = 0x90 which is 40us ramp time according to datasheet. Seems like no compelling reason to change
#define MY_PARAMP 0x09 // 40us ramp time




// ---------- Overcurrent Protection Register ----------
#define REG_OCP              0x13
// Controls the overcurrent protection (OCP) feature that shuts down the PA if current exceeds a certain limit to prevent damage
// OCP is important to protect the radio, especially if you accidentally create a short circuit with your antenna or try to use too high of a power setting
// Bits 7-5:    reserved, always 0
// Bit 4:       OcpOn          enables overcurrent protection when set to 1, recommended to keep on
// Bits 3-0:    OcpTrim        OCP current limit setting, value = (45 + 5 * OcpTrim) mA
// OcpTrim default value is 0x1A, 95mA limit. just fine for us.
#define MY_OCP 0x1A // OCP on, 95mA limit




// ---------- RX Power / Low Noise Amplifier Register ----------
#define REG_LNA             0x18  // RX gain
// Controls the Low Noise Amplifier - the first amplifier in the receive chain
// Amplifies incoming antenna signal before any processing occurs
// Called "low noise" because it amplifies weak signals without adding significant noise
//
// Bit 7:    LnaZin          - input impedance matching
//                             0 = 50 ohm  (use this, we have 50ohm antenna)
//                             1 = 200 ohm
// Bit 6: reserved, always 0
// Bits 5-3: LnaCurrentGain  - READ ONLY, current gain set by AGC
//                             shows which gain setting AGC has selected (G1-G6)
// Bits 2-0: LnaGainSelect   - gain control mode
//                             000 = AGC (Automatic Gain Control) on, gain set automatically (recommended)
//                             AGC automatically selects best gain for signal conditions   
//                             AGC is pretty much required with the constant changing distance of the robot and varying signal strength
//                             manually setting gain means radio cant adapt to varying signal strength
#define MY_LNA_RX_POWER 0x00 // 50 ohm input impedance, AGC on




// ---------- RX Bandwidth Register ----------
#define REG_RXBW            0x19  // RX bandwidth
// Controls the receiver channel filter bandwidth
// Bits 7-5: DccFreq - DC cancellation filter cutoff frequency
//           010 = default, leave here unless you have DC offset issues
//
// Bits 4-3: RxBwMant 00 = 16, 01 = 20, 10 = 24, 11 = reserved
//
// Bits 2-0: RxBwExp - exponent of bandwidth formula
//           In FSK, RX bandwidth is:
//           RxBw = F_osc(32MHz) / (RxBwMant * 2^(RxBwExp + 2))
//
// Rule: RxBw >= Fdev + (Bitrate / 2)
// At 100kbps, 50kHz*2 deviation: RxBw >= 100,000 + 50,000 = 150kHz minimum
// RxBwMant = 24 and RxBwExp = 1 gives us 32,000,000 / (24 * 2^(1+2)) =  32M / 24*8 = 166,667 Hz
// rxbw = 010 10 001 = 0x51
// Narrower = less noise but must be wide enough to capture your signal
// Wider    = captures signal easily but lets in more noise
// Must update this register if bitrate or deviation changes

// Also worth knowing — there is a sister register 0x1A — RegAfcBw which sets the bandwidth used during automatic frequency correction at startup. 
// It's recommended to set this slightly wider than RegRxBw, typically 1.5-2x:

#define MY_RXBW 0x51 // Rx bandwidth 167kHz




// ---------- Automatic Frequency Correction Bandwidth Register ----------
#define REG_AFCBW           0x1A
// Bits 7-5: DccFreq - DC cancellation filter cutoff frequency
//           010 = default, leave here unless you have DC offset issues
//
// Bits 4-3: RxBwMant 00 = 16, 01 = 20, 10 = 24, 11 = reserved
//
// Bits 2-0: RxBwExp - exponent of bandwidth formula
//           In FSK, RX bandwidth is:
//           RxBw = F_osc(32MHz) / (RxBwMant * 2^(RxBwExp + 2))
// Must always be wider than RegRxBw - gives AFC routine a larger window
// to find and correct frequency offset between radios at receiver startup
// RxBwMant=20, RxBwExp=1: 32MHz / (20 * 2^(1+2)) = 32MHz / 160 = 200kHz
// my value should be 010 01 001 = 0x49
#define MY_AFCBW 0x49 // AFC bandwidth 200kHz, wider than RxBw for better AFC



#define REG_AFCFEI 0x1E
// AFC = Automatic Frequency Correction - measures AND corrects frequency offset
// FEI = Frequency Error Indicator      - measures and REPORTS offset only, no correction
//
// Bit 7: reserved
// Bit 6: FeiDone        - READ ONLY, 1 = FEI measurement complete and result is ready
//                         poll this before reading RegFeiMsb/Lsb
// Bit 5: FeiStart       - write 1 to trigger a FEI measurement
//                         always reads back 0, one shot trigger. Useful for diagnostics to see how much your crystals are drifting
// Bit 4: AfcDone        - READ ONLY, 1 = AFC measurement complete
//                         you can poll this to confirm AFC finished before data arrives. Not really important since hardware already handles it
// Bit 3: AfcAutoClear   - 1 = AFC register cleared before each new AFC measurement
//                         0 = AFC result from previous packet is kept as starting point 
//                         (leave at default 0, keeping previous result helps in stable links. plus we are using the same to registers so the drift should be consistent)
// Bit 2: AfcAutoOn      - 1 = AFC runs automatically at receiver startup during preamble (definitly want this)
//                         0 = AFC only runs when manually triggered by AfcStart
// Bit 1: AfcClear       - write 1 to manually clear the AFC register
//                         always reads back 0, one shot trigger
//                         (probably wont need since our AFC should be the same most of the time. see note on bit 3)
// Bit 0: AfcStart       - write 1 to manually trigger an AFC measurement
//                         always reads back 0, one shot trigger
//                         (not needed since AFCauto is on)
#define MY_AFCFEI 0x04



// ---------- Digital I/O Mapping Register 1 ----------
#define REG_DIOMAPPING1     0x25 
// Controls DIO0 - DIO3
// Bits 7-6: Dio0 Mapping     - 00 -> CRCOk (in RX mode) and Packet Sent (in TX mode)
//                            - 01 -> Payload Ready (in RX mode) and TXready (in TX mode)
// Bits 5-4: Dio1 Mapping     - 00 not needed but have to map
// Bits 3-2: Dio2 Mapping     - 00 not needed but have to map
// Bits 1-0: Dio3 Mapping     - 00 not needed but have to map
#define MY_DIOMAPPING1_PACKETSENT 0x00
#define MY_DIOMAPPING1_PAYLOADREADY 0x40





// ---------- Digital I/O Mapping Register 1 ----------
#define REG_DIOMAPPING2     0x26
// Controls DIO4, DIO5, and the ClkOut frequency on DIO5
// Bits 7-6: DIO4 mapping     - 00 - not connected 
// Bits 5-4: DIO5 mapping     - 00 - not connected
// Bit 3:    reserved
// Bits 2-0: ClkOut           - clock output frequency available on DIO5 
//                              111 = OFF disable to save power when not needed
//                              datasheet explicitly recommends disabling ClkOut when not in use
#define MY_DIOMAPPING2 0x07


#define REG_IRQFLAGS1       0x27  // Read only flags register (no relevant flags)
#define REG_IRQFLAGS2       0x28  // Read only flags register (packet sent / payload ready)


// ---------- RSSI (Received Signal Strength Indicator) Threshold REgister ----------
#define REG_RSSITHRESH 0x29
// Minimum signal strength required before receiver starts AGC and sync word detection
// Radio sits in WAIT state ignoring everything below this threshold
// Formula: threshold_dBm = -(RegRssiThresh / 2)
// Reset value 0xFF = -127.5 dBm, too close to noise floor, AGC triggers on noise
// Recommended 0xE4 = -114 dBm, just above noise floor
// Set too low: AGC triggers on noise constantly, wastes time, false sync searches
// Set too high: might miss weak legitimate signals at the edge of range
#define MY_RSSITHRESH 0xE4



// ---------- RX Timeout 1 Register ----------
#define REG_RXTIMEOUT1      0x2A
// Timeout duration between entering RX mode and RSSI threshold being detected
// If no signal above RssiThreshold is detected within this time, Timeout interrupt fires
// 0x00 = disabled, radio waits indefinitely for a signal (recommended)
#define MY_RXTIMEOUT1       0x00  // disabled, software timeout used instead




// ---------- RX Timeout 2 Register ----------
#define REG_RXTIMEOUT2      0x2B
// Timeout duration between RSSI threshold being detected and PayloadReady
// If sync word and payload are not received within this time, Timeout interrupt fires
// 0x00 = disabled, radio waits indefinitely for payload after RSSI detect (recommended)
#define MY_RXTIMEOUT2       0x00  // disabled, software timeout used instead




// ---------- Preamble Registers ----------
#define REG_PREAMBLEMSB     0x2C
#define REG_PREAMBLELSB     0x2D
// Sets the number of preamble bytes transmitted before the sync word
// Preamble is an automatically generated alternating 1/0 pattern (0xAA or 0x55)
// Purpose: gives the receiver AGC and clock recovery time to lock onto
//          the signal before the sync word arrives
// Hardware generates it automatically on TX and looks for it on RX
// You only set the length, never touch the actual bytes
//
// Too short: receiver hasn't settled when sync word arrives, packet missed
// Too long:  wastes airtime unnecessarily
// Value is split across two bytes but you'll never need more than 255 preamble bytes so MsByte will always be 0x00
// Default is 3
#define MY_PREAMBLE_MSB 0x00
#define MY_PREAMBLE_LSB 0x08



// ---------- Sync Configuration Register ----------
#define REG_SYNC_CONFIG     0x2E
// Bit 7:    SyncOn          - 1 = enable sync word detection
// Bit 6:    FifoFillCond    - 0 = fill FIFO when sync word matched, 1 = always fill
// Bits 5-3: SyncSize        - sync word size in bytes, value = (bytes + 1), e.g. 011 = 4 bytes. 
//                            2 bytes is recommended, can do more if in a crowded 915MHz environment but it adds overhead
// Bits 2-0: SyncTol         - number of bit errors allowed in sync word (0-7, recommend 0 or 1)
#define MY_SYNC_CONFIG 0x98 // Sync on, only fill fifo when sync mathced, 4 bytes sync word, 0 bit tolerance
#define MY_SYNC_CONFIG2 0x9A // Sync on, only fill fifo when sync mathced, 4 bytes sync word, 2 bit tolerance



// ---------- Sync Word Registers ---------- 
// Registers that hold the sync word bytes
// Our set sync word = 0xCAFEBABE
#define REG_SYNCVALUE1      0x2F 
#define REG_SYNCVALUE2      0x30
#define REG_SYNCVALUE3      0x31
#define REG_SYNCVALUE4      0x32



// ---------- Packet Configuration Register 1 ----------
#define REG_PACKETCONFIG1   0x37
// Controls packet engine behavior
//
// Bit 7:    PacketFormat      - 0 = fixed length packets (hard to remeber to change later)
//                               1 = variable length packets (recommended, more flexible)
// Bits 6-5: DcFree            - 00 = none (recommended for RFM69 with sync word)
//                               01 = Manchester encoding (halves throughput, not needed)
//                               10 = Whitening (scrambles data for DC balance)
// Bit 4:    CrcOn             - 0 = CRC disabled
//                               1 = CRC enabled (recommended, filters corrupt packets)
// Bit 3:    CrcAutoClearOff   - 0 = clear FIFO on CRC fail (recommended, discard bad packets)
//                               1 = keep bad packet in FIFO
// Bits 2-1: AddressFiltering  - 00 = none (recommended, only 2 nodes on network)
//                               01 = match node address
//                               10 = match node or broadcast address
// Bit 0:    reserved, always 0
// reccomended value: 1001 0000 = 0x90 variable length packets, CRC on, discard bad packets, no address filtering
#define MY_PACKETCONFIG1 0x90



// ---------- Payload Length Register ----------
#define REG_PAYLOADLENGTH   0x38
// In variable length mode: sets the MAXIMUM allowed packet length
// Any packet whose length byte exceeds this value is discarded
// In fixed length mode: sets the exact expected packet length
#define MY_PAYLOAD_LENGTH 0x40 // 64 bytes, FIFO is 66 bytes, and one is used as the length byte




// ---------- FIFO Threshold Register ----------
#define REG_FIFOTHRESH      0x3C
// Controls when transmission starts and the FIFO threshold level
//
// Bit 7: TxStartCondition    - what triggers the radio to start transmitting (when in TX mode)
//                              0 = start TX when FIFO threshold is exceeded
//                              1 = start TX when there is at least 1 byte in FIFO (FifoNotEmpty) (do this option, becasue when switching to tx the payload should be ready)
// Bits 6-0: FifoThreshold    - number of bytes that must be in FIFO before FifoLevel
//                              interrupt flag is set, value 0-63
//                              only relevant if TxStartCondition = 0
//                              with TxStartCondition = 1 this value is ignored for TX
//                              still used for RX to trigger FifoLevel interrupt
//                              set to packet size - 1 as a reasonable default
#define MY_FIFOTHRESH 0x80 // Transmit if fifo not empty, threshold = 0




// ---------- Packet Configuration Register 2 ----------
#define REG_PACKETCONFIG2   0x3D
// Additional packet engine controls
//
// Bits 7-4: InterPacketRxDelay - delay between packet received and receiver restart. 
//                                (after you've finished reading the last packet from the fifo(fifo clear), wait this long before hunting for the next one.)
//                                value = 2^(InterPacketRxDelay) / bitrate
//                                value = 2^(InterPacketRxDelay) / 100,000 
//                                0011 = 2^3 / 100,000 = 80us, double PA ramp time to be safe
//                                Must match or exceed the transmitter’s PA(power amplifier) ramp-down time. found in reg 0x12
// Bit 3:    reserved, always 0
// Bit 2:    RestartRx          - write 1 to manually force receiver restart
//                                always reads back 0, one shot trigger
// Bit 1:    AutoRxRestartOn    - 1 = automatically restart receiver after packet received
//                                0 = manual restart required, not recommended
// Bit 0:    AesOn              - 0 = AES encryption off (keep off for now, can add later)
//                                1 = AES encryption on 
// reccomended value: 0011 0010 = 0x32 InterPacketRxDelay 80us, auto RX restart on, AES encryption off
#define MY_PACKETCONFIG2 0x32


// ---------- Digital Automatic Gain Control ----------
#define REG_TESTDAGC        0x6F
// Improves receiver performance in fading conditions by continuously
// adjusting digital gain every 2 bits throughout packet reception
// Works in addition to the analog AGC (RegLna) - fully transparent to your code (you dont need to do anything to it, set it and forget it)
//
// 0x00 = default, DAGC off
// 0x20 = DAGC on, recommended for low modulation index (AfcLowBetaOn=1)
// 0x30 = DAGC on, recommended for normal modulation index (our case)
//
// Datasheet explicitly recommends always enabling DAGC
// Free improvement to RX reliability in a noisy gym environment
// No downside to enabling it
#define MY_TESTDAGC         0x30  // DAGC on, normal modulation index

