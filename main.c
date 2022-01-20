#include <msp430x14x.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "lcd.h"
#include "portyLcd.h"
#include "notes.h"

#define BUTTON1 BIT4&P4IN
#define BUTTON2 BIT5&P4IN
#define BUTTON3 BIT6&P4IN
#define BUTTON4 BIT7&P4IN
//---------------- zmienne globalne -------------

unsigned int i = 0;
unsigned int counter = 0;
int option = 0;
int highScorePoints = 0;
int pos = 0;
int playerA = 0, playerB = 0;

int A[2][14];
int Pion = 2, Poziom = 14;
int newFace = 0, sadFaceOnBoard = 0, checked, kliknietoA, kliknietoB;

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

void Clock(void);

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

void init_chars() {
	create_chars();
	//create_chars(f0);
	//create_chars(f1);
	//create_chars(f2);
	//create_chars(f3);
	//create_chars(f4);
	//create_chars(f5);
	//
}

void write_text(unsigned char *text) {
	for (int i = 0; i < strlen(text); i++) {
		SEND_CHAR(text[i]);
	}
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

void liczba(int x) {
	if (x >= 10) {
		liczba(x / 10);
	}
	int digit = x % 10 + 48;
	SEND_CHAR(digit);
}

/*
int wypisz(char A[2][16])
{
	for(int i=0;i<2;i++)
		for(int j=0;j<16;j++)
		  if(A[i][j])
		  swich(A[i][j])
		  {
		  case1:SEND_CHAR(f4);
		  }
			
}*/

int randomNum() {
	srand(time(0));
	int num = (rand() %
			   (12 - 3 + 1)) +
			  3;

	return num;
}

void melody(int czestotliwosc, int k) {
	int n, i;

	for (i = 0; i < k; i++) {
		P4OUT |= 0x08;
		P4OUT &= ~0x04;
		for (n = 0; n < czestotliwosc; n++);
		P4OUT |= 0x04;
		P4OUT &= ~0x08;
		for (n = 0; n < czestotliwosc; n++);
		__delay_cycles(200);
	}
}


void levelScreen(int level) {
	write_text("LEVEL ");
	liczba(level);

	for (long i = 0; i < 10000; i++);
}

//==============================GAME=====================================

void game(void) {
	P1OUT = P1OUT & ~BIT5;
	int rPos = 0;      // rabbit
	int wPos = randomNum();      // wolf
	int rate = 60;     // tempo
	int direction = 1; // kierunek
	int level = 1;

	int points = 1000;
	int totalPoints = 0;
	int btnPressed = 0;


	while (1) {
		if ((P4IN & BIT6) == 0 && btnPressed == 0) {
			btnPressed = 1; // zeby punkty dobrze liczylo i nie wchodzilo tu kilka razy

			P1OUT = P1OUT | BIT5;
			clearDisplay();

			if ((direction == 1 && rPos - 1 == wPos) || (direction == -1 && rPos + 1 == wPos)) {
				level++;
				levelScreen(level);

				if (rate != 1)
					rate -= 3;

				rPos = 0;
				wPos = randomNum();
				direction = 1;
				totalPoints += points;
				points = 1000;
				btnPressed = 0;
			} else {
				write_text("GAME OVER");
				SEND_CMD(DD_RAM_ADDR2);
				liczba(totalPoints);
				for (long i = 0; i < 2000000; i++);
				//clearDisplay();
				if (totalPoints > highScorePoints) {
					highScorePoints = totalPoints;
				}
				break;
			}
		}

		if (btnPressed == 0) {
			SEND_CMD(DD_RAM_ADDR2 + wPos);
			SEND_CHAR(5); // wolf
		}


		if (pos % rate == 0 && btnPressed == 0) {
			pos = 0;
			SEND_CMD(CUR_HOME);
			SEND_CMD(DD_RAM_ADDR + rPos);

			// czyszczenie poprzedniej pozycji i ruch
			if (direction == 1) {
				SEND_CHAR(4);

				if (rPos != 0) {
					SEND_CMD(DD_RAM_ADDR + rPos - 1);
					SEND_CHAR(' ');
				}
			} else if (direction == -1) {
				SEND_CHAR(3);

				if (rPos != 15) {
					SEND_CMD(DD_RAM_ADDR + rPos + 1);
					SEND_CHAR(' ');
				}
			}

			// zmiana ruchu
			if ((rPos == 15 && direction == 1) || (rPos == 0 && direction == -1))
				direction = direction * (-1);

			if (direction == 1)
				rPos++;
			else
				rPos--;

			// obsluga punktow
			if (points != 100)
				if ((direction == 1 && rPos - 2 == wPos) || (direction == -1 && rPos + 2 == wPos))
					points -= 100;
		}
	}
}

void desc(void) {
	write_text("Zlap zajaca jak");
	SEND_CMD(DD_RAM_ADDR2);
	write_text("najszybciej!");
	for (long i = 0; i < 4000000; i++);
	clearDisplay();
	SEND_CMD(DD_RAM_ADDR);
	write_text("najszybciej!");
	SEND_CMD(DD_RAM_ADDR2);
	write_text("Uwaga! Z czasem");
	for (long i = 0; i < 4000000; i++);
	clearDisplay();
	SEND_CMD(DD_RAM_ADDR);
	write_text("bedzie szybszy");
	for (long i = 0; i < 4000000; i++);
}

void authors() {

	SEND_CMD(DD_RAM_ADDR);
	write_text("Jedrzej");
	SEND_CMD(DD_RAM_ADDR2);
	write_text("Dudzicz");
	for (long i = 0; i < 3000000; i++);
	clearDisplay();
	SEND_CMD(DD_RAM_ADDR);
	write_text("Julita");
	SEND_CMD(DD_RAM_ADDR2);
	write_text("Gajewska");
	for (long i = 0; i < 3000000; i++);
	clearDisplay();
	SEND_CMD(DD_RAM_ADDR);
	write_text("Michal");
	SEND_CMD(DD_RAM_ADDR2);
	write_text("Dzienisik");
	for (long i = 0; i < 3000000; i++);
	clearDisplay();
	SEND_CMD(DD_RAM_ADDR);
	write_text("Filip");
	SEND_CMD(DD_RAM_ADDR2);
	write_text("Drazba");
	for (long i = 0; i < 3000000; i++);
	clearDisplay();

}

void highscore(void) {
	SEND_CMD(DD_RAM_ADDR);
	liczba(highScorePoints);
	for (long i = 0; i < 3000000; i++);
	clearDisplay();
}


void menu(void) {

	switch (option) {
		case 0:
			SEND_CMD(DD_RAM_ADDR);
			write_text("    > GAME <      ");
			SEND_CMD(DD_RAM_ADDR2);
			write_text("      DESC      ");
			break;

		case 1:
			SEND_CMD(DD_RAM_ADDR);
			write_text("      GAME      ");
			SEND_CMD(DD_RAM_ADDR2);
			write_text("    > DESC <    ");
			break;

		case 2:
			SEND_CMD(DD_RAM_ADDR);
			write_text("      DESC      ");
			SEND_CMD(DD_RAM_ADDR2);
			write_text("   > AUTHORS <  ");
			break;

		case 3:
			SEND_CMD(DD_RAM_ADDR);
			write_text("     AUTHORS    ");
			SEND_CMD(DD_RAM_ADDR2);
			write_text("  > HIGHSCORE < ");
			break;
	}

	if ((P4IN & BIT7) == 0) {
		clearDisplay();


		switch (option) {
			case 0:
				game();
				option = 0;
				break;

			case 1:
				desc();
				option = 1;
				break;

			case 2:
				authors();
				option = 2;
				break;

			case 3:
				highscore();
				option = 3;
				break;
		}

	} else if ((P4IN & BIT5) == 0) // down
	{
		if (counter % 5 == 0) {
			counter = 0;
			if (option != 3)
				option++;

		}
	} else if ((P4IN & BIT4) == 0) // up
	{
		if (counter % 5 == 0) {
			counter = 0;
			if (option != 0)
				option--;

		}
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

void playerAwygral() {
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
	SEND_CHAR(' ');
	SEND_CHAR('A');
	SEND_CHAR('!');
	playerA = 0;
	playerB = 0;
	for (int i = 1000; i > 0; i--)
		for (int j = 1000; j > 0; j--);
	resetPlanszy();
}


void playerBwygral() {
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
	SEND_CHAR(' ');
	SEND_CHAR('B');
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
		playerAwygral();
	else if (playerB == limit)
		playerBwygral();
	else if (playerA == -limit)
		playerBwygral();
	else if (playerB == -limit)
		playerAwygral();


}

//----------------- main program -------------------
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
			if (rand() % 12)
				newFace = rand() % 4;
			else
				newFace = rand() % 4 + 4;

			A[rand() % Pion][rand() % Poziom] = newFace;
			sadFaceOnBoard += newFace > 3;

			pokazPlansze();

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
