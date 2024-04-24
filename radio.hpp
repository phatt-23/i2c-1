#include <mbed.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <mbed.h>
#include "i2c-lib.h"
#include "si4735-lib.h"

#define R       0b00000001
#define W       0b00000000
#define PCF8574 0b01000000
                //^^^^  
#define A123    0b00000000
                //    ^^^
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

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
            t_in_data[i] = i2c_input();
            i2c_ack();
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
        i2c_start();
        u8 l_led = 0;
        for(size_t i = 0; i < t_level; ++i) {
            l_led <<= 1;
            l_led += 1;
        }
        printf("LED: %.8b\n", l_led);
        i2c_output(PCF8574 | A123 | W);
		i2c_output(l_led);
        i2c_stop();
    }
};

i32 clamp(i32* x, i32 l, i32 h) {
    if(*x > h) *x = h;
    if(*x < l) *x = l; 
}

class Radio {
    uint8_t m_vol = 0;
    uint16_t m_freq = 0;

public:
    Expander led;
    Radio() {}

    u8 set_volume(u8 t_level) {
        if (t_level > 63) {
          t_level = 63;
        }
        if(t_level == 0) {
            t_level = 1;
        }
        m_vol = t_level;
        led.bar(u8(float(t_level + 1) / 64.0 * 8));
        u8 l_data_out[6] = {0x12, 0x00, 0x40, 0x00, 0x00, t_level};
        return i2c_out_in(SI4735_ADDRESS, l_data_out, 6, nullptr, 0);
    }

    u8 search_freq() {
        u8 l_data_out[2] = {0x21, 0b00001100};
        int l_ack = 0;
        l_ack = i2c_out_in(SI4735_ADDRESS, l_data_out, 2, nullptr, 0);
        return l_ack;
    }

    u8 set_freq(u16 t_freq) {
        u8 l_data_out[5] = {0x20, 0x00, (u8)(((u32)t_freq) >> 8), (u8)(t_freq & 0x00FF), 0};
        m_freq = t_freq;
        printf("--> Freq set to %d\n", t_freq);
        int l_ack = 0;
        l_ack |= i2c_out_in(SI4735_ADDRESS, l_data_out, 5, nullptr, 0);
        u8 l_status[8] = {0};
        get_tune_status(l_status, 8);
        this->led.bar(l_status[4] / 2);
        return l_ack;
    }

    u8 get_tune_status(u8* t_data_status, u32 t_data_len) {
        u8 l_data_out[2] = {0x22, 0};
        int l_ack = 0;
        l_ack = i2c_out_in(SI4735_ADDRESS, l_data_out, 2, t_data_status, t_data_len);
        
        printf("f (0x 0x) = %x %x\n", t_data_status[2], t_data_status[3]);
        printf("f (0x) = %.4x\n", (t_data_status[2] << 8) + t_data_status[3]);
        printf("f (num) = %d\n", (t_data_status[2] << 8) + t_data_status[3]);
        printf("strength: %d dBμV -> (0x) %x\n", t_data_status[4], t_data_status[4]);

        return l_ack;
    }
#if 1 // WARNING: this was implemented blindly
    u8 get_signal_quality(u8* data, u32 len) {
        u8 command[2] = {0x23, 0x00};
        int ack = 0;
        ack = i2c_out_in(SI4735_ADDRESS, command, 2, data, 8);
        
        int led_level = (data[3] & 0x7f) / 8;
        led.bar(led_level);

        return ack;
    }
#endif

    u8 get_rds_status(u8* t_data_status, u32 t_data_len) {
        int l_ack = 0;
        u8 l_data_out[2] = { 0x24, 0x01 };
        l_ack = i2c_out_in(SI4735_ADDRESS, l_data_out, 2, t_data_status, 13);



        return l_ack;
    }

    u8 get_volume() { return m_vol; }
    u16 get_freq() { return m_freq; }
};

