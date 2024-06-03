#include <stdbool.h>
#include <stm8s.h>
#include "main.h"
#include "milis.h"
#include "uart1.h"
#include "swi2c.h"
#include <stdio.h>

void init(void)
{
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);      // taktovani MCU na 16MHz
    GPIO_Init(BTN_PORT, BTN_PIN, GPIO_MODE_IN_FL_NO_IT);

    init_milis();
    init_uart1();
    swi2c_init();
}


int main(void)
{


    uint32_t time = 0;
    uint8_t precteno[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t zapsano[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t data[] = { 0xA, 0xB, 0xC, 0xD, 0xE, 0xF };
    uint8_t err;
    uint16_t *p_teplota;
    uint8_t teplota;
    int8_t desetina;


    init();

    printf("\nScan I2C bus:\n");
    printf("Recover: 0x%02X\n", swi2c_recover());
    for (uint8_t addr = 0; addr < 128; addr++) {
        if (swi2c_test_slave(addr << 1) == 0) {
            printf("0x%02X \n", addr);
        }
    }
    printf("------------- scan end --------------------\n");

    /*-----  zápis do EEPROM  ----------- */
    err = swi2c_write_eemem(0x57 << 1, 0, data, 6);     // zapisuji od adresy 0
    printf("návratový kód zápisu do EEPROM: %d\n", err);
    for (uint8_t i = 0; i < sizeof(data); i++) {
        data[i] = 0;            // do pole si přepíšu nulama
    }
    /*-----  čtení z  EEPROM  ----------- */
    err = swi2c_read_eemem(0x57 << 1, 2, data, 6);      // čtu od adresy 2
    printf("návratový kód čtení z EEPROM: %d\n", err);
    printf("data: ");
    for (uint8_t i = 0; i < sizeof(data); i++) {
        printf("%02X ", data[i]);
    }
    putchar('\n');

    /*     Nastavení času v RTC */

    // RTC používá BCD kód, proto používám HEXA
    zapsano[0] = 0x00;          // sekundy
    zapsano[1] = 0x39;          // minuty
    zapsano[2] = 0x10;          // hodiny
    zapsano[3] = 0x01;          // den v týdnu
    zapsano[4] = 0x03;          // den
    zapsano[5] = 0x06;          // měsíc
    zapsano[6] = 0x24;          // rok
/*
    while(!PUSH(BTN));
    printf("Zápis do RTC StatusCode: %X\n",  swi2c_write_buf(0x68 <<1 , 0x00, zapsano, 7));
*/

    while (1) {
        if (milis() - time > 10E3) {
            time = milis();

            // čas
            err = swi2c_read_buf(0x68 << 1, 0x00, precteno, 7);
            printf("%d%d.%d%d. 20%d%d %d%d:%d%d:%d%d \n",       // v RTC obvodu je čas uložen v BCD
                   precteno[4] >> 4, precteno[4] & 0x0F,
                   precteno[5] >> 4, precteno[5] & 0x0F,
                   precteno[6] >> 4, precteno[6] & 0x0F,
                   precteno[2] >> 4, precteno[2] & 0x0F,
                   precteno[1] >> 4, precteno[1] & 0x0F,
                   precteno[0] >> 4, precteno[0] & 0x0F);

            // teplota
            err = swi2c_read_buf(0x48 << 1, 0x00, precteno, 2);
            p_teplota = (uint16_t *) precteno;
            teplota = precteno[0];
            if (precteno[1] & 0b10000000) {
                desetina = 5;
            } else {
                desetina = 0;
            }
            printf("Teplota: %d,%d°C  <-  RAW: %02X\n", teplota, desetina,
                   *p_teplota);

        }
    }
}

/*-------------------------------  Assert -----------------------------------*/
#include "__assert__.h"
