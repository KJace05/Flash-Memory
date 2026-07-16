#include <LiquidCrystal.h>
#include <EEPROM.h>

// Button initalization
#define R_BUTT_PIN 6
#define G_BUTT_PIN 5
#define Y_BUTT_PIN 4
#define B_BUTT_PIN 3
#define REPLAY_BUTT_PIN A5


// LED initalization
#define R_LED 10
#define G_LED 11
#define Y_LED 12
#define B_LED 13


// IC LED & Buzzer initalization
#define IC_LEVEL_DATA 9
#define IC_LEVEL_LATCH 8
#define IC_LEVEL_CLOCK 7


// setting up game state for main loop
enum GameState {SHOW_SEQUENCE, WAIT_FOR_INPUT, LEVEL_COMPLETE, GAME_WON}; // for switch statement
GameState state_of_game = SHOW_SEQUENCE; //begin at showing sequence


//colored LEDS
int colored_leds[] = {10,11,12,13}; // colored LEDS
int level = 0; // start on level 1
bool level_over = false;
int total_flashing = 2; // initally, 2LEDS will flash. 6 max
int sequence[6] = {};
bool sequence_made = false;
int flash_index = 0;
bool led_on = false;
unsigned long last_LED_flash = millis();
unsigned long flash_delay = 300;


// White LEDS
int level_leds[] = {1,2,3,4,5};
int level_led = 1;


// BUTTONS
int buttons[] = {6,5,4,3}; //buttons
unsigned long last_debounce_time[4] = {0,0,0,0};
unsigned long debounce_delay = 25;
bool button_state[4] = {LOW,LOW,LOW,LOW};
bool last_button_state[4] = {LOW,LOW,LOW,LOW};


//replay button
int replay_button = A5;
bool replay_state = LOW;
bool last_replay_state = LOW;
unsigned long last_replay_debounce_time = 0;


// LCD screen
LiquidCrystal lcd{A0,A1,A2,A3,A4,2};
int row = 0;
int col = 0;


//player guess
int guess[8] = {};
int guess_index = 0;


//initalial setup
void setup()
{
 // Serial.begin(115200);
  //inialize random seed for sequencing
  unsigned long seed = 0;
  EEPROM.get(0, seed);
  if (seed == 0xFFFFFFFF) seed = 0; // handle blank EEPROM on first boot
  seed++;
  EEPROM.put(0, seed);
  randomSeed(seed);
  // burn through initial similar values
  for (int i = 0; i < 10; i++)
  {
      random(10, 14);
  }

  lcd.begin(16,2);
  show_title();

  //initalize colored LEDS
  for (int i = 0; i < 4; i++)
  {
    pinMode(colored_leds[i], OUTPUT);
  }

  //initalize buttons
  for (int i = 0; i < 4; i++)
  {
    pinMode(buttons[i], INPUT_PULLUP);
  }

  //inialitze replay button
  pinMode(replay_button, INPUT_PULLUP);


  //initalize level and buzzer IC
  pinMode(IC_LEVEL_DATA, OUTPUT);
  pinMode(IC_LEVEL_LATCH, OUTPUT);
  pinMode(IC_LEVEL_CLOCK, OUTPUT);

  // clear all IC outputs on startup
  digitalWrite(IC_LEVEL_LATCH, LOW);
  shiftOut(IC_LEVEL_DATA, IC_LEVEL_CLOCK, MSBFIRST, B00000000);
  digitalWrite(IC_LEVEL_LATCH, HIGH);

  delay(2000);
  last_LED_flash = millis(); // reset timer after delay
}


//display game title on LCD
void show_title()
{
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print("Flash");

  lcd.setCursor(6,1);
  lcd.print("Memory");
}

//randomly flash LEDS / SHOW SEQUENCE
void flash_leds()
{
  //stop showing sequence
  if (level_over)
  {
    return;
  }

  //adds pins to squence once
  if(!sequence_made)
  {
    //add LED pins to sequence
    for (int i = 0; i < total_flashing; i++)
    {
      sequence[i] = random(10,14); // colored LEDs are pins 10,11,12,13
    }
    sequence_made = true;
  }
  //time
  unsigned time_now = millis(); //current time


  //flash those led pins
  if(time_now - last_LED_flash >= flash_delay)
  {
    last_LED_flash = time_now;
    //if the LED is on, turn it off
    if(led_on)
    {
        digitalWrite(sequence[flash_index], LOW); // turn colored off
        led_on = false;
        flash_index++; // move to next LED


        //when completed the sequence
        if(flash_index >= total_flashing)
        {
          flash_index = 0;
          level_over = true;
        }
    }
    else // if LED is off, turn it on 
    {
    digitalWrite(sequence[flash_index], HIGH);
    led_on = true;
    buzzer_sound();
    }
  }
}


// WAIT FOR INPUT
void player_input()
{
  //stop if level is complete
  if (!level_over)
  {
    return;
  }

  //check state of each of the 4 buttons
  for (int i = 0; i < 4; i++)
  {
    bool reading = digitalRead(buttons[i]); //read raw electrical state of button, unverified 

    // check if the buttons reading changed
    if (reading != last_button_state[i])
    {
      last_debounce_time[i] = millis();
    }

    // check if button reading was stable
    if (millis() - last_debounce_time[i] > debounce_delay)
    {

      // if new state
      if (reading != button_state[i])
      {
        button_state[i] = reading; //confirmed state

        if (button_state[i] == LOW) //if high, go to handle function
        {
          handle_button_press(i);
        }
      }
    }
    last_button_state[i] = reading; //store raw reading for next comparison 
  } 
}


// compare player guess to sequence
void handle_button_press(int button_index)
{
  //Serial.print("PRESS DETECTED on button ");
 // Serial.println(button_index);

  int pressed_led = colored_leds[button_index];

  // flash player input
  digitalWrite(pressed_led, HIGH);
  delay(100); //delay doesnt cause much interference with program
  digitalWrite(pressed_led, LOW);

  //comparison
  if(pressed_led == sequence[guess_index])
  {
    guess_index++;

    //level complete
    if (guess_index >= total_flashing)
    {
      //won game
      if (level >= 4)
      {
        // light all 5 LEDs on win
        level = 5;
        initalize_level_leds();

        state_of_game = GAME_WON;
        lcd.clear();
        lcd.setCursor(5,0);
        lcd.print("YOU");

        lcd.setCursor(7,1);
        lcd.print("WIN!");
        return;
      }

      level++;
      total_flashing++;
      guess_index = 0;
      flash_index = 0;
      led_on = false;
      level_over = false;
      sequence_made = false;
      last_LED_flash = millis(); 
      state_of_game = SHOW_SEQUENCE;
     // Serial.println("level complete");

      initalize_level_leds(); // update IC output

      delay(1000); //delay to stop all input from user 
    }
  }
  else // wrong guess
  {
    //Serial.print("Wrong button: ");
    //Serial.println(pressed_led);
    buzzer_sound(); // buzzes when wrong

    // flash LEDS when wrong guess
    for(int i = 0 ; i < 4; i++)
    {
      digitalWrite(colored_leds[i], HIGH);
    }
    delay(100);
    for(int i = 0 ; i < 4; i++)
    {
      digitalWrite(colored_leds[i], LOW);
    }

    //replay sequence
    guess_index = 0;
    flash_index = 0;
    led_on = false;
    level_over = false;
    last_LED_flash = millis(); 
    state_of_game = SHOW_SEQUENCE;
    delay(1000);
  }
}


// CHECK REPLAY
void replay_sequence()
{


  // rest of function...
  //only allow if waiting for player response
  if (state_of_game != WAIT_FOR_INPUT)
  {
    return;
  }

  bool reading = digitalRead(replay_button); //read current button state

  // if current read is different than last, change time
  if(reading != last_replay_state)
  {
    last_replay_debounce_time = millis();
  }

  if(millis() - last_replay_debounce_time > debounce_delay)
  {
    if(reading != replay_state)
    {
      replay_state = reading;

      // if clicked, replay sequence
      if(replay_state == LOW)
      {
        guess_index = 0;
        flash_index = 0;
        led_on = false;
        level_over = false;
        last_LED_flash = millis();
        state_of_game = SHOW_SEQUENCE;
      }
    }
  }
  last_replay_state = reading; //store to compare later
}


//LEVEL LEDS
void initalize_level_leds()
{
  byte led_data = 0; // start data chain with 0

  for(int i = 0; i < level; i++)
  {
    bitSet(led_data, level_leds[i] ); //turn ON the bit at that position.
  }


  //send data to IC
  digitalWrite(IC_LEVEL_LATCH, LOW); //wait for all input
  shiftOut(IC_LEVEL_DATA, IC_LEVEL_CLOCK, MSBFIRST, led_data); // shift data to clock by most significant bit first
  digitalWrite(IC_LEVEL_LATCH, HIGH); // release all bits
}


//BUZZER
void buzzer_sound()
{
  byte buzzer_data = 0;

  //preserve white leds
  for(int i = 0; i < level; i++)
  {
    bitSet(buzzer_data, level_leds[i]);
  }

  bitSet(buzzer_data, 0); // on
  digitalWrite(IC_LEVEL_LATCH, LOW);
  shiftOut(IC_LEVEL_DATA, IC_LEVEL_CLOCK, MSBFIRST, buzzer_data);
  digitalWrite(IC_LEVEL_LATCH, HIGH);
  
  delay(100);

  bitClear(buzzer_data, 0); // off
  digitalWrite(IC_LEVEL_LATCH, LOW);
  shiftOut(IC_LEVEL_DATA, IC_LEVEL_CLOCK, MSBFIRST, buzzer_data);
  digitalWrite(IC_LEVEL_LATCH, HIGH);
}


//main game
void loop()
{
  replay_sequence();   //always listening

  //main loop
  switch(state_of_game)
  {
    case SHOW_SEQUENCE:
     // Serial.println(level);
      flash_leds();
      if(level_over)
      {
        state_of_game = WAIT_FOR_INPUT;
      }
      break;

    case WAIT_FOR_INPUT:
      player_input();
      break;

    case LEVEL_COMPLETE:
      level++;
      total_flashing++; // 2..3..4..5..6
      state_of_game = SHOW_SEQUENCE;
      break;

    case GAME_WON:
      break;
    default:
      break;
  }

}