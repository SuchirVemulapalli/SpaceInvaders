// SpaceInvaders.c
// Runs on TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10
//Suchir Vemulapalli and James Cheshire

// Last Modified: 1/12/2022 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php

// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// buttons connected to PE0-PE3
// 32*R resistor DAC bit 0 on PB0 (least significant bit)
// 16*R resistor DAC bit 1 on PB1
// 8*R resistor DAC bit 2 on PB2 
// 4*R resistor DAC bit 3 on PB3
// 2*R resistor DAC bit 4 on PB4
// 1*R resistor DAC bit 5 on PB5 (most significant bit)
// LED on PD1
// LED on PD0


#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "Print.h"
#include "Random.h"
#include "TExaS.h"
#include "ADC.h"
#include "DAC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer1.h"

uint32_t ADCsample;
uint32_t score = 0;
uint32_t trigger = 0;

uint32_t shotFired = 0;
uint32_t bulletX;
uint32_t bulletY  = 150;
uint32_t pause = 0;
uint32_t bulletDraw = 0;
uint32_t collision = 0;
uint32_t shipsKilled = 0;
uint32_t english = 0;
uint32_t spanish = 0;
uint32_t enemies[6];
uint32_t yCursor = 150;

uint32_t SE10A_X = 0;
uint32_t SE10B_X = 20;
uint32_t SE20A_X = 40;
uint32_t SE20B_X = 60;
uint32_t SE30A_X = 80;
uint32_t SE30B_X = 100;
uint32_t enemyRow = 10;

uint32_t SE10A_hits = 0;
uint32_t SE10B_hits = 0;
uint32_t SE20A_hits = 0;
uint32_t SE20B_hits = 0;
uint32_t SE30A_hits = 0;
uint32_t SE30B_hits = 0;

void SysTick_Init(uint32_t period){
  NVIC_ST_CTRL_R = 0;         									// disable SysTick during setup
  NVIC_ST_RELOAD_R = period - 1;									// reload value
  NVIC_ST_CURRENT_R = 0;     									  // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0x60000000;
  NVIC_ST_CTRL_R = 0x00000007;
}

void SysTick_Handler(void){
	//check slide pot
	//check if bullets have hit anything
	ADCsample = ADC_In();
	if(GPIO_PORTD_DATA_R == 0x02){
		trigger=1;
	}
	if((GPIO_PORTD_DATA_R == 0x01)||(GPIO_PORTD_DATA_R == 0x03)){
		pause = 1;
	}
	if((bulletY)== enemyRow){
		if((bulletX > 1) && (bulletX < 14) && (enemies[0]!=0)){
			collision = 1;
		}
		if((bulletX > 21) && (bulletX < 34) && (enemies[1]!=0)){
			collision = 1;
		}
		if((bulletX > 41) && (bulletX < 54) && (enemies[2]!=0)){
			collision = 1;
		}
		if((bulletX > 61) && (bulletX < 74) && (enemies[3]!=0)){
			collision = 1;
		}
		if((bulletX > 81) && (bulletX < 94) && (enemies[4]!=0)){
			collision = 1;
		}
		if((bulletX > 101) && (bulletX < 114) && (enemies[5]!=0)){
			collision = 1;
		}
	}
}

void PortD_Init(void){
  SYSCTL_RCGCGPIO_R |=0x08; //turn on port C
		__nop();
		__nop();
		GPIO_PORTD_DIR_R &= ~(0x03);
		GPIO_PORTD_DEN_R |= 0x03; 
}

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds

void Timer1A_Handler(void){ // can be used to perform tasks in background
  TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER1A timeout
   // move sprites down
	enemyRow+=1;
}

int main(void){
  DisableInterrupts();
  TExaS_Init(NONE);       // Bus clock is 80 MHz 
  Random_Init(1);
	SysTick_Init(2000000);
  Output_Init();
	ADC_Init();
	DAC_Init();
	PortD_Init();
	Timer1_Init(40000000, 5);
	for(uint8_t i = 0; i < 6; i++){
		enemies[i] = 1;
	}
	ST7735_FillScreen(0x0000);
	while(GPIO_PORTD_DATA_R == 0){
		ST7735_SetCursor(1, 1);
		ST7735_OutString("SELECT LANGUAGE");
		ST7735_SetCursor(1, 2);
		ST7735_OutString("1) English");
		ST7735_SetCursor(1, 3);
		ST7735_OutString("2) Spanish");
		if(GPIO_PORTD_DATA_R == 0x02){
			english = 1;
		}
		if(GPIO_PORTD_DATA_R == 0x01){
			spanish = 1;
		}
	}
	ST7735_FillScreen(0x0000);
	ST7735_DrawBitmap(SE10A_X, enemyRow, SmallEnemy10pointA, 16,10);
	ST7735_DrawBitmap(SE10B_X, enemyRow, SmallEnemy10pointB, 16,10);
	ST7735_DrawBitmap(SE20A_X, enemyRow, SmallEnemy20pointA, 16,10);
	ST7735_DrawBitmap(SE20B_X, enemyRow, SmallEnemy20pointB, 16,10);
	ST7735_DrawBitmap(SE30A_X, enemyRow, SmallEnemy30pointA, 16,10);
	ST7735_DrawBitmap(SE30B_X, enemyRow, SmallEnemy30pointB, 16,10);
	EnableInterrupts();
	while(1){
		if(SE10A_hits < 1){ST7735_DrawBitmap(SE10A_X, enemyRow, SmallEnemy10pointA, 16,10);}
		if(SE10B_hits < 1){ST7735_DrawBitmap(SE10B_X, enemyRow, SmallEnemy10pointB, 16,10);}
		if(SE20A_hits < 1){ST7735_DrawBitmap(SE20A_X, enemyRow, SmallEnemy20pointA, 16,10);}
		if(SE20B_hits < 1){ST7735_DrawBitmap(SE20B_X, enemyRow, SmallEnemy20pointB, 16,10);}
		if(SE30A_hits < 1){ST7735_DrawBitmap(SE30A_X, enemyRow, SmallEnemy30pointA, 16,10);}
		if(SE30B_hits < 1){ST7735_DrawBitmap(SE30B_X, enemyRow, SmallEnemy30pointB, 16,10);}
		
		if(SE10A_hits >= 1){ST7735_DrawBitmap(SE10A_X, enemyRow, Explode10A, 16,10);}
		if(SE10B_hits >= 1){ST7735_DrawBitmap(SE10B_X, enemyRow, Explode10B, 16,10);}
		if(SE20A_hits >= 1){ST7735_DrawBitmap(SE20A_X, enemyRow, Explode20A, 16,10);}
		if(SE20B_hits >= 1){ST7735_DrawBitmap(SE20B_X, enemyRow, Explode20B, 16,10);}
		if(SE30A_hits >= 1){ST7735_DrawBitmap(SE30A_X, enemyRow, Explode30A, 16,10);}
		if(SE30B_hits >= 1){ST7735_DrawBitmap(SE30B_X, enemyRow, Explode30B, 16,10);}
		
		ST7735_DrawBitmap((ADCsample/32), 159, PlayerShip0, 20, 8);
		if((trigger == 1)&&(shotFired != 1)){		//display new bullet, clear trigger flag
			bulletX = (ADCsample/32)+9;
			ST7735_DrawBitmap(bulletX, bulletY, Bullet, 2, 8);
			trigger = 0;
			shotFired = 1;
			//play shooting sound
			Sound_Shoot();
		}
		if(shotFired == 1){	//shift bullet up screen
			bulletDraw++;
			trigger=0;
			if(bulletDraw%20 == 0 ){
				ST7735_DrawBitmap(bulletX, bulletY, Bullet, 2, 8);
				bulletY-=1;
			}
			if(bulletY == 10){
				ST7735_DrawBitmap(bulletX, bulletY, blackBullet, 2, 8);
				bulletY = 150;
				shotFired = 0;
			}
		}
		//check for loss
		if(enemyRow == 150){
			DisableInterrupts();
			ST7735_FillScreen(0x0000);   // set screen to black
			if(english == 1){
				 ST7735_FillScreen(0x0000);   // set screen to black
					ST7735_SetCursor(1, 1);
					ST7735_OutString("GAME OVER");
					ST7735_SetCursor(1, 2);
					ST7735_OutString("YOU LOSE");
					ST7735_SetCursor(1, 3);
					ST7735_OutString("SCORE: ");
					//ST7735_SetCursor(1, 4);	//change x
					ST7735_OutUDec(score);
					ST7735_SetCursor(1, 10);
					ST7735_OutString("OU SUCKS");
					while(1){
					}
			}
			
			if(spanish == 1){
				 ST7735_FillScreen(0x0000);   // set screen to black
					ST7735_SetCursor(1, 1);
					ST7735_OutString("JUEGO TERMINADO");
					ST7735_SetCursor(1, 2);
					ST7735_OutString("USTED PIERDE");
					ST7735_SetCursor(1, 3);
				  ST7735_OutString("PUNTAJE: ");
					//ST7735_SetCursor(1, 4);	//change x
					ST7735_OutUDec(score);
					ST7735_SetCursor(1, 10);
					ST7735_OutString("OU SUCKS");
					while(1){
					}
			}
		
		}
		
		if(collision == 1){
			DisableInterrupts();
			collision = 0;
			shotFired = 0;
			trigger = 0;
			ST7735_DrawBitmap(bulletX, bulletY, blackBullet, 2, 8);
			bulletY = 150;
			Sound_Explosion();
		
			//greater than 1, less than 14 equals hit on 10 point A
			if((bulletX > 1) && (bulletX < 14) && (SE10A_hits < 1)){
				SE10A_hits++;
				if(SE10A_hits == 1){
					shipsKilled++;
					score+=10;
					//explosion sounds and graphics?
					ST7735_DrawBitmap(SE10A_X, enemyRow, Explode10A, 16, 10);
					//ST7735_DrawBitmap(SE10A_X, enemyRow, BlackEnemy, 16, 10);
				}
				else{
					//play hitmarker sound
				}
			}
			//greater than 21, less than 34 equals hit on 10 point B
			if((bulletX > 21) && (bulletX < 34) && (SE10B_hits < 1)){
				SE10B_hits++;
				if(SE10B_hits == 1){
					shipsKilled++;
					score+=10;
					//explosion sounds and graphics?
					ST7735_DrawBitmap(SE10B_X, enemyRow, Explode10B, 16, 10);
				}
				else{
					//play hitmarker sound
				}
			}
			if((bulletX > 41) && (bulletX < 54) && (SE20A_hits < 1)){
				SE20A_hits++;
				if(SE20A_hits == 1){
					shipsKilled++;
					score+=20;
					//explosion sounds and graphics?
					ST7735_DrawBitmap(SE20A_X, enemyRow, Explode20A, 16, 10);
				}
				else{
					//play hitmarker sound
				}
			}
			if((bulletX > 61) && (bulletX < 74) && (SE20B_hits < 1)){
				SE20B_hits++;
				if(SE20B_hits == 1){
					shipsKilled++;
					score+=20;
					//explosion sounds and graphics?
					ST7735_DrawBitmap(SE20B_X, enemyRow, Explode20B, 16, 10);
				}
				else{
					//play hitmarker sound
				}
			}
			if((bulletX > 81) && (bulletX < 94) && (SE30A_hits < 1)){
				SE30A_hits++;
				if(SE30A_hits == 1){
					shipsKilled++;
					score+=30;
					//explosion sounds and graphics?
					ST7735_DrawBitmap(SE30A_X, enemyRow, Explode30A, 16, 10);
				}
				else{
					//play hitmarker sound
				}
			}
			if((bulletX > 101) && (bulletX < 114) && (SE30B_hits < 1)){
				SE30B_hits++;
				if(SE30B_hits == 1){
					shipsKilled++;
					score+=30;
					//explosion sound
					ST7735_DrawBitmap(SE30B_X, enemyRow, Explode30B, 16, 10);
				}
				else{
					//hitmarker
				}
			}
			EnableInterrupts();
		}
		while(shipsKilled == 6){
				DisableInterrupts();
				ST7735_FillScreen(0xEFB3);
			if(english == 1){
					ST7735_DrawCharS(1, 1, 'Y', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(15, 1, 'O', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(29, 1, 'U', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(57, 1, 'W', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(71, 1, 'I', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(85, 1, 'N', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(99, 1, '!', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(1, 20, 'S', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(15, 20, 'C', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(29, 20, 'O', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(43, 20, 'R', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(57, 20, 'E', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(71, 20, ':', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(1, 40, '1', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(15, 40, '2', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(29, 40, '0', 0x0000, 0xEFB3, 2);
				while(1){}	
			}
			if(spanish == 1){
				while(1){
					ST7735_DrawCharS(1, 1, 'U', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(15, 1, 'S', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(29, 1, 'T', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(43, 1, 'E', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(57, 1, 'D', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(1, 20, 'G', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(15, 20, 'A', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(29, 20, 'N', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(43, 20, 'A', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(57, 20, '!', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(1, 40, 'P', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(15, 40, 'U', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(29, 40, 'N', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(43, 40, 'T', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(57, 40, 'A', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(71, 40, 'J', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(85, 40, 'E', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(99, 40, ':', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(1, 60, '1', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(15, 60, '2', 0x0000, 0xEFB3, 2);
					ST7735_DrawCharS(29, 60, '0', 0x0000, 0xEFB3, 2);
				}	
			}
		}
		//while(pause is pressed){display pause screen which has score};
		while(pause == 1){
			DisableInterrupts();
			ST7735_DrawBitmap((ADCsample/32), 159, BlackShip, 20, 8);
			//print pause text and score on bottom
			if((GPIO_PORTD_DATA_R != 0x01)&&(GPIO_PORTD_DATA_R != 0x03)){
				pause = 0;
				EnableInterrupts();
			}
		} 
  }
}


typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};

/*int main1(void){ char l;
  DisableInterrupts();
  TExaS_Init(NONE);       // Bus clock is 80 MHz 
  Output_Init();
  ST7735_FillScreen(0x0000);            // set screen to black
  for(phrase_t myPhrase=HELLO; myPhrase<= GOODBYE; myPhrase++){
    for(Language_t myL=English; myL<= French; myL++){
         ST7735_OutString((char *)Phrases[LANGUAGE][myL]);
      ST7735_OutChar(' ');
         ST7735_OutString((char *)Phrases[myPhrase][myL]);
      ST7735_OutChar(13);
    }
  }
  Delay100ms(30);
  ST7735_FillScreen(0x0000);       // set screen to black
  l = 128;
  while(1){
    Delay100ms(20);
    for(int j=0; j < 3; j++){
      for(int i=0;i<16;i++){
        ST7735_SetCursor(7*j+0,i);
        ST7735_OutUDec(l);
        ST7735_OutChar(' ');
        ST7735_OutChar(' ');
        ST7735_SetCursor(7*j+4,i);
        ST7735_OutChar(l);
        l++;
      }
    }
  }  
}*/




