#include <LiquidCrystal.h>
#include<Keypad.h>
#include<stdio.h>


#define ROWS 4
#define COLS 3
#define LIMIT 280
#define pulPin 10
#define dirPin 11
#define dx 1
#define scale 8.0
#define LIMIT_X 280
const char *fmt = "%3di+%3dj+%3dk";
typedef struct vector{
  int x,y,z;
} Vector;

struct d{
  Vector current_location;
  Vector next_location;
  bool enter;
  bool read;
  char buffer[4];
  int counter;
  int base;
  int input;
  long dur;
  long cur;
  int stepsize;
  float speed;
} data;

char screen[2][16];

const char keys[ROWS][COLS] = {
                                {'1','2','3'},
                                {'4','5','6'},
                                {'7','8','9'},
                                {'*','0','#'}
                              };

byte rowPins[ROWS] = {A1,A2,A3,A4};
byte columnPins[COLS] = {A5,3,2};

Keypad csPad = Keypad(makeKeymap(keys),rowPins,columnPins,ROWS,COLS);
const int rs = 9, en = 8, d4 = 7, d5 = 6, d6 = 5, d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

char key;

void display(){
  for(int i=0;i<2;i++){
    for(int j=0;j<16;j++){
      lcd.setCursor(j, i);
      lcd.write(screen[i][j]);
    }
  }
}



void writeData(){
  screen[0][sprintf(screen[0],fmt,data.next_location.x,data.next_location.y,data.next_location.z)] = ' ';
  
}

bool equal(Vector *v1,Vector *v2){
  if(v1->x != v2->x) return false;
  if(v1->y != v2->y) return false;
  if(v1->z != v2->z) return false;
  return true;
}

void rotateRev(float rev){
  float r = rev*1E+6/(data.speed*data.stepsize);  
  for(int i=0;i<r;i++){
    digitalWrite(pulPin,HIGH);
    digitalWrite(pulPin,LOW);
    delayMicroseconds(data.stepsize);
  }
  
}

void evaluate(){

  int k = 0;
  for(int i=0;i<data.base;i++){
    k = k*10 + (data.buffer[i] - '0');
  }
  if(data.input == 'X') data.next_location.x = k;
  if(data.input == 'Y') data.next_location.y = k;
  if(data.input == 'Z') data.next_location.z = k;
  writeData();
}

int stepMove(int x,int y,int dirpin){
  if (y == x) return 0;
  bool oppose = y < x;
  if(oppose) digitalWrite(dirpin,HIGH);
  else digitalWrite(dirpin,LOW);
  rotateRev(1/scale);
  return oppose ? -1 : 1;
}

Vector *movDX(Vector *src,Vector *dst){
  src->y = dst->y;
  src->z = dst->z;
  src->x += stepMove(src->x,dst->x,dirPin);
  // Serial.println(src->x);
}

int k = 60;
int n = 10;
int m = millis();
int o_key = 0,p_key = 9;

void showDigit(char *buff,int k){
  if(!k){
    screen[1][0] = '0';
    return;
  }
  memcpy(screen[1],"     ",5);
  memcpy(screen[1],buff,k);

  
}

void setup(){
 memset(screen,' ',32);
 memcpy(screen,"SHASHANK",8);
 pinMode(A0,INPUT_PULLUP);
 pinMode(pulPin,OUTPUT);
 pinMode(dirPin,OUTPUT);
 digitalWrite(dirPin,LOW);
 Serial.begin(9600);
 lcd.begin(16, 2);
 lcd.clear();
 data.enter = false;
 data.counter = 3;
 data.buffer[data.counter] = '\0';
 data.current_location.x = data.next_location.x = 0;
 data.current_location.y = data.next_location.y = 0;
 data.current_location.z = data.next_location.z = 0;
 data.input = 'X';
 data.cur = 0;
 data.dur = 500;
 data.read = true;
 data.speed = 2;
 writeData();
 data.stepsize = 1E+6/(1600*data.speed);
}

void handleInput(){
  p_key = csPad.getKey();
  screen[1][14] = data.input;
  if(p_key == '*'){
    // Serial.println("Reading.\n");
    data.enter = !data.enter;
    if(data.enter){
      data.base = 0;
      screen[1][15] = 'W';

    }else{
      evaluate();
      data.base = 0;
      screen[1][15] = ' ';
      screen[1][14] = ' ';
    }
    memset(data.buffer,'0',data.counter);
    memset(screen[1],' ',data.counter);
  }
  if(data.enter && '0' <= p_key && p_key <= '9' && data.base < data.counter){
    data.buffer[data.base] = p_key;
    data.base += 1;
    // Serial.println(data.buffer);
    
  }
  if(data.enter && p_key == '#' && data.base > 0){
    data.buffer[--data.base] = '0';
    // Serial.println(data.buffer);
  }
  showDigit(data.buffer,data.base);
}
int ke = 1;
void pushButtons(){
  int key = analogRead(A0);
  if(data.read){
    if(key<580){
      data.next_location.x += dx;
      if(data.next_location.x > LIMIT_X) data.next_location.x -= dx;
      writeData();
    }
    else if (key < 750){
      data.next_location.x -= 1;
      if(data.next_location.x < 0) data.next_location.x += dx;
      writeData();
    }
    else if(key < 800){
        data.input = 'X' + (ke++)%3;
        // Serial.println(data.input);
        data.read = false;
        data.cur = millis();
    }
  }
  else{
    if(millis() - data.cur > data.dur) data.read = true;

  }
  //Serial.println(key);
}

void loop(){
  handleInput();
  pushButtons();
  display();
  if(!equal(&data.current_location,&data.next_location)){
    movDX(&data.current_location,&data.next_location);
  }
}