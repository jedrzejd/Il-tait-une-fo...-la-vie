#include <msp430x14x.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "lcd.h"
#include "portyLcd.h"

#define BUTTON1 BIT4&P4IN
#define BUTTON2 BIT5&P4IN
#define BUTTON3 BIT6&P4IN
#define BUTTON4 BIT7&P4IN

int playerA = 0,
		playerB = 0,
		A[2][14],
		Pion = 2,
		Poziom = 14,
		newFace = 0,
		sadFaceOnBoard = 0,
		checked,
		kliknietoA,
		kliknietoB;

///c:
char f0[] = {0x00, 0x00, 0x0A, 0x00, 0x11, 0x0E, 0x00, 0x00};
char f1[] = {0x00, 0x00, 0x02, 0x09, 0x01, 0x09, 0x02, 0x00};
char f2[] = {0x00, 0x00, 0x0E, 0x11, 0x00, 0x0A, 0x00, 0x00};
char f3[] = {0x00, 0x00, 0x08, 0x12, 0x10, 0x12, 0x08, 0x00};
///:c
char f4[] = {0x00, 0x00, 0x00, 0x0A, 0x00, 0x0E, 0x11, 0x00};
char f5[] = {0x00, 0x00, 0x02, 0x14, 0x04, 0x14, 0x02, 0x00};
char f6[] = {0x00, 0x00, 0x11, 0x0E, 0x00, 0x0A, 0x00, 0x00};
char f7[] = {0x00, 0x00, 0x08, 0x05, 0x04, 0x05, 0x08, 0x00};


void create_chars() {
	SEND_CMD(CG_RAM_ADDR);

	for (int i = 0; i < 8; i++)
		SEND_CHAR(f0[i]);

	for (int i = 0; i < 8; i++)
		SEND_CHAR(f1[i]);

	for (int i = 0; i < 8; i++)
		SEND_CHAR(f2[i]);

	for (int i = 0; i < 8; i++)
		SEND_CHAR(f3[i]);


	for (int i = 0; i < 8; i++)
		SEND_CHAR(f4[i]);

	for (int i = 0; i < 8; i++)
		SEND_CHAR(f5[i]);

	for (int i = 0; i < 8; i++)
		SEND_CHAR(f6[i]);

	for (int i = 0; i < 8; i++)
		SEND_CHAR(f7[i]);

	SEND_CMD(DD_RAM_ADDR);
}

void display(unsigned char *text) {
	clearDisplay();
	SEND_CMD(DD_RAM_ADDR);
	write_text(text);
	Delayx100us(250);
	for (int i = 0; i < strlen(text); i++) {
		SEND_CMD(DATA_ROL_LEFT);
		Delayx100us(250);
	}
}

void sound(int freq, int dur) {
	for (i = 0; i < dur; i++) {
		P4OUT |= 0x08;
		P4OUT &= ~0x04;
		for (int n = 0; n < freq; n++);
		P4OUT |= 0x04;
		P4OUT &= ~0x08;
	}
}

void SEND_INT(unsigned int i) {
	SEND_CHAR(i + 48);
}

void pokazPlansze() {
	SEND_CMD(DD_RAM_ADDR);
	if (playerA >= 0)
		SEND_CHAR('A');
	else
		SEND_CHAR('-');

	for (int p = 0; p < Poziom; p++)
		SEND_CHAR(A[0][p]);

	if (playerB >= 0)
		SEND_CHAR('B');
	else
		SEND_CHAR('-');

	SEND_CMD(DD_RAM_ADDR2);
	if (playerA >= 0)
		SEND_INT(playerA);
	else
		SEND_INT(-playerA);

	for (int p = 0; p < Poziom; p++)
		SEND_CHAR(A[1][p]);

	if (playerB >= 0)
		SEND_INT(playerB);
	else
		SEND_INT(-playerB);
}

void resetPlanszy() {
	for (int i = 0; i < Pion; i++)
		for (int j = 0; j < Poziom; j++)
			A[i][j] = rand() % 4;
	sadFaceOnBoard = 0;
	pokazPlansze();
}

void wygrana(char playerName) {
	clearDisplay();
	SEND_CHAR('W');
	SEND_CHAR('y');
	SEND_CHAR('g');
	SEND_CHAR('r');
	SEND_CHAR('a');
	SEND_CHAR('l');
	SEND_CHAR(' ');
	SEND_CHAR('g');
	SEND_CHAR('r');
	SEND_CHAR('a');
	SEND_CHAR('c');
	SEND_CHAR('z');
	SEND_CHAR(playerName);
	SEND_CHAR('A');
	SEND_CHAR('!');
	playerA = 0;
	playerB = 0;
	for (int i = 1000; i > 0; i--)
		for (int j = 1000; j > 0; j--);
	resetPlanszy();
}


void czyKtosWygral() {
	int limit = 2;
	if (playerA == limit)
		wygrana('A');
	else if (playerB == limit)
		wygrana('B');
	else if (playerA == -limit)
		wygrana('B');
	else if (playerB == -limit)
		wygrana('A');


}

void main(void) {

	P2DIR |= BIT1;
	P4DIR |= BIT2;
	P4DIR |= BIT3;
	P4DIR &= ~BIT4;
	P4DIR &= ~BIT5;
	P4DIR &= ~BIT6;
	P4DIR &= ~BIT7;

	WDTCTL = WDTPW + WDTHOLD;
	srand(time(NULL));
	InitPortsLcd();
	InitLCD();
	clearDisplay();

	init_chars();

	BCSCTL1 |= XTS;

	resetPlanszy();
	int y = 100, n = 900, t = 250;
	while (1) {
		for (int odstepCzasowy = 100; odstepCzasowy > 0;) {
			if (rand() % 20)
				newFace = rand() % 4;
			else
				newFace = rand() % 4 + 4;

			A[rand() % Pion][rand() % Poziom] = newFace;
			sadFaceOnBoard += newFace > 3;
			P2OUT ^= BIT1;
			pokazPlansze();
			P2OUT ^= BIT1;
			checked = 0;

			kliknietoA = 0;
			kliknietoB = 0;
			for (int j = 0; j < odstepCzasowy; j++)
				for (int i = 0; i < odstepCzasowy; i++) {
					if (!checked) {
						if (!(BUTTON1)) {
							kliknietoA = 1;
							while (!(BUTTON1));
						} else if (!(BUTTON4)) {
							kliknietoB = 1;
							while (!(BUTTON4));
						}
						if (kliknietoA) {
							odstepCzasowy -= 5;
							if (sadFaceOnBoard) {
								playerA++;
								resetPlanszy();
								sound(y, t);
							} else {
								playerA--;
								resetPlanszy();
								sound(n, t);
							}
							czyKtosWygral();
							checked = 1;
						} else if (kliknietoB) {
							odstepCzasowy -= 5;

							while (!(BUTTON4));
							if (sadFaceOnBoard) {
								playerB++;
								resetPlanszy();
								sound(y, t);
							} else {
								playerB--;
								resetPlanszy();
								sound(n, t);
							}
							czyKtosWygral();
							checked = 1;
						}
					}
				}
		}
		resetPlanszy();
	}
	SEND_CMD(DD_RAM_ADDR);
}

#pragma vector = TIMERA0_VECTOR

__interrupt void Timer_A(void) {
	_BIC_SR_IRQ(LPM3_bits);

	pos++;
	pos = pos % 100;
}
