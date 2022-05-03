/**
* \file main.c
*
* \brief Main source file.
*
(c) 2022 Microchip Technology Inc. and its subsidiaries.
    Subject to your compliance with these terms, you may use this software and
    any derivatives exclusively with Microchip products. It is your responsibility
    to comply with third party license terms applicable to your use of third party
    software (including open source software) that may accompany Microchip software.
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE.
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*
*/

#define F_CPU  4000000UL

#include <util/delay.h>
#include <avr/io.h>
#include <stdint.h>

#define HOST_DATA              'H'
#define CLIENT_DATA            'C'
#define NULL_DATA              '\0'

#define SPI_CLIENT_SELECT()    PORTA.OUTCLR = PIN7_bm  /* Set SS pin value to LOW */
#define SPI_CLIENT_DESELECT()  PORTA.OUTSET = PIN7_bm  /* Set SS pin value to HIGH */

/* Default fuses configuration:
- BOD disabled
- Oscillator in High-Frequency Mode
- UPDI pin active(WARNING: DO NOT CHANGE!)
- RESET pin used as GPIO
- CRC disabled
- MVIO enabled for dual supply
- Watchdog Timer disabled
*/
FUSES =
{
    .BODCFG = ACTIVE_DISABLE_gc | LVL_BODLEVEL0_gc | SAMPFREQ_128Hz_gc | SLEEP_DISABLE_gc,
    .BOOTSIZE = 0x0,
    .CODESIZE = 0x0,
    .OSCCFG = CLKSEL_OSCHF_gc,
    .SYSCFG0 = CRCSEL_CRC16_gc | CRCSRC_NOCRC_gc | RSTPINCFG_GPIO_gc | UPDIPINCFG_UPDI_gc,
    .SYSCFG1 = MVSYSCFG_DUAL_gc | SUT_0MS_gc,
    .WDTCFG = PERIOD_OFF_gc | WINDOW_OFF_gc,
};

void LED_Init(void)
{
    PORTF.OUTSET = PIN5_bm;
    PORTF.DIRSET = PIN5_bm;
}

void LED_Toggle(void)
{
    PORTF.OUTTGL = PIN5_bm;
}

void SPI0_init(void)
{
    /* SPI in host mode setting */
    PORTA.PIN7CTRL |= PORT_PULLUPEN_bm; /* Pull-up enable on PIN7 */
    PORTA.OUTSET = PIN7_bm;             /* Set SS pin to high state */
    PORTA.DIRSET = PIN4_bm;             /* Set MOSI pin direction to output */
    PORTA.DIRCLR = PIN5_bm;             /* Set MISO pin direction to input */
    PORTA.DIRSET = PIN6_bm;             /* Set SCK pin direction to output */
    PORTA.DIRSET = PIN7_bm;             /* Set SS pin direction to output */

    /* SPI Clock speed is set to 2 x 4MHz / 16 = 500 kHz */
    SPI0.CTRLA = SPI_CLK2X_bm           /* Enable double-speed */
               | SPI_ENABLE_bm          /* Enable module */
               | SPI_MASTER_bm          /* SPI module in Host mode */
               | SPI_PRESC_DIV16_gc;    /* System Clock divided by 16 */
}

uint8_t SPI0_exchangeData(uint8_t sent_data)
{
    SPI0.DATA = sent_data;
    while (!(SPI0.INTFLAGS & SPI_IF_bm));  /* waits until data is exchanged */

    return SPI0.DATA;
}

int main(void)
{
    uint8_t rx_data, tx_data = HOST_DATA;
    LED_Init();
    SPI0_init();

    while (1)
    {
        SPI_CLIENT_SELECT();
        rx_data = SPI0_exchangeData(tx_data);
        SPI_CLIENT_DESELECT();
        if(rx_data == CLIENT_DATA)
        {
            LED_Toggle();
            tx_data = HOST_DATA;
        }
        else
        {
            tx_data = NULL_DATA;
        }

        _delay_us(25);
    }
}
