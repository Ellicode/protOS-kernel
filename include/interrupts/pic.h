#ifndef PIC_H
#define PIC_H

#include <stdint.h>

#define PIC1_ADDR	           0x20		/* IO base address for master PIC */
#define PIC2_ADDR    	       0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	  PIC1_ADDR
#define PIC1_DATA	(PIC1_ADDR + 1)
#define PIC2_COMMAND	  PIC2_ADDR
#define PIC2_DATA	(PIC2_ADDR + 1)

#define PIC_EOI		           0x20		/* End-of-interrupt command code */

#define ICW1_ICW4	           0x01     /* Indicates that ICW4 will be present */
#define ICW1_SINGLE	           0x02     /* Single (cascade) mode */
#define ICW1_INTERVAL4         0x04     /* Call address interval 4 (8) */
#define ICW1_LEVEL	           0x08     /* Level triggered (edge) mode */
#define ICW1_INIT	           0x10     /* Initialization - required! */

#define ICW4_8086              0x01     /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	           0x02     /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	       0x08     /* Buffered mode/slave */
#define ICW4_BUF_MASTER	       0x0C     /* Buffered mode/master */
#define ICW4_SFNM	           0x10     /* Special fully nested (not) */

typedef uint8_t irq_t;

void pic_init();
void eoi(uint8_t irq);
void unmask_irq(uint8_t irq);
void mask_irq(uint8_t irq);

#endif // PIC_H