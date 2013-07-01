#define F_CPU 1000000UL

#include <stdint.h> 
#include <stdbool.h> 
#include <avr/io.h> 
#include <avr/interrupt.h> 
//#include <avr/delay.h> 
#include <util/delay.h> 

#define R_MOT_F PD5 
#define R_MOT_B PD4

#define L_MOT_F PD6
#define L_MOT_B PD7

#define IR_LED PB6
#define IR_SENSOR PINB7

#define R_SENZOR PINC5
#define L_SENZOR PINC4

#define LED_1 PD0
#define LED_2 PD1
#define LED_3 PD2







// -------------------------- 8-Bit PWM --------------------------- //
// An - 254 Graustufen - Aus

#define CHANNELS 4
#define PWM_PORT PORTD

uint8_t pwm_pin[CHANNELS] = { PD4, PD5, PD6, PD7 };

uint8_t pwm_mask_A[CHANNELS + 1];
uint8_t pwm_timing_A[CHANNELS + 1];

uint8_t pwm_mask_B[CHANNELS + 1];
uint8_t pwm_timing_B[CHANNELS + 1];

uint8_t *ptr_PORT_main = pwm_mask_A;
uint8_t *ptr_timing_main = pwm_timing_A;

uint8_t *ptr_PORT_isr = pwm_mask_B;
uint8_t *ptr_timing_isr = pwm_timing_B;

volatile uint8_t pwm_value[CHANNELS];
volatile uint8_t pwm_queue[CHANNELS];

volatile uint8_t pwm_cycle;
volatile bool pwm_change;


void pwm_update(void)
{
  uint8_t i;
  pwm_change = 0;

  // Sortieren mit InsertSort

  for( i=1; i<CHANNELS; i++)
  {
    uint8_t j = i;

    while(j && pwm_value[pwm_queue[j - 1]] > pwm_value[pwm_queue[j]])
    {
      uint8_t temp = pwm_queue[j - 1];
      pwm_queue[j - 1] = pwm_queue[j];
      pwm_queue[j] = temp;

      j--;
    }
  }

  // Maske generieren

  for( i=0; i<(CHANNELS + 1); i++)
  {
    ptr_PORT_main[i] = 0;
    ptr_timing_main[i] = 0;
  }

  uint8_t j = 0;

  for( i=0; i<CHANNELS; i++)
  {
    // Sonderfall "ganz AUS" ausschließen:
    //  - Pins garnicht erst anschalten
    //  - Pins die vorher "ganz AN" waren müssen trotzdem noch deaktiviert werden, deshalb nur folgendes in der if
    if(pwm_value[pwm_queue[i]] != 0)
    {
      ptr_PORT_main[0] |= (1 << pwm_pin[pwm_queue[i]]);
    }

    // Sonderfall "ganz AN" ausschließen:
    //  - Nicht in Timing-Tabelle übernehmen da sonst die Pointer niemals getauscht werden
    //  - Pins die "ganz AN" sind müssen nicht abgeschaltet werden
    if(pwm_value[pwm_queue[i]] != 255)
    {
      ptr_PORT_main[j + 1] |= (1 << pwm_pin[pwm_queue[i]]);

      ptr_timing_main[j] = pwm_value[pwm_queue[i]];
    }

    if(pwm_value[pwm_queue[i]] != pwm_value[pwm_queue[i + 1]])
    {
      j++;
    }
  }

  /*
  // Der ISR etwas Arbeit abnehmen
  for( i=1; i<(CHANNELS + 1); i++)
  {
    ptr_PORT_main[i] = ~ptr_PORT_main[i];
  }
  */

  pwm_change = 1;
}


ISR(TIMER1_COMPA_vect)
{
  pwm_cycle = 0;

  PWM_PORT |= ptr_PORT_isr[0];
  OCR1B = ptr_timing_isr[0];
}



ISR(TIMER1_COMPB_vect)
{
  pwm_cycle++;

  PWM_PORT &= ~ptr_PORT_isr[pwm_cycle];

  if(ptr_timing_isr[pwm_cycle] != 0)
  {
    OCR1B = ptr_timing_isr[pwm_cycle];
  }
  else if(pwm_change)
  {
    pwm_change = 0;

    uint8_t *temp_ptr;

    temp_ptr = ptr_PORT_main;
    ptr_PORT_main = ptr_PORT_isr;
    ptr_PORT_isr = temp_ptr;

    temp_ptr = ptr_timing_main;
    ptr_timing_main = ptr_timing_isr;
    ptr_timing_isr = temp_ptr;
  }
}



void set_motor(int left, int right);
void beep(int time);

int main(void) 
{ 	
  uint8_t i;
  int t=0;

// PWM initialisieren
  for( i=0; i<CHANNELS; i++)
  {
    pwm_queue[i] = i;
  }

  // CLKPR=_BV(CLKPCE);
  //CLKPR=0;
	PORTC = 0b111111; /* pull-up u všech pinů na portu C*/
	DDRC = 0b000000; /* port C nastaven jako vstup */
	
	PORTD = 0x00;
	DDRD = 0b11111111;	/* PORT D jako výstup */
	
	 
	PORTB = 0b10001111; /* nastavení pull-upů - 1 = pull-up na konkrétním pinu*/
	DDRB  = 0b01000000; /* nastevení portu B jaký pin bude jako výstup a jaký jako vstup */
	
	ASSR = 0x00;
	TCCR2 = 0x01;
	TCNT2 = 178;
	OCR2 = 0x00;
	OCR1A = 255;
	//TIMSK = (1 << OCIE1A) | (1 << OCIE1B);
	TCCR1B = (1 << WGM12) | 4;
	MCUCR = 0x00;
	TIMSK = 0x40|(1 << OCIE1A) | (1 << OCIE1B);
	sei();
						
	  beep(200);
	  beep(250);
	  beep(300);
	  beep(350);
	  
	  
	  while(1)
	    {
	      uint8_t cmd;
	      

	      cmd=PINC & 0x0f;
	      if(cmd == 0xe)
		if (t > -255) t--;
	      {
		set_motor(t,t);
	      }
	      if(cmd == 0xd)
		{
		  if ( t < 255) t++;
		  {
		set_motor(t,t);
		  }
		}
	      if(cmd == 0x7)
		set_motor(-128,128);
	      if(cmd == 0xb)
		set_motor(128,-128);
	      if(cmd == 0x3)
		{set_motor(0,0); t=0; }
	      _delay_ms(100);
	     
	    }














	
	/* Začátek programu Slidila na DO */
		
		if(bit_is_set(PINB,PINB0) && bit_is_set(PINB,PINB1) && bit_is_set(PINB,PINB2) && bit_is_set(PINB,PINB3)) // 1111
		{
				 beep(200);
				 beep(250);
				 beep(300);
				 beep(350);
			while(1)
			{
				if ((PINC & 0xf) == 0xf) 
				PORTD &= ~((1<<R_MOT_F) | (1<<R_MOT_B) | (1<<L_MOT_F) | (1<<L_MOT_B));
				
				if(bit_is_clear(PINC,PINC0) && bit_is_set(PINC,PINC1) && bit_is_set(PINC,PINC2) && bit_is_set(PINC,PINC3)) // dopredu
				{	
					_delay_ms(10);
					if(bit_is_clear(PINC,PINC0) && bit_is_set(PINC,PINC1) && bit_is_set(PINC,PINC2) && bit_is_set(PINC,PINC3)) // dopredu
					{											
						PORTD |= (1<<R_MOT_F) | (1<<L_MOT_F);				
						//PORTD |= (1<<R_MOT_B) | (1<<L_MOT_B);			
						_delay_ms(100);
					}
				}
				
				if(bit_is_set(PINC,PINC0) && bit_is_clear(PINC,PINC1) && bit_is_set(PINC,PINC2) && bit_is_set(PINC,PINC3)) // dozadu
				{	
					_delay_ms(10);
					if(bit_is_set(PINC,PINC0) && bit_is_clear(PINC,PINC1) && bit_is_set(PINC,PINC2) && bit_is_set(PINC,PINC3)) // dozadu
					{
						PORTD |= (1<<R_MOT_B) | (1<<L_MOT_B);			
						//PORTD |= (1<<R_MOT_F) | (1<<L_MOT_F);				
						_delay_ms(100);
					}
				}
				
				if(bit_is_set(PINC,PINC0) && bit_is_set(PINC,PINC1) && bit_is_set(PINC,PINC2) && bit_is_clear(PINC,PINC3)) // otaceni vlevo
				{
					_delay_ms(10);
					if(bit_is_set(PINC,PINC0) && bit_is_set(PINC,PINC1) && bit_is_set(PINC,PINC2) && bit_is_clear(PINC,PINC3)) // otaceni vlevo
					{ 
						PORTD |= (1<<L_MOT_B) | (1<<R_MOT_F);
						//PORTD |= (1<<R_MOT_B) | (1<<L_MOT_F);
						_delay_ms(100);				
					}			
				}
				
				if(bit_is_set(PINC,PINC0) && bit_is_set(PINC,PINC1) && bit_is_clear(PINC,PINC2) && bit_is_set(PINC,PINC3)) // otaceni vpravo
				{
					_delay_ms(10);
					if(bit_is_set(PINC,PINC0) && bit_is_set(PINC,PINC1) && bit_is_clear(PINC,PINC2) && bit_is_set(PINC,PINC3)) // otaceni vpravo
					{
						PORTD |= (1<<R_MOT_B) | (1<<L_MOT_F);
						//PORTD |= (1<<L_MOT_B) | (1<<R_MOT_F);
						_delay_ms(100);				
					}			
				}
				
				
				if(bit_is_set(PINC,PINC0) && bit_is_clear(PINC,PINC1) && bit_is_clear(PINC,PINC2) && bit_is_set(PINC,PINC3)) // Funkce 1
				{
					_delay_ms(10);
					if(bit_is_set(PINC,PINC0) && bit_is_clear(PINC,PINC1) && bit_is_clear(PINC,PINC2) && bit_is_set(PINC,PINC3)) // Funkce 1
					{
						PORTD |= (1<<LED_1) | (1<<LED_2);
					}			
				}
		
				if(bit_is_set(PINC,PINC0) && bit_is_set(PINC,PINC1) && bit_is_clear(PINC,PINC2) && bit_is_clear(PINC,PINC3)) // Funkce 2
				{
					_delay_ms(10);
					if(bit_is_set(PINC,PINC0) && bit_is_set(PINC,PINC1) && bit_is_clear(PINC,PINC2) && bit_is_clear(PINC,PINC3)) // Funkce 2
					{
						PORTD |= (1<<LED_3);
					}			
				}
		
				if(bit_is_set(PINC,PINC0) && bit_is_clear(PINC,PINC1) && bit_is_set(PINC,PINC2) && bit_is_clear(PINC,PINC3)) // Funkce 3
				{
					_delay_ms(10);
					if(bit_is_set(PINC,PINC0) && bit_is_clear(PINC,PINC1) && bit_is_set(PINC,PINC2) && bit_is_clear(PINC,PINC3)) // Funkce 3
					{
						PORTD &= ~((1<<LED_1) | (1<<LED_2) | (1<<LED_3));	
					}			
				}

				if(bit_is_clear(PINC,PINC0) && bit_is_clear(PINC,PINC1) && bit_is_set(PINC,PINC2) && bit_is_set(PINC,PINC3)) // Funkce 4
				{
					_delay_ms(10);
					if(bit_is_clear(PINC,PINC0) && bit_is_clear(PINC,PINC1) && bit_is_set(PINC,PINC2) && bit_is_set(PINC,PINC3)) // Funkce 4
					{
						PORTD ^= ((1<<LED_1) | (1<<LED_2) | (1<<LED_3));
						_delay_ms(500);
					}			
				}					
			}					
		}
		
		/* Konec programu Slidila na DO */
		
		/* Začátek programu Slidil cara */
		
		if(bit_is_clear(PINB,PINB0) && bit_is_clear(PINB,PINB1) && bit_is_clear(PINB,PINB2) && bit_is_clear(PINB,PINB3)) //0000		
		{		
			while(1)
			{
				PORTD |= ((1<<LED_1) | (1<<LED_2) | (1<<LED_3));
				
				PORTD |= (1<<R_MOT_F) | (1<<L_MOT_F);
				
				if(bit_is_set(PINC,R_SENZOR)) 
				{
					PORTD &= ~(1<<L_MOT_F);
					PORTD |= (1<<L_MOT_B);
					_delay_ms(20);
					PORTD &= ~(1<<L_MOT_B);
				}
				
				if(bit_is_set(PINC, L_SENZOR))
				{
					PORTD &= ~(1<<R_MOT_F);
					PORTD |= (1<<R_MOT_B);
					_delay_ms(20);
					PORTD &= ~(1<<R_MOT_B);
				}
				
				_delay_ms(20);
			}
		}
		
		/* Konec programu Slidil cara */
		
		/* Začátek programu Slidil + otoceni */
		
		if(bit_is_set(PINB,PINB0) && bit_is_set(PINB,PINB1) && bit_is_clear(PINB,PINB2) && bit_is_clear(PINB,PINB3)) // 1100
		{
			
			while(1)
			{						
				PORTD |= ((1<<LED_1) | (1<<LED_2) | (1<<LED_3));
							
				if(bit_is_clear(PINB, IR_SENSOR))
				{
					_delay_ms(10);
					if(bit_is_clear(PINB, IR_SENSOR))
					{
						PORTD &= ~((1<<R_MOT_F) | (1<<L_MOT_F));
						PORTD |= (1<<R_MOT_B) | (1<<L_MOT_B);
						
						while(1)
						{
							if(bit_is_set(PINC,R_SENZOR) || bit_is_set(PINC,L_SENZOR)) 
							{
								PORTD &= ~((1<<R_MOT_F) | (1<<R_MOT_B) | (1<<L_MOT_F) | (1<<L_MOT_B));
								goto OK;
							}
						}
					}			
				}
								
				OK:
				PORTD |= (1<<R_MOT_F) | (1<<L_MOT_F); 
													
				if(bit_is_set(PINC,R_SENZOR)) 
				{
					PORTD &= ~(1<<L_MOT_F);
					PORTD |= (1<<L_MOT_B);
					_delay_ms(20);
					PORTD &= ~(1<<L_MOT_B);
				}
				
				if(bit_is_set(PINC, L_SENZOR))
				{
					PORTD &= ~(1<<R_MOT_F);
					PORTD |= (1<<R_MOT_B);
					_delay_ms(20);
					PORTD &= ~(1<<R_MOT_B);
				}
				
				_delay_ms(20);	
			}
		}
		
		/* Konec programu Slidil + otoceni */
		
		/*Začátek programu Slidil + cekani */
				
		if(bit_is_clear(PINB,PINB0) && bit_is_clear(PINB,PINB1) && bit_is_set(PINB,PINB2) && bit_is_set(PINB,PINB3)) // 0011
		{
			while(1)
			{						
				PORTD |= ((1<<LED_1) | (1<<LED_2) | (1<<LED_3));
								
				BACK: 			
				if(bit_is_clear(PINB, IR_SENSOR))
				{
					_delay_ms(10);
					if(bit_is_clear(PINB, IR_SENSOR))
					{
						PORTD &= ~((1<<R_MOT_F) | (1<<R_MOT_B) | (1<<L_MOT_F) | (1<<L_MOT_B));
						
						PORTD ^= ((1<<LED_1) | (1<<LED_2) | (1<<LED_3));
						_delay_ms(200);
						goto BACK;
					}			
				}
				
				PORTD |= (1<<R_MOT_F) | (1<<L_MOT_F); 
													
				if(bit_is_set(PINC,R_SENZOR)) 
				{
					PORTD &= ~(1<<L_MOT_F);
					PORTD |= (1<<L_MOT_B);
					_delay_ms(20);
					PORTD &= ~(1<<L_MOT_B);
				}
				
				if(bit_is_set(PINC, L_SENZOR))
				{
					PORTD &= ~(1<<R_MOT_F);
					PORTD |= (1<<R_MOT_B);
					_delay_ms(20);
					PORTD &= ~(1<<R_MOT_B);
				}
				
				_delay_ms(20);	
			}
		}
		
		/* Konec programu Slidil + cekani */
}		

volatile unsigned char x = 0;

ISR(TIMER2_OVF_vect) 
{
	TCNT2 = 178;
	
	if(x==0)
	{
		PORTB |= (1<<IR_LED);
		x = 1;
		goto EX;	
	}
		
	if(x==1)
	{
		PORTB &= ~(1<<IR_LED);
		x = 0;	
	}
		
	EX: ;
}


void beep(int time)
{
int a;

 for(a=0;a<500;a++)
 {
  _delay_us(time/2);
  PORTD &= ~(1<<R_MOT_B);
  _delay_us(time/2);
  PORTD |= (1<<R_MOT_B);
 }
}


void set_motor(int left, int right)
{
  if (left < 0)
    { pwm_value[0]=0; pwm_value[1]=-right; }
  else 
    { pwm_value[0]=right; pwm_value[1]=0; }

if (right < 0)
    { pwm_value[3]=0; pwm_value[2]=-left; }
  else 
    { pwm_value[3]=left; pwm_value[2]=0; }
 pwm_update();
}
