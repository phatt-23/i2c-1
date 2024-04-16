#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#include "i2c-lib.h"
#include "si4735-lib.h"
#include <mbed.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#define R 0b00000001
#define W 0b00000000

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

DigitalOut g_led_PTA1(PTA1, 0);
DigitalOut g_led_PTA2(PTA2, 0);

u8 i2c_out_in(u8 t_adr, 
            u8* t_out_data, u32 t_out_len, 
            u8* t_in_data, u32 t_in_len) 
{
    i2c_start();
    u8 l_ack = i2c_output(t_adr | W);
    if(l_ack == 0) {
        for(size_t i = 0; i < t_out_len; ++i) {
            l_ack |= i2c_output(t_out_data[i]); 
        }
    }
    if(l_ack != 0) { 
        i2c_stop();
        return l_ack;
    }
    if(t_in_data != nullptr) {
        i2c_start(); 
        l_ack |= i2c_output(t_adr | R);
        for(size_t i = 0; i < t_in_len; ++i) {
            i2c_ack();
            t_in_data[i] = i2c_input();
        }
        i2c_nack();
    }
    i2c_stop();
    return l_ack;
}

class Expander {
public:
    Expander() {}

    void bar(u8 t_level) {
      #if 1

        int l_ack = 0;
        u8 l_led = 0;
        for(size_t i = 0; i < t_level; ++i) {
            l_led += 1 << i;
        }
        l_ack |= i2c_output( HWADR_PCF8574 | A012 | W );
		l_ack |= i2c_output( l_led );

      #else 
        
        int led = 0;
        for (u8 i = 0; i < t_level; i++) {
          led = (led << 1);
          led += 1;
        }

        u8 data_out[4] = {led};
        l_ack = i2c_out_in(0x4E, data_out, 1, nullptr, 0);
        
      #endif
    }
};

class Radio {
public:
    Radio() {
        if((l_ack = si4735_init() != 0)) {
            printf("Initialization of SI4735 finish with error (%d)\r\n", l_ack);
            return 0;
        } else
            printf("SI4735 initialized.\r!n");

        l_ack = rad.set_freq(8900);
        l_ack = rad.set_volume(30);
    }

    u8 set_volume(u16 t_level) {
        if (t_level > 63) {
          t_level = 10;
        }
        u8 l_data_out[6] = {0x12, 0x00, 0x40, 0x00, 0x00, t_level};
        return i2c_out_in(SI4735_ADDRESS, l_data_out, 6, nullptr, 0);
    }

    u8 search_freq() {
        u8 l_data_out[2] = {0x21, 0b00001100};
        return i2c_out_in(SI4735_ADDRESS, l_data_out, 2, nullptr, 0);
    }

    u8 set_freq(u16 t_freq) {
        u8 l_data_out[5] = {0x20, 0x00, t_freq >> 8, t_freq & 0xFF, 0};
        return i2c_out_in(SI4735_ADDRESS, l_data_out, 5, nullptr, 0);
    }

    u8 get_tune_status(u8* t_data_status, u32 t_data_len) {
        u8 l_data_out[2] = {0x22, 0};
        return i2c_out_in(SI4735_ADDRESS, l_data_out, 2, t_data_status, t_data_len);
    }
};

//////////////////////////////////////////////////

char g_buf[256] = {0};

int main(void) {
    i2c_init();
    Radio rad();
 
    Expander led;
    led.bar(4);

	uint8_t l_status[8] = {0};
	double l_freq = 0.f;
	
	while(1) {
		if(g_but_PTC9 && !g_but_PTC11) {
			l_ack = rad.set_volume(rad.get_volume() + 1);	
			if(!l_ack) 
                printf("OK :: Volume Up %d\n", rad.get_volume());
		}

		if(g_but_PTC9 && !g_but_PTC10) {
			l_ack = rad.set_volume(rad.get_volume() - 1);	
			if(!l_ack) 
                printf("OK :: Volume Down %d\n", rad.get_volume());
		}

		if(!g_but_PTC9 && !g_but_PTC10) {
			l_ack = rad.search_freq();	
			if(!l_ack) 
                printf("OK :: searching\n");
		}

		if(!g_but_PTC9 && !g_but_PTC11) {
			l_ack = rad.get_tune_status(l_status, 8);	
			if(!l_ack) 
                printf("OK :: getting status\n");

            sprintf(g_buf, "%d.%d", l_status[2] / 100, l_status[3] % 100);
			l_freq = atof(g_buf);
			printf("f = %d.%d MHz -> %lf\n", 
                l_status[2] / 100, l_status[3] % 100, l_freq);
		}
		
		if(g_but_PTC9 && !g_but_PTC12) {
			l_ack = rad.set_freq(rad.get_freq() + 10);
			if(!l_ack) printf("OK :: Freq Up\n");
		}
		
		if(!g_but_PTC9 && !g_but_PTC12) {
			l_ack = rad.set_freq(rad.get_freq() - 10);
			if(!l_ack) printf("OK :: Freq Down\n");
		}
	}

    // while (1);
    return 0;
}
