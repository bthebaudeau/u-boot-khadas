#include "registers.h"
#include <stdint.h>

/* #define P_AO_UART_WFIFO                 (0xc81004c0) */
/* #define P_AO_RTI_PIN_MUX_REG          (0xc8100014) */

#define UART_PORT_CONS P_AO_UART_WFIFO

#define UART_STP_BIT UART_MODE_MASK_STP_1BIT
#define UART_PRTY_BIT 0
#define UART_CHAR_LEN   UART_MODE_MASK_CHAR_8BIT
#define UART_MODE_RESET_MASK	\
					(UART_MODE_MASK_RST_TX \
					| UART_MODE_MASK_RST_RX \
					| UART_MODE_MASK_CLR_ERR)

#define UART_WFIFO      (0<<2)
#define UART_RFIFO	(1<<2)
#define UART_MODE	(2<<2)
#define UART_STATUS     (3<<2)
#define UART_IRQCTL	(4<<2)
#define UART_CTRL	(5<<2)
#define UART_MODE_MASK_STP_1BIT                 (0<<16)
#define UART_MODE_MASK_CHAR_8BIT                (0<<20)
#define UART_MODE_MASK_TX_EN                    (1<<12)
#define UART_MODE_MASK_RX_EN                    (1<<13)
#define UART_MODE_MASK_RST_TX                   (1<<22)
#define UART_MODE_MASK_RST_RX                   (1<<23)
#define UART_MODE_MASK_CLR_ERR                  (1<<24)
#define UART_STAT_MASK_TFIFO_FULL               (1<<21)
#define UART_CTRL_USE_XTAL_CLK			(1<<24)
#define UART_CTRL_USE_NEW_BAUD_RATE		(1<<23)

#define P_UART(uart_base, reg)		(uart_base+reg)
#define P_UART_WFIFO(uart_base)		P_UART(uart_base, UART_WFIFO)
#define P_UART_MODE(uart_base)		P_UART(uart_base, UART_MODE)
#define P_UART_CTRL(uart_base)		P_UART(uart_base, UART_CTRL)
#define P_UART_STATUS(uart_base)	P_UART(uart_base, UART_STATUS)


#define writel(v, addr) (*((unsigned *)addr) = v)
#define readl(addr) (*((unsigned *)addr))

static int uart_tx_isfull(void)
{
	return readl(P_UART_STATUS(UART_PORT_CONS)) &
		UART_STAT_MASK_TFIFO_FULL;
}

int uart_putc(int c)
{
	if (c == '\n') {
		while ((readl(P_UART_STATUS(UART_PORT_CONS)) &
					UART_STAT_MASK_TFIFO_FULL))
						;
		writel('\r', P_UART_WFIFO(UART_PORT_CONS));
	}

	/* Wait until TX is not full */
	while (uart_tx_isfull())
		;

	writel((char)c, P_UART_WFIFO(UART_PORT_CONS));
	return c;
}

int uart_puts(const char *s)
{
	while (*s)
		uart_putc(*s++);
	return 1;
}

void wait_uart_empty(void)
{
	while (!(readl(P_UART_STATUS(UART_PORT_CONS)) & (1 << 22)))
		;
}

void uart_put_hex(unsigned int data, unsigned bitlen)
{
	int i;
	unsigned char s;
	for (i = bitlen - 4; i >= 0; i -= 4) {
		if ((data >> i) == 0) {
			uart_putc(0x30);
			continue;
		}
		s = (data >> i) & 0xf;
		if (s < 10)
			uart_putc(0x30 + s);
		else
			uart_putc(0x61 + s - 10);
	}
}