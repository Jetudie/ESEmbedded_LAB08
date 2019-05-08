#include <stdint.h>
#include "reg.h"

void init_usart1(void)
{
	// PB6: USART1_Tx 
	// PB7: USART1_Rx

	// RCC EN GPIOB
	SET_BIT(RCC_BASE + RCC_AHB1ENR_OFFSET, GPIO_EN_BIT(GPIO_PORTB));
	SET_BIT(RCC_BASE + RCC_APB2ENR_OFFSET, USART1EN); // APB enable USART

	// GPIO Configurations
	// MODER 10: Alternate function mode
	SET_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_MODER_OFFSET, MODERy_1_BIT(6));
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_MODER_OFFSET, MODERy_0_BIT(6));
	SET_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_MODER_OFFSET, MODERy_1_BIT(7));
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_MODER_OFFSET, MODERy_0_BIT(7));

	// OTYPER 0
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_OTYPER_OFFSET, OTy_BIT(6));
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_OTYPER_OFFSET, OTy_BIT(7));

	// OSPEEDR led pin = 01 => Medium speed
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_OSPEEDR_OFFSET, OSPEEDRy_1_BIT(6));
	SET_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_OSPEEDR_OFFSET, OSPEEDRy_0_BIT(6));
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_OSPEEDR_OFFSET, OSPEEDRy_1_BIT(7));
	SET_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_OSPEEDR_OFFSET, OSPEEDRy_0_BIT(7));
	
	// RCC EN USART1
	WRITE_BITS(GPIO_BASE(GPIO_PORTB) + GPIOx_AFRL_OFFSET, AFRLy_3_BIT(6), AFRLy_0_BIT(6), 0b0111);
	WRITE_BITS(GPIO_BASE(GPIO_PORTB) + GPIOx_AFRL_OFFSET, AFRLy_3_BIT(7), AFRLy_0_BIT(7), 0b0111);

	// Baud
	const unsigned int BAUD = 115200;
	const unsigned int SYSCLK_MHZ = 168;
	const double USARTDIV = SYSCLK_MHZ * 1.0e6 / 16 / BAUD;

	const uint32_t DIV_MANTISSA = (uint32_t) USARTDIV;
	const uint32_t DIV_FRACTION = (uint32_t) ((USARTDIV - DIV_MANTISSA) * 16);

	//USART Configurations
	WRITE_BITS(USART1_BASE + USART_BRR_OFFSET, DIV_MANTISSA_11_BIT, DIV_MANTISSA_0_BIT, DIV_MANTISSA); // set mantissa for baudrate
	WRITE_BITS(USART1_BASE + USART_BRR_OFFSET, DIV_FRACTION_3_BIT, DIV_FRACTION_0_BIT, DIV_FRACTION); // set fraction for baudrate
	
	SET_BIT(USART1_BASE + USART_CR1_OFFSET, UE_BIT); // enable USART
	SET_BIT(USART1_BASE + USART_CR1_OFFSET, RE_BIT); // enable receiver
	SET_BIT(USART1_BASE + USART_CR1_OFFSET, TE_BIT); // enable transimitter
	

}

void usart1_send_char(const char ch)
{
	// wait until TXE bit is set
	while(!READ_BIT(USART1_BASE + USART_SR_OFFSET, TXE_BIT))
		;
	
	// Write the data to send in the USART_DR register (this clears the TXE bit)
	REG(USART1_BASE + USART_DR_OFFSET) = ch;
}

char usart1_receive_char(void)
{
	// When a character is received, the RXNE bit is set
	while(!READ_BIT(USART1_BASE + USART_SR_OFFSET, RXNE_BIT))
		;
    
	// Read to the USART_DR register (this clears the RXNE bit)
	return (char) REG(USART1_BASE + USART_DR_OFFSET);
}

int main(void)
{
	init_usart1();

	char *hello = "Hello world!\r\n";

	//send Hello world
	while (*hello != '\0')
		usart1_send_char(*hello++);


	//receive char and resend it
	char ch;
	while (1)
	{
		ch = usart1_receive_char();

		if (ch == '\r')
			usart1_send_char('\n');

		usart1_send_char(ch);
	}
}
