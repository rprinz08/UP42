#ifndef _XMODEM_H_
#define _XMODEM_H_

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define CTRLZ 0x1A

#define DLY_1S 1000
#define MAXRETRANS 25

extern int xmodemReceive(unsigned char *, int);
extern int xmodemTransmit(unsigned char *, int);

#endif