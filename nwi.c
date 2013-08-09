#include <math.h>
#include <stdlib.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
//#include <compat/twi.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#include "nwi.h"
#include "motion_control.h"
#include "config.h"
#include "protocol.h"
#include "serial.h"

static volatile uint8_t nwi_state;
static volatile uint8_t nwi_slarw;
static volatile uint8_t nwi_sendStop;			// should the transaction end with a stop
static volatile uint8_t nwi_inRepStart;			// in the middle of a repeated start

static uint8_t nwi_txBuffer[NWI_BUFFER_LENGTH];
static volatile uint8_t nwi_txBufferIndex;
static volatile uint8_t nwi_txBufferLength;

static uint8_t nwi_rxBuffer[NWI_BUFFER_LENGTH];
static volatile uint8_t nwi_rxBufferHead;
static volatile uint8_t nwi_rxBufferIndex;

static volatile uint8_t nwi_error;
uint8_t address = 4;

/* 
 * Function twi_init
 * Desc     readys twi pins and sets twi bitrate
 * Input    none
 * Output   none
 */
void nwi_init(void)
{
  // initialize state
  nwi_state = NWI_READY;
  //nwi_sendStop = true;		// default value
  //nwi_inRepStart = false;
  
  nwi_txBufferLength = 0;
  nwi_txBufferIndex = 0;

  nwi_rxBufferHead = 0;
  nwi_rxBufferIndex = 0;

  // activate internal pullups for twi.
  sbi(PORTC, 4);//digitalWrite(SDA, 1);
  sbi(PORTC, 5);//digitalWrite(SCL, 1);

  // initialize twi prescaler and bit rate
  cbi(TWSR, TWPS0);
  cbi(TWSR, TWPS1);
  TWBR = ((F_CPU / NWI_FREQ) - 16) / 2;

  /* twi bit rate formula from atmega128 manual pg 204
  SCL Frequency = CPU Clock Frequency / (16 + (2 * TWBR))
  note: TWBR should be 10 or higher for master mode
  It is 72 for a 16mhz Wiring board with 100kHz TWI */

  // enable twi module, acks, and twi interrupt
  TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA);

  // set address to 4
  TWAR = address << 1;
}

uint8_t nwi_read()
{
	if(nwi_rxBufferHead == nwi_rxBufferIndex){
		return NWI_NO_DATA;
	} else {
		uint8_t data = nwi_rxBuffer[nwi_rxBufferIndex];
		nwi_rxBufferIndex++;
		if(nwi_rxBufferIndex == NWI_BUFFER_LENGTH){ nwi_rxBufferIndex = 0; }

		return data;
	}
}

void nwi_reset_read_buffer()
{
	nwi_rxBufferIndex = nwi_rxBufferHead;
}


/* 
 * Function twi_reply
 * Desc     sends byte or readys receive line
 * Input    ack: byte indicating to ack or to nack
 * Output   none
 */
void nwi_reply(uint8_t ack)
{
  // transmit master read ready signal, with or without ack
  if(ack){
    TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA);
  }else{
	  TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
  }
}

/* 
 * Function twi_stop
 * Desc     relinquishes bus master status
 * Input    none
 * Output   none
 */
void nwi_stop(void)
{
  // send stop condition
  TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTO);

  // wait for stop condition to be exectued on bus
  // TWINT is not set after a stop condition!
  while(TWCR & _BV(TWSTO)){
    continue;
  }

  // update twi state
  nwi_state = NWI_READY;
}

/* 
 * Function twi_releaseBus
 * Desc     releases bus control
 * Input    none
 * Output   none
 */
void nwi_releaseBus(void)
{
  // release bus
  TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT);

  // update twi state
  nwi_state = NWI_READY;
}


ISR(TWI_vect)
{
	uint8_t ndata = TWDR;
	uint8_t next_head;
	switch(NW_STATUS){
		// All Master
		case NW_START:		// sent start condition
		case NW_REP_START:	// sent repeated start condition
			// copy device address and r/w/ bit to output register
			TWDR = nwi_slarw;
			nwi_reply(1);
			break;

		case NW_SR_SLA_ACK:    // addressed, returned ack
		case NW_SR_GCALL_ACK: // addressed generally, returned ack
		case NW_SR_ARB_LOST_SLA_ACK:	// lost arbitration, returned ack
		case NW_SR_ARB_LOST_GCALL_ACK:	// lost arbitration, returned ack
			// enter slave receive mode
			nwi_state = NWI_SRX;
			// indicate that rx buffer can be overwritten and ack
			//nwi_rxBufferIndex = 0;
			nwi_reply(1);
			break;
		case NW_SR_DATA_ACK:		// data received, returned ack
		case NW_SR_GCALL_DATA_ACK:	// data received generally, returned ack
			//if there is still room in the rx buffer

			switch(ndata){
				case CMD_STATUS_REPORT: sys.execute |= EXEC_STATUS_REPORT; break; // Set as true
				case CMD_CYCLE_START:   sys.execute |= EXEC_CYCLE_START; break; // Set as true
				case CMD_FEED_HOLD:     sys.execute |= EXEC_FEED_HOLD; break; // Set as true
				case CMD_RESET:         mc_reset(); break; // Call motion control reset routine.
				default: // Write character to buffer
					next_head = nwi_rxBufferHead + 1;
					if(next_head == NWI_BUFFER_LENGTH) { next_head = 0; nwi_reply(0); }// nack

					// Write data to buffer unless it is full.
					if(next_head != nwi_rxBufferIndex) {
						nwi_rxBuffer[nwi_rxBufferHead] = ndata;
						nwi_rxBufferHead = next_head;
						nwi_reply(1); //ack
					}
			}
			break;
		case NW_SR_STOP:	// stop or repeated start condition received
			// put a null char after data if there's room
			if(nwi_rxBufferHead < NWI_BUFFER_LENGTH){
				nwi_rxBuffer[nwi_rxBufferIndex] = '\0';
			}
			// sends ack and stops interface for clock stretching
			nwi_stop();
			//nwi_rxBufferIndex = 0;
			nwi_releaseBus();
			break;
		case NW_SR_DATA_NACK:	// data received, returned nack
		case NW_SR_GCALL_DATA_NACK:	// data received generally, returned nack
			// nack back at master
			nwi_reply(0);
			break;
		
			// slave transmitter
		case NW_ST_SLA_ACK: // addressed, returned ack
		case NW_ST_ARB_LOST_SLA_ACK:	// arbitration lost, returned ack
			// enter slave transmitter mode
			nwi_state = NWI_STX;
			// ready the tx buffer index for iteration
			nwi_txBufferIndex = 0;
			// set tx buffer length to be zero, to verify if user changes it
			nwi_txBufferLength = 0;
			// request for txBuffer to be filled and length to be set
			// note: user must call twi_transmit(bytes, length) to do this
			if(0 == nwi_txBufferLength){
				nwi_txBufferLength = 1;
				nwi_txBuffer[0] = 0x00;
			}
			// transmit first byte from buffer, ...fall?
		case NW_ST_DATA_ACK: // byte sent, ack returned
			// copy data to output register
			TWDR = nwi_txBuffer[nwi_txBufferIndex++];
			// if there is more to send, ack, otherwise nack
			if(nwi_txBufferIndex < nwi_txBufferLength){
				nwi_reply(1);
			}else{
				nwi_reply(0);
			}
			break;
		case NW_ST_DATA_NACK:	// received nack, we are done
		case NW_ST_LAST_DATA:	// received ack, but we are done already!
			// ack future responses
			nwi_reply(1);
			// leave slave receiver state
			nwi_state = NWI_READY;
			break;

			// All
		case NW_NO_INFO:	// no state information
			break;
		case NW_BUS_ERROR:	// bus error, illegal stop/start
			nwi_error = NW_BUS_ERROR;
			nwi_stop();
			break;
	}
}

uint8_t nwi_transmit(const uint8_t* data, uint8_t length)
{
	uint8_t i;

	// ensure data will fit into buffer
	if(NWI_BUFFER_LENGTH < length){
		return 1;
	}

	// ensure we are currently a slave transmitter
	if(NWI_STX != nwi_state){
		return 2;
	}

	// set length and copy data into tx buffer
	nwi_txBufferLength = length;
	for(i = 0; i < length; ++i){
		nwi_txBuffer[i] = data[i];
	}

	return 0;

}