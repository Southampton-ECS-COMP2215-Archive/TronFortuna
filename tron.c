#include "switches.h"
#include "lcd/lcd.h"
#include "color.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h> 

#define LED_ON      PORTB |=  _BV(PINB7)
#define LED_OFF     PORTB &= ~_BV(PINB7) 
#define LED_TOGGLE  PINB  |=  _BV(PINB7)

volatile int score = 0;

typedef struct {
	uint16_t x, y;
	uint16_t dir, len;
} line;	

uint16_t hightscore = 0;
uint16_t line_count = 0;
line tron_lines[3000];

uint16_t lives = 3;
volatile uint16_t players = 4;

uint16_t player_x[4];
uint16_t player_y[4];
uint16_t player_lastturn_x[4];
uint16_t player_lastturn_y[4];
volatile uint16_t player_dir[4];
volatile uint16_t player_collision[4];
volatile uint16_t player_line[4];
uint16_t player_color[4] = {RED, BLUE, GREEN, YELLOW};

typedef enum {Up,Down,Left,Right} direction;

void set_default_pos() {
	if(players == 2) {
		player_x[0] = 160;
		player_y[0] = 108;
		player_dir[0] = 0;
		
		player_x[1] = 160;
		player_y[1] = 132;
		player_dir[1] = 2;
		
	} else if (players == 3) {
		player_x[0] = 160;
		player_y[0] = 108;
		player_dir[0] = 0;
		
		player_x[1] = 148;
		player_y[1] = 132;
		player_dir[1] = 3;
		
		player_x[2] = 172;
		player_y[2] = 132;
		player_dir[2] = 1;
	} else if (players == 4) {
		player_x[0] = 148;
		player_y[0] = 108;
		player_dir[0] = 0;
		
		player_x[1] = 172;
		player_y[1] = 108;
		player_dir[1] = 0;
		
		player_x[2] = 148;
		player_y[2] = 132;
		player_dir[2] = 2;
		
		player_x[3] = 172;
		player_y[3] = 132;
		player_dir[3] = 2;
	}
	
	int i;
	for(i = 0; i < players; i++) {
		player_lastturn_x[i] = player_x[i];
		player_lastturn_y[i] = player_y[i];
		player_collision[i] = 0;
		
		line l = {player_x[i] , player_y[i] , player_dir[i], 0};
		tron_lines[line_count] = l;
		player_line[i] = line_count;
		line_count++;
	}
	
	
		
}
void redraw(){

	char points[15];
	sprintf(points, "Lives: %d", lives);
	display_string_xy(points,320/4,10);
	sprintf(points, "Score: %d", score);
	display_string_xy(points,320/4,20);
	sprintf(points, "Highscore: %d", hightscore);
	display_string_xy(points,320/2,10);
	
	/*
	sprintf(points, "Debug");
	display_string_xy(points,270,0);
	sprintf(points, "%d %d", p1_x, p1_y);
	display_string_xy(points,270,10);
	sprintf(points, "%d %d", p2_x, p2_y);
	display_string_xy(points,270,20);*/
	

		
		int plr, i;
		if(player_collision[0] != 1) {
			for(plr = 0; plr < players; plr++) {
				if(player_collision[plr] == 1)
					continue;
				
				for(i = 0; i < line_count; i++) {
					if(player_line[plr] == i)
						continue;
					
					if(tron_lines[i].dir == 0) {
						if(player_y[plr] <= tron_lines[i].y && player_y[plr] >= tron_lines[i].y - tron_lines[i].len && player_x[plr] == tron_lines[i].x) {
							player_collision[plr] = 1;
							break;
						}
					}
					else if(tron_lines[i].dir == 1) {
						if(player_x[plr] >= tron_lines[i].x && player_x[plr] <= tron_lines[i].x + tron_lines[i].len && player_y[plr] == tron_lines[i].y) {
							player_collision[plr] = 1;
							break;
						}
					}
					else if(tron_lines[i].dir == 2) {
						if(player_y[plr] >= tron_lines[i].y && player_y[plr] <= tron_lines[i].y + tron_lines[i].len && player_x[plr] == tron_lines[i].x) {
							player_collision[plr] = 1;
							break;
						}
					}
					else if(tron_lines[i].dir == 3) {
						if(player_x[plr] <= tron_lines[i].x && player_x[plr] >= tron_lines[i].x - tron_lines[i].len && player_y[plr] == tron_lines[i].y) {
							player_collision[plr] = 1;
							break;
						}
					}
				}
					
				if(player_x[plr] <= 20 || player_x[plr] >= 300 || player_y[plr] <= 40 || player_y[plr] >= 220) {
					player_collision[plr] = 1;
				}
			}
		}
		
		for(i = 0; i < players; i++) {
			rectangle r = {player_x[i]-1, player_x[i]+1, player_y[i]-1, player_y[i]+1};
			if(player_collision[i] == 1) {
				fill_rectangle(r, WHITE);
			} else {
				fill_rectangle(r, player_color[i]);
			}
		}
}

void reset(){
	if(lives == 0) {
		score = 0;
		lives = 3;
	}
	else
		lives--;
	
	line_count = 0;
	set_default_pos();
}

ISR(TIMER1_COMPA_vect)
{
	if(player_collision[0] == 1)
		return;
		
	if (center_pressed()){
		players++;
		if(players > 4)
			players = 2;
		
		player_collision[0] = 1;
	}
	if(left_pressed()){ // up
		player_dir[0] = 0;
		line l = {player_x[0] , player_y[0] , player_dir[0], 0};
		tron_lines[line_count] = l;
		player_line[0] = line_count;
		line_count++;
	}
	if(right_pressed()){ // down
		player_dir[0] = 2;
		line l = {player_x[0] , player_y[0] , player_dir[0], 0};
		tron_lines[line_count] = l;
		player_line[0] = line_count;
		line_count++;
	}
	if(up_pressed()){ // right
		player_dir[0] = 1;
		line l = {player_x[0] , player_y[0] , player_dir[0], 0};
		tron_lines[line_count] = l;
		player_line[0] = line_count;
		line_count++;
	}
	if(down_pressed()){ // left
		player_dir[0] = 3;
		line l = {player_x[0] , player_y[0] , player_dir[0], 0};
		tron_lines[line_count] = l;
		player_line[0] = line_count;
		line_count++;
	}
}

ISR(TIMER3_COMPA_vect)
{
	if(player_collision[0] == 0 && (player_collision[1] == 0 || player_collision[2] == 0 || player_collision[3] == 0)) {
		
		score++;
		
		// Move theplayer 1
		if(player_dir[0] == 0)
			player_y[0] -= 6;
		else if (player_dir[0] == 1)
			player_x[0] += 6;
		else if (player_dir[0] == 2)
			player_y[0] += 6;
		else if (player_dir[0] == 3)
			player_x[0] -= 6;

		uint16_t len = 0;
		if(player_dir[0] == 0) {
			len = tron_lines[player_line[0]].y-player_y[0];
		} else if (player_dir[0] == 1) {
			len = player_x[0]-tron_lines[player_line[0]].x;
		} else if (player_dir[0] == 2) {
			len = player_y[0]-tron_lines[player_line[0]].y;
		} else if (player_dir[0] == 3) {
			len = tron_lines[player_line[0]].x-player_x[0];
		}
		tron_lines[player_line[0]].len = len;
			
		int plr;
		for(plr = 1; plr < players; plr++) {
			if(player_collision[plr] == 1)
				continue;
				
			// P2 Check if line is coming up ahead
			int will_collide = 0;
			uint16_t tempx = player_x[plr];
			uint16_t tempy = player_y[plr];
			
			if(player_dir[plr] == 0) {
				tempy -= 6;
			} else if (player_dir[plr] == 1) {
				tempx += 6;
			} else if (player_dir[plr] == 2) {
				tempy += 6;
			} else if (player_dir[plr] == 3) {
				tempx -= 6;
			}
			
			int i;

			for(i = 0; i < line_count; i++) {
				if(tron_lines[i].dir == 0) {
					if(tempy <= tron_lines[i].y && tempy >= tron_lines[i].y - tron_lines[i].len && tempx == tron_lines[i].x) {
						will_collide =  1;
						break;
					}
				}
				else if(tron_lines[i].dir == 1) {
					if(tempx >= tron_lines[i].x && tempx <= tron_lines[i].x + tron_lines[i].len && tempy == tron_lines[i].y) {
						will_collide =  1;
						break;
					}
				}
				else if(tron_lines[i].dir == 2) {
					if(tempy >= tron_lines[i].y && tempy <= tron_lines[i].y + tron_lines[i].len && tempx == tron_lines[i].x) {
						will_collide =  1;
						break;
					}
				}
				else if(tron_lines[i].dir == 3) {
					if(tempx <= tron_lines[i].x && tempx >= tron_lines[i].x - tron_lines[i].len && tempy == tron_lines[i].y) {
						will_collide =  1;
						break;
					}
				}
			}
			
			// or will it hit a wall
			if(player_dir[plr] == 2 && tempy >= 220)
				will_collide = 1;
				
			if(player_dir[plr] == 1 && tempx >= 300)
				will_collide = 1;
			
			if(player_dir[plr] == 0 && tempy <= 40)
				will_collide = 1;
				
			if(player_dir[plr] == 3 && tempx <= 20)
				will_collide = 1;
						
			int new_dir = player_dir[plr];
			
			uint16_t decision = rand() % 10;
			// Lets try and choose a direction which wont hit a wall/line
			if(will_collide == 1 || decision == 0) {
				int ok_path = 0;
				int tries = 0;
					
				if(rand() % 2 == 1) {
					new_dir++;
				} else {
					new_dir += 3;
				}
				new_dir = new_dir % 4;
				
				
				while(ok_path != 1 && tries < 2){
					tries++;

					tempx = player_x[plr];
					tempy = player_y[plr];
					
					if(new_dir == 0) {
						tempy -= 6;
					} else if (new_dir == 1) {
						tempx += 6;
					} else if (new_dir == 2) {
						tempy += 6;
					} else if (new_dir == 3) {
						tempx -= 6;
					}
					
					ok_path = 1;
					
					for(i = 0; i < line_count; i++) {
					if(tron_lines[i].dir == 0) {
						if(tempy <= tron_lines[i].y && tempy >= tron_lines[i].y - tron_lines[i].len && tempx == tron_lines[i].x) {
							ok_path =  0;
							break;
						}
					}
					else if(tron_lines[i].dir == 1) {
						if(tempx >= tron_lines[i].x && tempx <= tron_lines[i].x + tron_lines[i].len && tempy == tron_lines[i].y) {
							ok_path =  0;
							break;
						}
					}
					else if(tron_lines[i].dir == 2) {
						if(tempy >= tron_lines[i].y && tempy <= tron_lines[i].y + tron_lines[i].len && tempx == tron_lines[i].x) {
							ok_path =  0;
							break;
						}
					}
					else if(tron_lines[i].dir == 3) {
						if(tempx <= tron_lines[i].x && tempx >= tron_lines[i].x - tron_lines[i].len && tempy == tron_lines[i].y) {
							ok_path =  0;
							break;
						}
					}
				}
			
				if(new_dir == 2 && tempy >= 220)
					ok_path = 0;
					
				if(new_dir == 1 && tempx >= 300)
					ok_path = 0;
				
				if(new_dir == 0 && tempy <= 40)
					ok_path = 0;
					
				if(new_dir == 3 && tempx <= 20)
					ok_path = 0;
				
				if(ok_path == 0)
					new_dir = (new_dir + 2) % 4;
				}
				
				if(tries == 2 && decision == 0 && will_collide == 0) {
					new_dir = player_dir[plr];
				}
			}
		

			if(new_dir != player_dir[plr]) {
				player_dir[plr] = new_dir;
				line l = {player_x[plr] , player_y[plr] , player_dir[plr], 0};
				tron_lines[line_count] = l;
				player_line[plr] = line_count;
				line_count++;
			}
			
			if(player_dir[plr] == 0)
				player_y[plr] -= 6;
			else if (player_dir[plr] == 1)
				player_x[plr] += 6;
			else if (player_dir[plr] == 2)
				player_y[plr] += 6;
			else if (player_dir[plr] == 3)
				player_x[plr] -= 6;
			
			uint16_t len = 0;
			if(player_dir[plr] == 0) {
				len = tron_lines[player_line[plr]].y-player_y[plr];
			} else if (player_dir[plr] == 1) {
				len = player_x[plr]-tron_lines[player_line[plr]].x;
			} else if (player_dir[plr] == 2) {
				len = player_y[plr]-tron_lines[player_line[plr]].y;
			} else if (player_dir[plr] == 3) {
				len = tron_lines[player_line[plr]].x-player_x[plr];
			}
			tron_lines[player_line[plr]].len = len;
		}
	}
	redraw();
}

int main()
{
	
	CLKPR = (1 << CLKPCE);
	CLKPR = 0;
	init_lcd();
	init_switches();
	set_frame_rate_hz(61);
	set_orientation(North);
	
	DDRB |= _BV(PB7);
	PORTB &= ~_BV(PB7);

	/* enable button press inturrupt */
	TCCR1A = 0;
	TCCR1B = _BV(WGM12);
	TCCR1B |= _BV(CS10);
	TIMSK1 |= _BV(OCIE1A);

	OCR3A = 0;

	/*timer for random seed */
	TCCR2B |= (1 << CS10);

	/* timer for block falling speed*/
	TCCR3A = 0;
	TCCR3B = _BV(WGM32);
	TCCR3B |= _BV(CS32);
	TIMSK3 |= _BV(OCIE3A);
	
	OCR3A = 5000;
	
	set_orientation(West);
	
	display_string_col("  ___________                     \n", BLUE_3);
	display_string_col("  \\__    ___/______  ____   ____  \n", BLUE_3);
	display_string_col("    |    |  \\_  __ \\/  _ \\ /    \\ \n", BLUE_3);
	display_string_col("    |    |   |  | \\(  <_> )   |  \\\n", BLUE_3);
	display_string_col("    |____|   |__|   \\____/|___|  /\n", BLUE_3);
 	display_string_col("                               \\/ By Max Hayman\n", BLUE_3);
	display_string_col("\n", BLUE_3);
	display_string_col("Flynn's Arcade is a video game arcade owned and\n", BLUE_3);
	display_string_col("operated by Kevin Flynn.\n\n", BLUE_3);
	display_string_col("It is a multi-story establishment, with numerous\n", BLUE_3);
	display_string_col("assembled arcade games on the ground floor and\n", BLUE_3);
	display_string_col("office space on the floor above. \n", BLUE_3);
	display_string_col("\n          Press Center to Start", BLUE_3);
	LED_ON;


	do{
		while(!center_pressed()){}
		reset();
		
		display.background = BLACK;
		display.foreground = BLUE_3;
		
		// all black
		rectangle background = {0,display.width,0,display.height};
		fill_rectangle(background,display.background);
		
		// blue bar
		rectangle bar = {0,display.width,200,30};
		fill_rectangle(bar,display.foreground);
		
		int i, j;
		for (i = 20; i <= 300; i=i+20){
			rectangle shape = {i, i,40,220};
			fill_rectangle(shape,DEEP_SKY_BLUE_3);
		}
		
		for (j = 40; j <= 220; j=j+20){
			rectangle shape = {20,300,j,j};
			fill_rectangle(shape,DEEP_SKY_BLUE_3);
		}

		redraw();

		srand(TCNT2);
		//spawn_block();
		OCR1A = 65535;
		LED_OFF;
		sei();
		while(player_collision[0] == 0 && (player_collision[1] == 0 || player_collision[2] == 0 || player_collision[3] == 0));
		cli();
		LED_ON;
		if(player_collision[0] == 1) {
			if(lives==0) {
				display_string_xy("Game Over",130,220); 
			} else {
				display_string_xy("You have another life!",100,220);
			}
		}
		else
		{
			display_string_xy("You Won (Gained extra 500 points)",70,220); 
			score += 500;
			redraw();
			if(score > hightscore) {
				hightscore = score;
				display_string_xy("A NEW HIGH SCORE",5,5);
			}
		}
		display.background = BLACK;
		display.foreground = WHITE;
		display_string_xy("Press Center to Restart",95,230);
	} while (1);

	return -1;
}