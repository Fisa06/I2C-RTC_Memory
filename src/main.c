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

    init_milis();
    init_uart1();
    swi2c_init();
}


int main(void)
{
  

    uint32_t time = 0;
    uint8_t precteno[10] = {0,0,0,0,0,0,0,0,0,0};
    uint8_t zapsano[10] = {0,0,0,0,0,0,0,0,0,0};
    uint8_t err;
    uint16_t * p_teplota;
    uint8_t teplota;
    int8_t desetina;


    init();

    printf("\nScan I2C bus:\n");
    printf("Recover: 0x%02X\n", swi2c_recover());
    for (uint8_t addr = 0; addr < 128; addr++) {
        if (swi2c_test_slave(addr<<1) == 0) {
            printf("0x%02X \n", addr);
        }
    }
    printf("------------- scan end --------------------\n");

    while (1) {
        if (milis() - time > 1111 ) {
            time = milis();

            // čas
            err = swi2c_read_buf(0x68<<1, 0x00, precteno, 7);
            printf("%d%d.%d%d. 20%d%d %d%d:%d%d:%d%d \n",       // v RTC obvodu je čas uložen v BCD
                   precteno[4] >> 4, precteno[4] & 0x0F,
                   precteno[5] >> 4, precteno[5] & 0x0F,
                   precteno[6] >> 4, precteno[6] & 0x0F,
                   precteno[2] >> 4, precteno[2] & 0x0F,
                   precteno[1] >> 4, precteno[1] & 0x0F,
                   precteno[0] >> 4, precteno[0] & 0x0F);

            // teplota
            err = swi2c_read_buf(0x48, 0x00, precteno, 2);
            p_teplota = (uint16_t *) precteno;
            teplota = precteno[0];
            if (precteno[1] & 0b10000000) {
                desetina = 5; 
            } else {
                desetina = 0; 
            }
            printf("%d,%d     <-   %02X\n", teplota, desetina, * p_teplota);

        }
    }
}

/*-------------------------------  Assert -----------------------------------*/
#include "__assert__.h"
