
#ifndef nwi_h
#define nwi_h

#include <inttypes.h>

#ifndef NWI_FREQ
#define NWI_FREQ 100000L
#endif

#ifndef NWI_BUFFER_LENGTH
#define NWI_BUFFER_LENGTH 32
#endif

#define NWI_READY 0
#define NWI_MRX   1
#define NWI_MTX   2
#define NWI_SRX   3
#define NWI_STX   4

#define NWI_NO_DATA 0xff



/*@{*/
/* Master */
/** \ingroup util_twi
    \def TW_START
    start condition transmitted */
#define NW_START		0x08

/** \ingroup util_twi
    \def TW_REP_START
    repeated start condition transmitted */
#define NW_REP_START		0x10

/* Master Transmitter */
/** \ingroup util_twi
    \def TW_MT_SLA_ACK
    SLA+W transmitted, ACK received */
#define NW_MT_SLA_ACK		0x18

/** \ingroup util_twi
    \def TW_MT_SLA_NACK
    SLA+W transmitted, NACK received */
#define NW_MT_SLA_NACK		0x20

/** \ingroup util_twi
    \def TW_MT_DATA_ACK
    data transmitted, ACK received */
#define NW_MT_DATA_ACK		0x28

/** \ingroup util_twi
    \def TW_MT_DATA_NACK
    data transmitted, NACK received */
#define NW_MT_DATA_NACK		0x30

/** \ingroup util_twi
    \def TW_MT_ARB_LOST
    arbitration lost in SLA+W or data */
#define NW_MT_ARB_LOST		0x38

/* Master Receiver */
/** \ingroup util_twi
    \def TW_MR_ARB_LOST
    arbitration lost in SLA+R or NACK */
#define NW_MR_ARB_LOST		0x38

/** \ingroup util_twi
    \def TW_MR_SLA_ACK
    SLA+R transmitted, ACK received */
#define NW_MR_SLA_ACK		0x40

/** \ingroup util_twi
    \def TW_MR_SLA_NACK
    SLA+R transmitted, NACK received */
#define NW_MR_SLA_NACK		0x48

/** \ingroup util_twi
    \def TW_MR_DATA_ACK
    data received, ACK returned */
#define NW_MR_DATA_ACK		0x50

/** \ingroup util_twi
    \def TW_MR_DATA_NACK
    data received, NACK returned */
#define NW_MR_DATA_NACK		0x58

/* Slave Transmitter */
/** \ingroup util_twi
    \def TW_ST_SLA_ACK
    SLA+R received, ACK returned */
#define NW_ST_SLA_ACK		0xA8

/** \ingroup util_twi
    \def TW_ST_ARB_LOST_SLA_ACK
    arbitration lost in SLA+RW, SLA+R received, ACK returned */
#define NW_ST_ARB_LOST_SLA_ACK	0xB0

/** \ingroup util_twi
    \def TW_ST_DATA_ACK
    data transmitted, ACK received */
#define NW_ST_DATA_ACK		0xB8

/** \ingroup util_twi
    \def TW_ST_DATA_NACK
    data transmitted, NACK received */
#define NW_ST_DATA_NACK		0xC0

/** \ingroup util_twi
    \def TW_ST_LAST_DATA
    last data byte transmitted, ACK received */
#define NW_ST_LAST_DATA		0xC8

/* Slave Receiver */
/** \ingroup util_twi
    \def TW_SR_SLA_ACK
    SLA+W received, ACK returned */
#define NW_SR_SLA_ACK		0x60

/** \ingroup util_twi
    \def TW_SR_ARB_LOST_SLA_ACK
    arbitration lost in SLA+RW, SLA+W received, ACK returned */
#define NW_SR_ARB_LOST_SLA_ACK	0x68

/** \ingroup util_twi
    \def TW_SR_GCALL_ACK
    general call received, ACK returned */
#define NW_SR_GCALL_ACK		0x70

/** \ingroup util_twi
    \def TW_SR_ARB_LOST_GCALL_ACK
    arbitration lost in SLA+RW, general call received, ACK returned */
#define NW_SR_ARB_LOST_GCALL_ACK 0x78

/** \ingroup util_twi
    \def TW_SR_DATA_ACK
    data received, ACK returned */
#define NW_SR_DATA_ACK		0x80

/** \ingroup util_twi
    \def TW_SR_DATA_NACK
    data received, NACK returned */
#define NW_SR_DATA_NACK		0x88

/** \ingroup util_twi
    \def TW_SR_GCALL_DATA_ACK
    general call data received, ACK returned */
#define NW_SR_GCALL_DATA_ACK	0x90

/** \ingroup util_twi
    \def TW_SR_GCALL_DATA_NACK
    general call data received, NACK returned */
#define NW_SR_GCALL_DATA_NACK	0x98

/** \ingroup util_twi
    \def TW_SR_STOP
    stop or repeated start condition received while selected */
#define NW_SR_STOP		0xA0

/* Misc */
/** \ingroup util_twi
    \def TW_NO_INFO
    no state information available */
#define NW_NO_INFO		0xF8

/** \ingroup util_twi
    \def TW_BUS_ERROR
    illegal start or stop condition */
#define NW_BUS_ERROR		0x00


/**
 * \ingroup util_twi
 * \def TW_STATUS_MASK
 * The lower 3 bits of TWSR are reserved on the ATmega163.
 * The 2 LSB carry the prescaler bits on the newer ATmegas.
 */
#define NW_STATUS_MASK		(_BV(TWS7)|_BV(TWS6)|_BV(TWS5)|_BV(TWS4)|\
				_BV(TWS3))
/**
 * \ingroup util_twi
 * \def TW_STATUS
 *
 * TWSR, masked by TW_STATUS_MASK
 */
#define NW_STATUS		(TWSR & NW_STATUS_MASK)
/*@}*/

/**
 * \name R/~W bit in SLA+R/W address field.
 */

/*@{*/
/** \ingroup util_twi
    \def TW_READ
    SLA+R address */
#define NW_READ		1

/** \ingroup util_twi
    \def TW_WRITE
    SLA+W address */
#define NW_WRITE	0
/*@}*/



void nwi_init(void);
uint8_t nwi_read();
void nwi_reset_read_buffer();
void nwi_reply(uint8_t ack);
void nwi_stop(void);
void nwi_releaseBus(void);
uint8_t nwi_transmit(const uint8_t*, uint8_t);

#endif