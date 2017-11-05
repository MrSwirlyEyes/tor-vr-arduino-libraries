#include "Radio.h"

/********************* INTERRUPTS AND STATIC RADIO VARIABLE ***********************/



// Required global ISR parameter
volatile static Radio * radio = NULL;



// This interrupt is called when radio TX is complete. We'll just
// use it to turn off our TX LED.
ISR(TRX24_TX_END_vect) {
    digitalWrite(TX_LED, LOW);
}



// This interrupt is called the moment data is received by the radio.
// We'll use it to gather information about RSSI -- signal strength --
// and we'll turn on the RX LED.
ISR(TRX24_RX_START_vect) {
    digitalWrite(RX_LED, HIGH);  // Turn receive LED on
    radio->rssiRaw = PHY_RSSI;  // Read in the received signal strength
}



// This interrupt is called at the end of data receipt. Here we'll gather
// up the data received. And store it into a global variable. We'll
// also turn off the RX LED.
ISR(TRX24_RX_END_vect) {
    uint8_t length;
    // Maximum transmission is 128 bytes
    uint8_t tempFrame[RF_BUFFER_SIZE];

    // The received signal must be above a certain threshold.
    if (PHY_RSSI & (1<<RX_CRC_VALID)) {
        // The length of the message will be the first byte received.
        length = TST_RX_LENGTH;
        // The remaining bytes will be our received data.
        memcpy(&tempFrame[0], (void*)&TRXFBST, length);

        // Now we need to collect the frame into our receive buffer.
        //  k will be used to make sure we don't go above the length
        //  i will make sure we don't overflow our buffer.
        unsigned int k = 0;
        unsigned int i = (radio->radioRXBuffer.head + 1) % RF_BUFFER_SIZE; // Read buffer head pos and increment;
        while ((i != radio->radioRXBuffer.tail) && (k < length-2)) {
            // First, we update the buffer with the first byte in the frame
            radio->radioRXBuffer.buffer[radio->radioRXBuffer.head] = tempFrame[k++];
            radio->radioRXBuffer.head = i; // Update the head
            i = (i + 1) % RF_BUFFER_SIZE; // Increment i % BUFFER_SIZE
        }
    }
    digitalWrite(RX_LED, LOW);  // Turn receive LED off, and we're out
}



/****************************** RADIO CLASS METHODS ***************************/



// Constructor for radio object
Radio::Radio(int bChannel):channel(bChannel) {
    // If channel is not within range (11-26)
    if ((channel < 11) || (channel > 26))
        channel = 11;

    /* Since the ISR depends on a global variable, this makes sure that the
     * variable points to something that exists when the radio object is created.
     */
    radio=this;

    rfFlush();
    rfBegin();
}



// Flushes the buffer
void Radio::rfFlush() {
    for (int i = 0; i < RF_BUFFER_SIZE; i++)
        radioRXBuffer.buffer[i] = 0;
    
    radioRXBuffer.head = radioRXBuffer.tail = 0;
}



// Initialize the RFA1's low-power 2.4GHz transciever.
// Sets up the state machine, and gets the radio into
// the RX_ON state. Interrupts are enabled for RX
// begin and end, as well as TX end.
uint8_t Radio::rfBegin() {
    // Setup RX/TX LEDs: These are pins B6/34 (RX) and B7/35 (TX).
    pinMode(RX_LED, OUTPUT);
    digitalWrite(RX_LED, LOW);
    pinMode(TX_LED, OUTPUT);
    digitalWrite(TX_LED, LOW);

    // Transceiver Pin Register -- TRXPR.
    // This register can be used to reset the transceiver, without
    // resetting the MCU.
    TRXPR |= (1<<TRXRST);   // TRXRST = 1 (Reset state, resets all registers)

    // Transceiver Interrupt Enable Mask - IRQ_MASK
    // This register disables/enables individual radio interrupts.
    // First, we'll disable IRQ and clear any pending IRQ's
    IRQ_MASK = 0;  // Disable all IRQs

    // Transceiver State Control Register -- TRX_STATE
    // This regiseter controls the states of the radio.
    // First, we'll set it to the TRX_OFF state.
    TRX_STATE = (TRX_STATE & 0xE0) | TRX_OFF;  // Set to TRX_OFF state
    for(volatile int i=0; i<100; i++)  // Creates a small delay for state change
        i+=1;

    // Transceiver Status Register -- TRX_STATUS
    // This read-only register contains the present state of the radio transceiver.
    // After telling it to go to the TRX_OFF state, we'll make sure it's actually
    // there.
    if ((TRX_STATUS & 0x1F) != TRX_OFF) { // Check to make sure state is correct
        return 0;  // Error, TRX isn't off
    }

    // Transceiver Control Register 1 - TRX_CTRL_1
    // We'll use this register to turn on automatic CRC calculations.
    TRX_CTRL_1 |= (1<<TX_AUTO_CRC_ON);  // Enable automatic CRC calc. 

    // Enable RX start/end and TX end interrupts
    IRQ_MASK = (1<<RX_START_EN) | (1<<RX_END_EN) | (1<<TX_END_EN);

    // Transceiver Clear Channel Assessment (CCA) -- PHY_CC_CCA
    // This register is used to set the channel. CCA_MODE should default
    // to Energy Above Threshold Mode.
    // Channel should be between 11 and 26 (2405 MHz to 2480 MHz) 
    PHY_CC_CCA = (PHY_CC_CCA & 0xE0) | channel; // Set the channel

    // Finally, we'll enter into the RX_ON state. Now waiting for radio RX's, unless
    // we go into a transmitting state.
    TRX_STATE = (TRX_STATE & 0xE0) | RX_ON; // Default to receiver

    return 1;
}



// This function sends a string of characters out of the radio.
// Given a string, it'll format a frame, and send it out.
void Radio::rfPrint(String toPrint) {
    uint8_t frame[127];  // We'll need to turn the string into an arry
    int length = toPrint.length();  // Get the length of the string
    for (int i=0; i<length; i++)  // Fill our array with bytes in the string 
        frame[i] = toPrint.charAt(i);


    // Transceiver State Control Register -- TRX_STATE
    // This regiseter controls the states of the radio.
    // Set to the PLL_ON state - this state begins the TX.
    TRX_STATE = (TRX_STATE & 0xE0) | PLL_ON;  // Set to TX start state

    while(!(TRX_STATUS & PLL_ON));  // Wait for PLL to lock

    digitalWrite(TX_LED, HIGH);

    // Start of frame buffer - TRXFBST
    // This is the first byte of the 128 byte frame. It should contain
    // the length of the transmission.
    TRXFBST = length + 2;
    memcpy((void *)(&TRXFBST+1), frame, length);
    // Transceiver Pin Register -- TRXPR.
    // From the PLL_ON state, setting SLPTR high will initiate the TX.
    TRXPR |= (1<<SLPTR);   // SLPTR high
    TRXPR &= ~(1<<SLPTR);  // SLPTR low

    // After sending the byte, set the radio back into the RX waiting state.
    TRX_STATE = (TRX_STATE & 0xE0) | RX_ON;
}



// This function will transmit a single byte out of the radio.
void Radio::rfWrite(uint8_t b) {
    uint8_t length = 3;

    // Transceiver State Control Register -- TRX_STATE
    // This regiseter controls the states of the radio.
    // Set to the PLL_ON state - this state begins the TX.
    TRX_STATE = (TRX_STATE & 0xE0) | PLL_ON;  // Set to TX start state

    while(!(TRX_STATUS & PLL_ON));  // Wait for PLL to lock

    digitalWrite(TX_LED, HIGH);  // Turn on TX LED

    // Start of frame buffer - TRXFBST
    // This is the first byte of the 128 byte frame. It should contain
    // the length of the transmission.
    TRXFBST = length;
    // Now copy the byte-to-send into the address directly after TRXFBST.
    memcpy((void *)(&TRXFBST+1), &b, 1);

    // Transceiver Pin Register -- TRXPR.
    // From the PLL_ON state, setting SLPTR high will initiate the TX.
    TRXPR |= (1<<SLPTR);   // SLPTR = 1
    TRXPR &= ~(1<<SLPTR);  // SLPTR = 0  // Then bring it back low

    // After sending the byte, set the radio back into the RX waiting state.
    TRX_STATE = (TRX_STATE & 0xE0) | RX_ON;
}



/** 
 *  Description: This function will transmit a packet of bytes out of the radio
 *  Arguments:
 *  b - pointer to the location in memory that we want to send over the radio
 *  length - the length of the buffer. The datasheet suggests that the
 *           radio can send packets of up-to 127 bytes long. In practice, it
 *           only sends approximately 125 bytes successfully.
 */
void Radio::rfWrite(uint8_t * b, uint8_t length) {
    // Transceiver State Control Register -- TRX_STATE
    // This regiseter controls the states of the radio.
    // Set to the PLL_ON state - this state begins the TX.
    TRX_STATE = (TRX_STATE & 0xE0) | PLL_ON;  // Set to TX start state

    while(!(TRX_STATUS & PLL_ON));  // Wait for PLL to lock

    digitalWrite(TX_LED, HIGH);  // Turn on TX LED

    // Start of frame buffer - TRXFBST
    // This is the first byte of the 128 byte frame. It should contain
    // the length of the transmission.
    TRXFBST = length + 2;
    // Now copy the byte-to-send into the address directly after TRXFBST.
    memcpy((void *)(&TRXFBST+1), b, length);

    // Transceiver Pin Register -- TRXPR.
    // From the PLL_ON state, setting SLPTR high will initiate the TX.
    TRXPR |= (1<<SLPTR);   // SLPTR = 1
    TRXPR &= ~(1<<SLPTR);  // SLPTR = 0  // Then bring it back low

    // After sending the byte, set the radio back into the RX waiting state.
    TRX_STATE = (TRX_STATE & 0xE0) | RX_ON;
}



// Returns how many unread bytes remain in the radio RX buffer.
// 0 means the buffer is empty. Maxes out at RF_BUFFER_SIZE.
unsigned int Radio::rfAvailable() {
    return (unsigned int)(RF_BUFFER_SIZE + radioRXBuffer.head - radioRXBuffer.tail) % RF_BUFFER_SIZE;
}



// This function reads the oldest data in the radio RX buffer.
// If the buffer is emtpy, it'll return a 255.
char Radio::rfRead() {
    if (radioRXBuffer.head == radioRXBuffer.tail)
        return -1;
    else {
        // Read from the buffer tail, and update the tail pointer.
        char c = radioRXBuffer.buffer[radioRXBuffer.tail];
        radioRXBuffer.tail = (unsigned int)(radioRXBuffer.tail + 1) % RF_BUFFER_SIZE;
        return c;
    }
}



// Read up to len bytes into buf.  Return the number of bytes read.
char Radio::rfRead(uint8_t * buf, uint8_t len) {
    uint8_t count = 0;
    while (count <= len) {
        if (radioRXBuffer.head == radioRXBuffer.tail)
            break;
        else {
            // Read from the buffer tail, and update the tail pointer.
            char c = radioRXBuffer.buffer[radioRXBuffer.tail];
            radioRXBuffer.tail = (unsigned int)(radioRXBuffer.tail + 1) % RF_BUFFER_SIZE;
            buf[count++] = c;
        }
    }
    return count;
}