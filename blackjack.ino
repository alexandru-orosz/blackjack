#include <LiquidCrystal.h>
#include <Keypad.h>

#define HEART 0
#define DIAMOND 1
#define SPADE 2
#define CLUB 3
#define TEN 4
#define FACE_DOWN_LEFT 5
#define FACE_DOWN_RIGHT 6

#define BUZZER_PIN 13

#define ASCII_DISPLACEMENT 48

#define TOTAL_DECK_CARDS 52

bool deck[TOTAL_DECK_CARDS];
bool finished = false;
bool blackjack = false;


unsigned long balance = 1000;
unsigned long bet = 0;

struct Card{
  byte name; 
  byte value;
  byte suit; 
  byte key; 
};

struct Hand{
  Card cards[10];
  byte size;
  byte points;
  byte cursor;
};

byte heart[8] = {
  B00000,
  B01010,
  B11111,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000
};

byte diamond[8] = {
  B00000,
  B00100,
  B01110,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000
};

byte spade[8] = {
  B00000,
  B00100,
  B01110,
  B11111,
  B11111,
  B00100,
  B01110,
  B00000
};

byte club[8] = {
  B00000,
  B01110,
  B01110,
  B11111,
  B11111,
  B00100,
  B01110,
  B00000
};

byte ten[8] = {
  B10111,
  B10101,
  B10101,
  B10101,
  B10101,
  B10101,
  B10101,
  B10111
};

byte face_down_left[8] = {
  B11111,
  B10110,
  B10101,
  B10110,
  B10101,
  B10110,
  B10000,
  B11111
};

byte face_down_right[8] = {
  B11111,
  B00001,
  B01111,
  B00101,
  B00101,
  B10101,
  B01001,
  B11111
};

const byte numRows= 4; 
const byte numCols= 4; 

char keymap[numRows][numCols]= 
{
{'1', '2', '3', 'A'}, 
{'4', '5', '6', 'B'}, 
{'7', '8', '9', 'C'},
{'*', '0', '#', 'D'}
};

byte rowPins[numRows] = {2,3,4,5};
byte colPins[numCols]= {6,7,8,9};

Keypad keypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

LiquidCrystal lcd(12, 11, A1, A2, A3, A4);

void setup() {
  pinMode(BUZZER_PIN,OUTPUT);
  
  randomSeed(analogRead(0));
  
  lcd.begin(16, 2);
  
  lcd.createChar(HEART,heart);
  lcd.createChar(DIAMOND,diamond);
  lcd.createChar(SPADE,spade);
  lcd.createChar(CLUB,club);
  lcd.createChar(TEN,ten);
  lcd.createChar(FACE_DOWN_LEFT, face_down_left);
  lcd.createChar(FACE_DOWN_RIGHT, face_down_right);
                
  lcd.setCursor(3,0);
  lcd.print("Blackjack");
  lcd.setCursor(6,1);
  
  lcd.print("A");
  lcd.write((byte)SPADE);
  lcd.print("J");
  lcd.write((byte)SPADE);
  
  delay(2000);
}

void loop() {
  
  struct Hand dealerHand;
  struct Hand playerHand;
  
  betting(dealerHand, playerHand);
  init_game(dealerHand, playerHand);
  
  while(finished == false){
    print_HS();
    check_keypad_HS(dealerHand, playerHand);
  }
  
  generate_dealer_cards(dealerHand,playerHand);

  calculate_balance(winner(dealerHand, playerHand));
  
  delay(2000);
  finished = false;
}

void betting(struct Hand &dealerHand, struct Hand &playerHand){
  if(balance == 0){
    lcd.clear();
    lcd.print("Your balance");
    lcd.setCursor(0,1);
    lcd.print("is $0.");
    delay(1500);
    
    lcd.clear();
    lcd.print("Adding another");
    lcd.setCursor(0,1);
    lcd.print("$1000.");
    delay(1500);
  
    balance = 1000;  
  }
  
  bet = 0;
  lcd.clear();
  lcd.print("Balance: $");
  lcd.print(balance);
  lcd.setCursor(0,1);
  lcd.print("Bet: $");
  
  while(finished == false){
    check_keypad_bet();
    lcd.setCursor(6,1);
    lcd.print(bet);
  }
  
  finished = false;
  delay(1000);         
}

void init_game(struct Hand &dealerHand, struct Hand &playerHand) {
  shuffle_deck();
  blackjack = false;
  
  //dealer cards
  lcd.setCursor(0,0);
  lcd.print("D:");
  
  dealerHand.size = 0;
  dealerHand.cursor = 6;
  
  generate_card(dealerHand);
  lcd.write((byte)FACE_DOWN_LEFT);
  lcd.write((byte)FACE_DOWN_RIGHT);
  print_card(generate_card(dealerHand));
  
  //player cards
  lcd.setCursor(0,1);
  lcd.print("P:");
  
  playerHand.size = 0;
  playerHand.cursor = 6;
  
  print_card(generate_card(playerHand));
  print_card(generate_card(playerHand));
  
  calculate_points(playerHand);
  print_points(1, playerHand.points);
  
  if(playerHand.points == 21){
     hide_HS();
     calculate_points(dealerHand);
     print_points(0, dealerHand.points);
     finished = true;
     
    if(dealerHand.points != 21){
      blackjack = true;
    }
  }
}

void check_keypad_HS(struct Hand &dealerHand, struct Hand &playerHand) {
  char keyPressed = keypad.getKey();
  if(keyPressed == 'A'){
    lcd.setCursor(playerHand.cursor, 1);
    print_card(generate_card(playerHand));
    playerHand.cursor += 2;
    
    calculate_points(playerHand);
    print_points(1, playerHand.points);
    
    if(playerHand.points >= 21){
      hide_HS();
      finished= true;
    }
  }
  else {
    if(keyPressed == 'B'){
      hide_HS();
      calculate_points(dealerHand);
      print_points(0, dealerHand.points);
      finished = true;
    }
    else{
      if(keyPressed != NO_KEY){
        error_sound();
      }
    }
  }  
}

void check_keypad_bet(){
  char keyPressed = keypad.getKey();
  
  if(keyPressed == 'C'){
    if(bet != 0 && bet <= balance){
      finished = true;
    }
    else{
      bet = 0;
      lcd.setCursor(6,1);
      lcd.print("0         ");
      error_sound();
    }
  }
  else{
    if(keyPressed == 'D'){
      bet = 0;
      lcd.setCursor(7,1);
      lcd.print("         ");
    }
  
    else{
      if(keyPressed >= '0' && keyPressed <= '9'){
        keyPressed -= ASCII_DISPLACEMENT;
        bet = bet * 10 + keyPressed;
      }
      else{
        if(keyPressed == '#'){
          lcd.setCursor(6,1);
          lcd.print("         ");
          bet = balance / 2;
        }
        else{
          if(keyPressed == '*'){
            lcd.setCursor(6,1);
            lcd.print("         ");
            bet = balance;
          }
          else{
            if(keyPressed != NO_KEY){
              error_sound();
            }
          }
        }
      }
    }
  }
}

struct Card generate_card(struct Hand &hand){
  struct Card card;
  
  while(true){
    card.suit = random(4);
    card.key = random(13);
    if(available_card(card) == true) break;
  }
  
  card.value = card.key + 2;
  
  switch(card.value){
    case 11: card.name = 'J'; card.value = 10; break;
    case 12: card.name = 'Q'; card.value = 10; break;
    case 13: card.name = 'K'; card.value = 10; break;
    case 14: card.name = 'A'; card.value = 11; break;
    case 10: break;
    default: card.name = card.value + ASCII_DISPLACEMENT;
  }
  
  remove_card_from_deck(card);
  hand.cards[hand.size]=card;
  (hand.size)++;
  
  return card;
}

void generate_dealer_cards(struct Hand &dealerHand, struct Hand &playerHand) {
  lcd.setCursor(2,0);
  print_card(dealerHand.cards[0]);
  
  while(dealerHand.points < 17 && playerHand.points <= 21 && !blackjack){
    delay(1000);
    
    lcd.setCursor(dealerHand.cursor, 0);
    print_card(generate_card(dealerHand));
    dealerHand.cursor += 2;
    
    calculate_points(dealerHand);
    print_points(0, dealerHand.points); 
  }
    calculate_points(dealerHand);
    print_points(0, dealerHand.points); 
}

void calculate_points(struct Hand &hand){
  hand.points = 0;
  for(int i=0; i<hand.size; i++){
    hand.points += hand.cards[i].value;
  }
  
  if(hand.points > 21 && ace_number(hand) >= 1){
    hand.points -= 10;
  }
  
  if(hand.points > 21 && ace_number(hand) >= 2){
    hand.points -= 10;
  }
  
  if(hand.points > 21 && ace_number(hand) >= 3){
    hand.points -= 10;
  }
  
  if(hand.points > 21 && ace_number(hand) == 4){
    hand.points -= 10;
  }
}

byte winner(struct Hand &dealerHand, struct Hand &playerHand){
  // return 0 - dealer wins
  // return 1 - player wins
  // return 2 - draw
  
  delay(2000);
  lcd.clear();
  
  if(playerHand.points > 21){
    lcd.print("Dealer wins!");
    return 0;
  }
  
  if(dealerHand.points > 21){
    lcd.print("Player wins!");
    return 1;
  }
  
  if(dealerHand.points == playerHand.points){
    lcd.print("Draw!");
    return 2;
  }
  
  if(dealerHand.points > playerHand.points){
    lcd.print("Dealer wins!");
    return 0;
  }
  else{
    lcd.print("Player wins!");
    return 1;
  }
}

void calculate_balance(byte winner){
  unsigned long old_balance = balance;
  
  lcd.setCursor(0,1);
  lcd.print("Balance: $");
  
  float time_delay = 0;
  float inc = bet / 100.0;
  
  switch(winner){
    case 0: balance -= bet;
        lose_sound();
        
        for(float i=old_balance; i>=balance; i-=inc){
          lcd.setCursor(10,1);            
              lcd.print((int)floor(i));
              lcd.print("    ");
              delay(time_delay);
            }
        break;
    case 1: if(blackjack) balance += 1.5 * bet; 
        else balance += bet;
        win_sound();
          
          for(float i=old_balance; i<=balance; i+=inc){
          lcd.setCursor(10,1);            
              lcd.print((int)floor(i));
              lcd.print("    ");
              delay(time_delay);
            }
            break; 
    case 2: lcd.setCursor(10,1);
        lcd.print(balance);
        break;
    default: break;
  }  
}

void print_HS(){
  lcd.setCursor(9,0);
  lcd.print("H or S?");
}

void hide_HS(){
  lcd.setCursor(9,0);
  lcd.print("       ");
}

void shuffle_deck(){
  lcd.clear();
  lcd.print("Shuffling");
  for(int i=0; i<3; i++){
    delay(500);
    lcd.print(".");
  }
  delay(500);
  lcd.clear();
  
  init_deck();
}

void print_points(byte player, byte points){
  if(points<10){
    lcd.setCursor(15,player);
  }
  else{
    lcd.setCursor(14,player);
  }
  lcd.print(points);
}

void error_sound(){
  tone(BUZZER_PIN, 220, 20);
  delay(200);
}

void win_sound(){ 
  tone(BUZZER_PIN,880,100); //A5
  tone(BUZZER_PIN,988,100); //B5
  tone(BUZZER_PIN,523,100); //C5
  tone(BUZZER_PIN,988,100); //B5
  tone(BUZZER_PIN,523,100); //C5
  tone(BUZZER_PIN,587,100); //D5
  tone(BUZZER_PIN,523,100); //C5
  tone(BUZZER_PIN,587,100); //D5
  tone(BUZZER_PIN,659,100); //E5
  tone(BUZZER_PIN,587,100); //D5
  tone(BUZZER_PIN,659,100); //E5
  tone(BUZZER_PIN,659,100); //E5
  delay(250);
}

void lose_sound(){
  delay(400);
  for(double i=0; i<4; i+=6.541)
  {
    tone(BUZZER_PIN, 440+i, 50);
  }
  tone(BUZZER_PIN, 466.164, 100);
  delay(80);
  for(double i=0; i<5; i+=4.939)
  {
    tone(BUZZER_PIN, 415.305+i, 50);
  }
  tone(BUZZER_PIN, 440.000, 100);
  delay(80);
  for(double i=0; i<5; i+=4.662)
  {
    tone(BUZZER_PIN, 391.995+i, 50);
  }
  tone(BUZZER_PIN, 415.305, 100);
  delay(80);
  for(int i=0; i<7; i++)
  {
    tone(BUZZER_PIN, 391.995, 70);
    tone(BUZZER_PIN, 415.305, 70);
  }
  delay(400);
}

byte ace_number(struct Hand &hand){
  byte count = 0;
  for(int i=0; i<hand.size; i++){
    if(hand.cards[i].name == 'A'){
      count++;
    }
  }
  return count;
}

void print_card(struct Card card){
  if (card.key == 8) lcd.write((byte)TEN);
  else lcd.print((char)card.name);
  
  lcd.write((byte)card.suit);
}

void init_deck(){
  for(int i=0; i<TOTAL_DECK_CARDS; i++){
    deck[i] = true;
  }
}

void remove_card_from_deck(struct Card card){
  deck[card.key * 4 + card.suit] = false;
}

bool available_card(struct Card card){
  if(deck[card.key * 4 + card.suit] == true) return true;
  else return false;
}
