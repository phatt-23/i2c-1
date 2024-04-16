
#include "i2c-lib.h"
#include "si4735-lib.h"
#include <mbed.h>

//************************************************************************

// Direction of I2C communication
#define R 0b00000001
#define W 0b00000000

#define HWADR_PCF8574 0x40
#define A012 0

#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

DigitalOut g_led_PTA1(PTA1, 0);
DigitalOut g_led_PTA2(PTA2, 0);

DigitalIn g_but_PTC9(PTC9);
DigitalIn g_but_PTC10(PTC10);
DigitalIn g_but_PTC11(PTC11);
DigitalIn g_but_PTC12(PTC12);

uint8_t i2c_out_in(uint8_t t_adr,                           // adresa slave
                   uint8_t *t_out_data, uint32_t t_out_len, // write
                   uint8_t *t_in_data,  uint32_t t_in_len   // read
                  ) 
{
    i2c_start();

    uint8_t l_ack = i2c_output(t_adr | W);

    if(l_ack == 0) {
        for (int i = 0; i < t_out_len; i++) {
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
        for (u8 i = 0; i < t_in_len; ++i) {
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
    int l_ack = 0;
      #if 1

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

int main(void) {
    i2c_init();
    Radio rad();
 
    Expander led;
    led.bar(4);

	uint8_t l_status[8] = {0};
	uint16_t l_freq = 0;
	
	while(1) {
		if(g_but_PTC9 && !g_but_PTC11) {
			l_ack = rad.set_volume(rad.get_volume() + 1);	
			if(!l_ack) printf("OK :: up %d\n", rad.get_volume());
		}

		if(g_but_PTC9 && !g_but_PTC10) {
			l_ack = rad.set_volume(rad.get_volume() - 1);	
			if(!l_ack) printf("OK :: down %d\n", rad.get_volume());
		}

		if(!g_but_PTC9 && !g_but_PTC10) {
			l_ack = rad.search_freq();	
			if(!l_ack) printf("OK :: searching\n");
		}

		if(!g_but_PTC9 && !g_but_PTC11) {
			l_ack = rad.get_tune_status(l_status, 8);	
			if(!l_ack) printf("OK :: getting status\n");
			
			l_freq = (l_status[2] << 8) + l_status[3];

			printf("f (MHz) = %d -> %.2x\n", l_freq, l_freq);
			printf("fh = %x\n fl = %x\n", l_status[2], l_status[3]);
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

#if 0

#include <mbed.h>

#include "i2c-lib.h"
#include "si4735-lib.h"


#define R	0b00000001
#define W	0b00000000

#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

DigitalOut g_led_PTA1( PTA1, 0 );
DigitalOut g_led_PTA2( PTA2, 0 );

DigitalIn g_but_PTC9( PTC9 );
DigitalIn g_but_PTC10( PTC10 );
DigitalIn g_but_PTC11( PTC11 );
DigitalIn g_but_PTC12( PTC12 );

int main( void )
{
	uint8_t l_S1, l_S2, l_RSSI, l_SNR, l_MULT, l_CAP;
	uint8_t l_ack = 0;

	printf( "K64F-KIT ready...\r\n" );

	i2c_init();

	i2c_start();

	i2c_stop();

	if ( ( l_ack = si4735_init() ) != 0 )
	{
		printf( "Initialization of SI4735 finish with error (%d)\r\n", l_ack );
		return 0;
	}
	else
		printf( "SI4735 initialized.\r\n" );

	printf( "\nTunig of radio station...\r\n" );

	int l_freq = 10140; // Radiozurnal

	i2c_start();
	l_ack |= i2c_output( SI4735_ADDRESS | W);
	l_ack |= i2c_output( 0x20 );			// FM_TUNE_FREQ
	l_ack |= i2c_output( 0x00 );			// ARG1
	l_ack |= i2c_output( l_freq >> 8 );		// ARG2 - FreqHi
	l_ack |= i2c_output( l_freq & 0xff );	// ARG3 - FreqLo
	l_ack |= i2c_output( 0x00 );			// ARG4
	i2c_stop();

	wait_us( 100000 );
	printf( "... station tuned.\r\n\n" );

	i2c_start();
	l_ack |= i2c_output( SI4735_ADDRESS | W );
	l_ack |= i2c_output( 0x22 );			// FM_TUNE_STATUS
	l_ack |= i2c_output( 0x00 );			// ARG1
	i2c_start();
	l_ack |= i2c_output( SI4735_ADDRESS | R );
	l_S1 = i2c_input();
	i2c_ack();
	l_S2 = i2c_input();
	i2c_ack();
	l_freq = ( uint32_t ) i2c_input() << 8;
	i2c_ack();
	l_freq |= i2c_input();
	i2c_ack();
	l_RSSI = i2c_input();
	i2c_ack();
	l_SNR = i2c_input();
	i2c_ack();
	l_MULT = i2c_input();
	i2c_ack();
	l_CAP = i2c_input();
	i2c_nack();
	i2c_stop();

	if ( l_ack != 0 )
		printf( "Communication error!\r\n" );
	else
		printf( "Current tuned frequency: %d.%dMHz\r\n", l_freq / 100, l_freq % 100 );

	return 0;
}
#endif

#include <mbed.h>
#include "i2c-lib.h"
#include "si4735-lib.h"
#include "button_classes.h"

//************************************************************************

// Direction of I2C communication
#define R	0b00000001
#define W	0b00000000

#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

DigitalOut g_led_PTA1( PTA1, 0 );
DigitalOut g_led_PTA2( PTA2, 0 );

DigitalIn g_but_PTC9( PTC9 );
DigitalIn g_but_PTC10( PTC10 );
DigitalIn g_but_PTC11( PTC11 );
DigitalIn g_but_PTC12( PTC12 );

bool btn_state[4] = {0};

BTN_NOT btn[] {
	{PTC9, &btn_state[0], 1},
	{PTC10, &btn_state[1], 100},
	{PTC11, &btn_state[2], 100},
	{PTC12, &btn_state[3], 100}
};

///////////////////////////////////////////////////
uint8_t i2c_out_in( uint8_t t_adr, uint8_t *t_out_data, uint32_t t_out_len,uint8_t *t_in_data, uint32_t t_in_len ){
  i2c_start();

  uint8_t l_ack = i2c_output( t_adr | W );

  if ( l_ack == 0 ){
    for (int i = 0; i < t_out_len; i++ ) {
      l_ack |= i2c_output( t_out_data[ i ] ); // send all t_out_data
    }
  }

  if ( l_ack != 0 ){ 
    i2c_stop();
    return l_ack;
  }

  if ( t_in_data != nullptr ){
    i2c_start(); // repeated start

    l_ack |= i2c_output( t_adr | R );

    for (int i = 0; i < t_in_len; i++ ){
      t_in_data[ i ] = i2c_input(); // receive all t_data_in
      i2c_ack();
	  printf("in i2c_in_out read in [%d]: %x\n", i, t_in_data[i]);
    }

    i2c_nack();
  }

  i2c_stop();

  return l_ack;
  }

////Vytvořte si třídu Expander s metodou void bar( uint8_t t_level );. Tato metoda zobrazí požadovaný počet (t_level) LED v řadě (sloupeček).

  class Expander{

  public:
	  Expander(){

	  }

	  void bar(uint8_t t_leve){
		  int led = 0;

		  for(uint8_t i = 0; i < t_leve; i ++){
			  led = (led << 1);
			  led += 1;
		  }
		  uint8_t data_out[4] = {led};
		  int l_ack = i2c_out_in(0x4E, data_out, 1, nullptr, 0);
	  }
  };


//Implementujte si třídu Radio s metodami set_volume( .. ),  search_freq( .. ),

  class Radio {
	uint8_t m_volume = 0;
	uint8_t m_freq = 0;

  public:
	  Radio() {}
	  uint8_t set_volume(uint16_t sila){
		  if(sila > 63){
			  sila = 10;
		  }
		  m_volume = sila;
		  uint8_t l_data_out[ 6 ] = { 0x12, 0x00, 0x40, 0x00, 0x00, sila };
		  return i2c_out_in( SI4735_ADDRESS, l_data_out, 6, nullptr, 0 );
	  }

	  uint8_t search_freq(){
		  uint8_t l_data_out[ 2 ] = { 0x21, 0b00001100 };
		  return i2c_out_in( SI4735_ADDRESS, l_data_out, 2, nullptr, 0 );
	  }

	  uint8_t set_freq( uint16_t t_freq ) {
	    uint8_t l_data_out[ 5 ] = { 0x20, 0x00, t_freq >> 8, t_freq & 0xFF, 0 };
	    return i2c_out_in( SI4735_ADDRESS, l_data_out, 5, nullptr, 0 );
	  }

	  uint8_t get_tune_status( uint8_t *t_data_status, uint32_t t_data_len ) {
	    uint8_t l_data_out[ 2 ] = { 0x22, 0 };
	    return i2c_out_in( SI4735_ADDRESS, l_data_out, 2, t_data_status, t_data_len );
	  }
	  
	  uint8_t get_volume() const { return m_volume; }

	  uint8_t get_freq() const { return m_freq; }
  };

//////////////////////////////////////////////////

int main( void ){
	//uint8_t l_S1, l_S2, l_RSSI, l_SNR, l_MULT, l_CAP;
	//uint8_t l_ack = 0;

	Radio rad;
	int l_ack;

	i2c_init();

	if( (l_ack = si4735_init() != 0)){
		printf("Initialization of SI4735 finish with error (%d)\r\n", l_ack);
		return 0;
	} else {
		printf("SI4735 initialized.\r!n");
	}

	Expander led;
	led.bar(4);
	
	l_ack = rad.set_freq(8980);
	l_ack = rad.set_volume(10);
	uint8_t l_status[8] = {0};
	uint16_t l_freq = 0;
	
	while(1) {
		if(g_but_PTC9 && !g_but_PTC11) {
			l_ack = rad.set_volume(rad.get_volume() + 1);	
			if(!l_ack) printf("OK :: up %d\n", rad.get_volume());
		}

		if(g_but_PTC9 && !g_but_PTC10) {
			l_ack = rad.set_volume(rad.get_volume() - 1);	
			if(!l_ack) printf("OK :: down %d\n", rad.get_volume());
		}

		if(!g_but_PTC9 && !g_but_PTC10) {
			l_ack = rad.search_freq();	
			if(!l_ack) printf("OK :: searching\n");
		}

		if(!g_but_PTC9 && !g_but_PTC11) {
			l_ack = rad.get_tune_status(l_status, 8);	
			if(!l_ack) printf("OK :: getting status\n");
			
			l_freq = (l_status[2] << 8) + l_status[3];

			printf("f (MHz) = %d -> %.2x\n", l_freq, l_freq);
			printf("fh = %x\n fl = %x\n", l_status[2], l_status[3]);
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


	// while(1);
	return 0;
}
#endif