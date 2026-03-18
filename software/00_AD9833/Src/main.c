// LED PORT
// PORT: D
// PIN: 13
// MODER13: bit 26 & bit 27

#define	PERIPH_BASE			(0x40000000UL)														// Define base address for peripherals
#define AHB1PERIPH_OFFSET 	(0x00020000UL)														// Offset for AHB1 peripheral bus

#define AHB1PERIPH_BASE 	(PERIPH_BASE + AHB1PERIPH_OFFSET)									// Base address for AHB1 peripherals

#define GPIOD_OFFSET    	(0x0C00UL)															// Offset for GPIOD
#define GPIOD_BASE 			(AHB1PERIPH_BASE + GPIOD_OFFSET)									// Base address for GPIOD

#define RCC_OFFSET			(0x3800UL)
#define RCC_BASE			(AHB1PERIPH_BASE + RCC_OFFSET)										// Base address for RCC

#define AHB1EN_R_OFFSET 	(0x30UL)															// Offset for AHB1EN register
#define RCC_AHB1EN_R 		(*(volatile unsigned int *)(RCC_BASE + AHB1EN_R_OFFSET))			// Address of AHB1EN register

#define MODE_R_OFFSET 		(0x00UL)															// Offset for mode register
#define GPIOD_MODE_R 		(*(volatile unsigned int *)(GPIOD_BASE + MODE_R_OFFSET))			// Address of GPIOD mode register

#define OD_R_OFFSET 		(0x14UL)															// Offset for output data register
#define GPIOD_OD_R 			(*(volatile unsigned int *)(GPIOD_BASE + OD_R_OFFSET))				// Address of GPIOD output data register

#define GPIODEN 			(1U << 3)															// Bit mask for enabling GPIOD (bit 3)
#define PIN13 				(1U << 13)															// Bit mask for GPIOD pin D
#define LED_PIN 			PIN13																// Alias for PIN5 representing LED pin


int main(void)
{
	// 18: Enable clock access to GPIOA
	RCC_AHB1EN_R |= GPIODEN;
	GPIOD_MODE_R |= (1U<<26); // 19: Set bit 10 to 1
	GPIOD_MODE_R &= ~(1U<<27); // 20: Set bit 11 to 0
	// 21: Start of infinite loop
	while(1)
	{
	// 22: Set PA5(LED_PIN) high
		GPIOD_OD_R |= LED_PIN;
	} // 23: End of infinite loop
} // 24: End of main function
