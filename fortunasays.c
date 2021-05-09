/*
 * FortunaSays - remember and input a sequence of increasing length using the direction buttons on the LaFortuna.
 * High scores can be saved to an SD card, if one is inserted.
 * Differs from traditional Simon Says in that the first (n-1) flashes also change in round n.
 *
 * Author: Daniel Burton, May 2021
 *
 * This code uses the rotary encoder library by Steve Gunn,
 * as well as FortunaOS, containing code by Steve Gunn, Klaus-Peter Zauner, Peter Dannegger.
 *
 * Licence: This work is licensed under the Creative Commons Attribution License.
 *           View this license at http://creativecommons.org/about/licenses/.
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <string.h>
#include "rotary.h"
#include "os.h"

#define PIXEL_SIZE 5
#define TEXT_CENTER 120

typedef enum {UP,DOWN,RIGHT,LEFT} arrow_dir;

struct level
{
	int response_num;
        arrow_dir flashes[100];
};


void init(void);
int num_times_to_flash(int, int);
struct level generate_level(int);
void display_level(struct level, int, int);
void delay_between_flashes(int);
int button_pressed(int);
void flash_led(void);
void flash_led_strike(void);
void drawLeft(void);
void drawRight(void);
void drawDown(void);
void drawUp(void);
void update_scores(void);
void show_scores(void);

int response = 0;
volatile int game_round = 0;
volatile int strikes = 0;
volatile int input_num = 0;
volatile arrow_dir next;
struct level current_level;
volatile int level_size;
volatile int score;
volatile int game_in_progress;
int sd_connected;

FIL scores; /* FAT file to store high scores */

int main()
{
	cli();
	init();
	os_add_task(button_pressed, 10, 1);
	sei();

	game_in_progress = 0;
	sd_connected = 0;
	clear_screen();
        display_string_xy("Press centre to start or down to view high scores.",0,TEXT_CENTER);
	while(1) {
	  while(!game_in_progress) {}
	  clear_screen();
	  game_round = 1;
  	  strikes = 0;
	  score = 0;
	  while(strikes < 3)
	  {
		  if(!response)
		  {
			  int times_to_flash = num_times_to_flash(game_round, strikes);
			  arrow_dir flashes[times_to_flash];
			  current_level = generate_level(times_to_flash);
			  display_level(current_level, game_round, times_to_flash);
			  level_size = times_to_flash;
			  clear_screen();
			  display_string("GO!");
		  	  response = 1;
		  }

		  if(game_round > 1) {} /* This appears to be necessary because of compiler optimisation. */
	  }

	  update_scores();
	  clear_screen();
	  char scoreStr[50];
	  sprintf(scoreStr, "%d", score);
	  char endText[] = "Game over.\nYour score: ";
	  strcat(endText, scoreStr);
	  strcat(endText, "\nCentre to play again or down to view scores.");
	  display_string_xy(endText, TEXT_CENTER, TEXT_CENTER);
	  game_in_progress = 0;
	}
}

void init(void)
{
	/* 8MHz clock with no prescaling */
	CLKPR = (1 << CLKPCE);
	CLKPR = 0;
	os_init();
	init_rotary();

	srand(time(0)); /* Use current time as random seed */

	DDRB  &=  ~_BV(PB7);  	 /* LED off */
}

int button_pressed(int state) {


	if(get_switch_long(_BV(OS_CD))) /* Acknowledge insertion of the SD card. */
	{
		sd_connected = 1;
	}

	if(!game_in_progress && get_switch_press(_BV(SWC))) /* Start a game */
	{
		clear_screen();
		game_in_progress = 1;
		return state;
	}

	int north = get_switch_press(_BV(SWN));
	int east = get_switch_press(_BV(SWE));
	int south = get_switch_press(_BV(SWS));
	int west = get_switch_press(_BV(SWW));


	if(!game_in_progress && south) /* Display the high scores screen. */
	{
		clear_screen();
		display_string("High Scores:\n");
		show_scores();
		display_string("\nCentre: new game. Down: reload scores.");
		return state;
	}

	if(!response && (north || east || south || west))
	{ /* If the player attempts to input while not in an input phase */
		display_string("Not ready!");
		return state;
	}

	if(!response) /* If the game is not in an input phase */
	{
		return state;
	}

	if(north && current_level.flashes[current_level.response_num] == UP)
	{
		flash_led();
		current_level.response_num++;
	}
	else if(east && current_level.flashes[current_level.response_num] == RIGHT)
	{
		flash_led();
		current_level.response_num++;
	}
	else if(south && current_level.flashes[current_level.response_num] == DOWN)
	{
		flash_led();
		current_level.response_num++;
	}
	else if(west && current_level.flashes[current_level.response_num] == LEFT)
	{
		flash_led();
		current_level.response_num++;
	}
	else if(north || east || south || west) /* Unmatching button pressed - report a strike. */
	{
		clear_screen();
		display_string_xy("Strike!", TEXT_CENTER, TEXT_CENTER);
		flash_led_strike();
		strikes++;
		response = 0;
		return state;
	}

	if(current_level.response_num >= level_size) /* Level passed successfully - move onto the next level. */
	{
		clear_screen();
		game_round++;
		response = 0;
		score += level_size;
	}	return state;
}

/* A quick LED flash indicates a correct input. */
void flash_led()
{
	DDRB  |=  _BV(PB7);
	_delay_ms(50);
	DDRB  &=  ~_BV(PB7);
}

/* A long LED flash indicates a wrong input. */
void flash_led_strike()
{
	DDRB  |=  _BV(PB7);
	_delay_ms(500);
	DDRB  &=  ~_BV(PB7);
}

void drawRight()
{
	rectangle r = {140, 180, 120, 125};
	fill_rectangle(r, WHITE);

	for(int i = 0; i < 4; i++)
	{
		rectangle a = {160 + PIXEL_SIZE*i, 165 + PIXEL_SIZE*i, 100 + PIXEL_SIZE*i, 105 + PIXEL_SIZE*i};
		fill_rectangle(a, WHITE);
	}

	for(int i = 0; i < 4; i++)
	{
		rectangle b = {160 + PIXEL_SIZE*i, 165 + PIXEL_SIZE*i, 140 - PIXEL_SIZE*i, 145 - PIXEL_SIZE*i};
		fill_rectangle(b, WHITE);
	}
}

void drawLeft()
{
	rectangle r = {140, 180, 120, 125};
        fill_rectangle(r, WHITE);

        for(int i = 0; i < 4; i++)
        {
                rectangle a = {155 - PIXEL_SIZE*i, 160 - PIXEL_SIZE*i, 100 + PIXEL_SIZE*i, 105 + PIXEL_SIZE*i};
                fill_rectangle(a, WHITE);
        }

        for(int i = 0; i < 4; i++)
        {
                rectangle b = {155 - PIXEL_SIZE*i, 160 - PIXEL_SIZE*i, 140 - PIXEL_SIZE*i, 145 - PIXEL_SIZE*i};
                fill_rectangle(b, WHITE);
        }
}

void drawUp()
{
	rectangle r = {160, 165, 100, 140};
        fill_rectangle(r, WHITE);

        for(int i = 0; i < 4; i++)
        {
                rectangle a = {140 + PIXEL_SIZE*i, 145 + PIXEL_SIZE*i, 115 - PIXEL_SIZE*i, 120 - PIXEL_SIZE*i};
                fill_rectangle(a, WHITE);
        }

        for(int i = 0; i < 4; i++)
        {
                rectangle b = {180 - PIXEL_SIZE*i, 185 - PIXEL_SIZE*i, 115 - PIXEL_SIZE*i, 120 - PIXEL_SIZE*i};
                fill_rectangle(b, WHITE);
        }

}

void drawDown()
{
	rectangle r = {160, 165, 100, 140};
        fill_rectangle(r, WHITE);

        for(int i = 0; i < 4; i++)
        {
                rectangle a = {140 + PIXEL_SIZE*i, 145 + PIXEL_SIZE*i, 120 + PIXEL_SIZE*i, 125 + PIXEL_SIZE*i};
                fill_rectangle(a, WHITE);
        }

        for(int i = 0; i < 4; i++)
        {
                rectangle b = {180 - PIXEL_SIZE*i, 185 - PIXEL_SIZE*i, 120 + PIXEL_SIZE*i, 125 + PIXEL_SIZE*i};
                fill_rectangle(b, WHITE);
        }
}

/* Determine number of times to flash, based on the round and number of strikes. */
int num_times_to_flash(int round, int strikes)
{
	if(strikes == 0) return 3 + round/2;
	else if(strikes == 1) return 2 + round/2;
	else return 1 + round/2;
}

/* Generate a level. Number of flashes is fixed based on calculation above, but the moves themselves are random. */
struct level generate_level(int num_flashes)
{
	struct level created;
	arrow_dir possible_flashes[4] = {UP,DOWN,LEFT,RIGHT};
	for(int i = 0; i < num_flashes; i++) created.flashes[i] = possible_flashes[rand() % 4];
	created.response_num = 0;
	return created;
}

/* Draws a level on-screen. */
void display_level(struct level current, int round, int num_flashes)
{
	if(strikes > 2) return;
	clear_screen();

	for(int i = 0; i < num_flashes; i++)
	{
		if(current.flashes[i] == UP) drawUp();
		else if(current.flashes[i] == DOWN) drawDown();
		else if(current.flashes[i] == RIGHT) drawRight();
		else drawLeft();

		delay_between_flashes(round);
		clear_screen();
		delay_between_flashes(round);
	}
}

/* Delay gets smaller to increase difficulty as the game progresses. */
void delay_between_flashes(int round)
{
	if(round < 3) _delay_ms(1000);
	else if(round < 5) _delay_ms(750);
	else if(round < 7) _delay_ms(500);
	else _delay_ms(400);
}

/* Display the top 10 scores from the scores file, if the SD card is inserted. */
void show_scores()
{
	if(!sd_connected)
        {
                display_string("No card connected!");
                return;
        }

        f_mount(&FatFs, "", 0);
        char buf[10][10];

        if (f_open(&scores, "scores.txt", FA_READ) == FR_OK)
        {
                int counter = 0;
                while(f_gets(buf[counter], 10, &scores))
                {
                        counter++;
                        if(counter > 9) break;
                }

		for(int i = 0; i < counter; i++)
		{
			display_string(buf[i]);
		}
		f_close(&scores);
        }
	else
	{
		display_string("SD Card - Read fail!");
	}
}

/* Inspect current contents of the scores file and insert the player's score, if high enough, in the top 10. */
void update_scores()
{
	if(!sd_connected)
	{
		display_string("No card connected!");
		return;
	}

	f_mount(&FatFs, "", 0);
	int score_index = 10;
	int num_scores = 0;
	char buf[10][10];

	if (f_open(&scores, "scores.txt", FA_READ) == FR_OK)
	{
		int counter = 0;
		while(f_gets(buf[counter], 10, &scores))
		{
			num_scores++;
			counter++;
			if(counter > 9) break;
		}

		for(int i = 0; i < counter; i++)
		{
			if(score > atoi(buf[i]))
			{
				score_index = i;
				break;
			}
		}
		f_close(&scores);

		if(score_index == 10 && counter < 9) score_index = counter;
	}
	else
	{
		display_string("SD Card - Read fail!");
	}

	if (f_open(&scores, "scores.txt", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK)
	{
		int counter = 0;
		for(int i = 0; i < 10; i++)
		{
			if(counter > num_scores) break;
			if(i == score_index)
			{
				f_printf(&scores, "%d\n", score);
			}
			else
			{
				f_printf(&scores, buf[counter]);
				counter++;
			}

		}

		f_close(&scores);
	}
	else
	{
		display_string("SD Card - Write fail!");
	}
}
