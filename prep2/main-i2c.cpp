
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#include "radio.hpp"

DigitalOut g_led_PTA1(PTA1, 0);
DigitalOut g_led_PTA2(PTA2, 0);

DigitalIn g_but_PTC9(PTC9);
DigitalIn g_but_PTC10(PTC10);
DigitalIn g_but_PTC11(PTC11);
DigitalIn g_but_PTC12(PTC12);

#define separate() printf("----------------------------------\n");
char g_buf[256] = {0};

char g_station_name[8] = {0};
char g_radiotext[64] = {0};

int main(void) {
    i2c_init();
	int l_ack = 0;

    Radio rad;

    if((l_ack = si4735_init() != 0)) {
        printf("Initialization of SI4735 finish with error (%d)\n", l_ack);
        return 0;
    } else printf("SI4735 initialized.!n");

    l_ack = rad.set_freq(8900);
    l_ack = rad.set_volume(30);

	uint8_t l_status[8] = {0};
	uint16_t l_freq = 0;

    uint8_t l_rds[13] = {0}; 

	while(1) {
		if(g_but_PTC9 && !g_but_PTC11) {
			separate();
            l_ack = rad.set_volume(rad.get_volume() + 1);	
			if(!l_ack) 
                printf("OK :: Volume Up %d\n", rad.get_volume());
		   separate();
        }

		if(g_but_PTC9 && !g_but_PTC10) {
			separate();
            l_ack = rad.set_volume(rad.get_volume() - 1);	
			if(!l_ack) 
                printf("OK :: Volume Down %d\n", rad.get_volume());
            separate();
		}

		if(!g_but_PTC9 && !g_but_PTC10) {
            separate();
			l_ack = rad.search_freq();	
			if(!l_ack) 
                printf("OK :: Searching Station\n");
            rad.get_tune_status(l_status, 8);
            separate();
		}

		if(!g_but_PTC9 && !g_but_PTC11) {
			l_ack = rad.get_tune_status(l_status, 8);	
			if(!l_ack) printf("OK :: Getting Status\n");

            // printf("f (0x 0x) = %x %x\n", l_status[2], l_status[3]);
            // printf("f (0x) = %.4x\n", (l_status[2] << 8) + l_status[3]);
            // printf("f (num) = %d\n", (l_status[2] << 8) + l_status[3]);
            separate();
		}
		
		// if(g_but_PTC9 && !g_but_PTC12) {
		// 	l_ack = rad.set_freq(rad.get_freq() + 10);
		// 	if(!l_ack) printf("OK :: Freq Up\n");
		// }
		
		// if(!g_but_PTC9 && !g_but_PTC12) {
        //     separate();
		// 	l_ack = rad.set_freq(rad.get_freq() - 10);
		// 	if(!l_ack) printf("OK :: Freq Down\n");
        //     separate();
		// }

        if(!g_but_PTC9 && !g_but_PTC12) {
            separate();
            printf(">> BUTTONS:  1 0 0 1 \n");
            l_ack = rad.get_rds_status(l_rds, 13);
            if(!l_ack)
                printf("OK :: Getting RDS Status\n");
            
            if(l_rds[6] >> 3 == 0) {
                i32 p = (l_rds[7] & 0x02) * 2;
                g_station_name[p] = l_rds[10];
                g_station_name[p + 1] = l_rds[11];
            }
            printf("Radio Station Name: %s\n", g_station_name);

            if(l_rds[6] >> 3 == 4) {
                i32 p = (l_rds[7] & 0x0f) * 4;
                for(size_t i = 0; i < 4; ++i)
                    g_radiotext[p + i] = rds[8 + i];
            }
            printf("Radiotext: %s\n", g_radiotext);

            separate();
        }
	}

    // while (1);
    return 0;
}
