/*
 * main.c
 *
 *  Created on: 2012/10/08
 *      Author: sin
 */

#include <stdio.h>
#include <stdint.h>

#include <stm32f4xx.h>
#include <stm32f4xx_usart.h>

#include "stm32f4xx_it.h"

#include "armcore.h"
#include "gpio.h"
#include "delay.h"
#include "usart.h"
#include "spi.h"
#include "i2c.h"

#include "ST7032i.h"
#include "USARTSerial.h"

ST7032i lcd;
USARTSerial Serial3(USART3);
I2CBus Wire;

int main(void) {
	uint16_t bits;
	uint32_t intval = 40;
	uint32_t tnow;
	uint8_t tmp[64];

	TIM2_timer_start();

	Serial3.begin(PC11, PC10, 19200);

	Serial3.print("This throne of king, this sceptr'd isle, \n");
	Serial3.print( "The kingdom of heaven belongs to them!\n");
	Serial3.print( "If thou beest he! But O how fall'n!\n");
	Serial3.print( "How chang'd from him!\n");
	Serial3.flush();
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);

	Serial3.print( "SYSCLK = ");
	Serial3.print(RCC_Clocks.SYSCLK_Frequency);
	Serial3.print( ", HCLK = ");
	Serial3.print( RCC_Clocks.HCLK_Frequency);
	Serial3.print( ", PCLK1 = ");
	Serial3.print( RCC_Clocks.PCLK1_Frequency);
	Serial3.print( ", PCLK2 = ");
	Serial3.print(RCC_Clocks.PCLK2_Frequency);
	Serial3.print("\n");
	Serial3.flush();

	GPIOMode(PinPort(PD12),
			(PinBit(PD12) | PinBit(PD13) | PinBit(PD14) | PinBit(PD15)), OUTPUT,
			FASTSPEED, PUSHPULL, NOPULL);
	spi_begin(SPI2, PB13, PB14, PB15, PB12);
	digitalWrite(PB12, HIGH);

	i2c_begin(&Wire, I2C1, PB9, PB8, 100000);
	lcd.init(&Wire);
	lcd.begin();
	lcd.setContrast(46);
	lcd.print("Sad!");       // Classic Hello World!

	bits = GPIO_ReadOutputData(GPIOD );
	GPIOWrite(GPIOD, PinBit(PD13) | (bits & 0x0fff));
	delay_ms(intval);
	tnow = millis() / 1000;
	while (tnow == millis() / 1000)
		;
	tnow = millis() / 1000;

	while (1) {
		bits = GPIO_ReadOutputData(GPIOD );

		GPIOWrite(GPIOD, PinBit(PD13) | (bits & 0x0fff));
		delay_ms(intval);
		GPIOWrite(GPIOD, PinBit(PD14) | (bits & 0x0fff));
		delay_ms(intval);
		GPIOWrite(GPIOD, PinBit(PD15) | (bits & 0x0fff));
		delay_ms(intval);
		GPIOWrite(GPIOD, PinBit(PD12) | (bits & 0x0fff));
		delay_ms(intval);
		//
		bits &= 0x0fff;
		switch ((tnow % 60) / 15) {
		case 3:
			bits |= PinBit(PD12);
		case 2:
			bits |= PinBit(PD15);
		case 1:
			bits |= PinBit(PD14);
		case 0:
		default:
			bits |= PinBit(PD13);
			break;
		}
		GPIOWrite(GPIOD, bits);

		while (tnow == millis() / 1000);
		tnow = millis() / 1000;

		Serial3.println(millis());

		lcd.setCursor(0, 1);
		lcd.print((float)millis()/1000, 2);
		lcd.print('.');

		digitalWrite(PB12, LOW);
		spi_transfer(SPI2, (uint8_t *) tmp, 8);
		digitalWrite(PB12, HIGH);

		uint16_t i = 0;
		if (Serial3.available() > 0) {
			while (Serial3.available() > 0 && i < 92) {
				tmp[i++] = (char) Serial3.read();
			}
			tmp[i] = 0;
			Serial3.print( "> ");
			Serial3.println((char*)tmp);
		}

	}
	return 0;
}
