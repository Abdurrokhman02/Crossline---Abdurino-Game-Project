#include <U8g2lib.h>
#include "assets.h"

// define pin
#define buzzer 10
#define button 3


#define jumpSound 700
#define DBOUNCE 180 //gap for button

// Game states
#define gameStart 0
#define gameEnd 1
#define gamePlaying 2

volatile int gameStatus = gameStart;

// Variabel game
int playerMove = 0;             // Posisi player naik/turun
int speed = 4; //kecepatan rintangan
int obstacles [2] = {0, 1}; //jenis rintangan
int obstaclex [2] = {128, 200}; //posisi rintangan
volatile int jumping = 0;       // Status lompat
unsigned long startTime = millis();

U8G2_SSD1306_128X64_NONAME_F_HW_I2C panel(U8G2_R0, U8X8_PIN_NONE);

// =======================================================================

void setup() {
  panel.begin();
  resetGame();
  
  pinMode(button, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(button), startStopGame, FALLING);
}

void loop() {
  panel.firstPage();
  do {
    draw();
  } while (panel.nextPage());

  if (gameStatus == gamePlaying){
    movePlayer();
    moveObstacles();
  }
}

// =======================================================================

void startStopGame(){
  static unsigned long last_interrupt = 0;
  if (millis() - last_interrupt > DBOUNCE){
    if (gameStatus == gamePlaying){
      if (jumping == 0){
        jumping = 1;
        tone(buzzer, jumpSound, 100);
      }
    }
    else if (gameStatus == gameStart) {
      resetGame();           // reset semua sebelum mulai
      gameStatus = gamePlaying;
    } 
    else {
      gameStatus = gameStart;
    }
  }
  last_interrupt = millis();
}


void resetGame(){
  startTime = millis();
  playerMove = 0;
  jumping = 0;
  obstaclex[0] = 128;
  obstaclex[1] = 200;
  obstacles[0] = 0;
  obstacles[1] = 1;
}


void movePlayer(){
  if (jumping == 0){
    playerMove = 0;  // diam di tanah
  } else {
    if (jumping == 1){
      playerMove += 5;         // naik
      if (playerMove > 25) jumping = 2;
    } else {
      playerMove -= 3;         // turun
      if (playerMove <= 0){
        jumping = 0;
        playerMove = 0;
      }
    }
  }
  checkCollision ();
}

void moveObstacles() {
  int obx = obstaclex [0];
  obx = obx - speed;
  if (obx < -20) {
    obstaclex[0] = obstaclex[1];
    obstaclex[1] = obstaclex[0] + random(80, 125);
    obstacles[0] = obstacles[1];
    obstacles[1] = random(0, 2); 
  }
  else {
    obstaclex[0] = obx;
    obstaclex[1] -= speed;
  }
}

// =======================================================================

void draw(){
  if (gameStatus == gamePlaying){
    drawPlayer();
    drawCloud();
    drawObstacles();
    panel.drawBox(0, 58, 128, 5);
  } 
  else if (gameStatus == gameStart){
    panel.setFont(u8g2_font_5x7_tr);
    panel.drawStr(32, 10, "ABDURINO GAME");

    panel.drawEllipse(64, 29, 29, 12);
    panel.drawStr(43, 33, "CROSSLINE");

    panel.setFont(u8g2_font_4x6_tr);
    panel.drawStr(20, 60, "press button to start");
  } 
  else { // gameEnd
    panel.setFont(u8g2_font_6x13B_tr);
    panel.drawStr(39, 37, "GAME OVER");
    drawPlayer();
    drawCloud();
    drawObstacles();
    panel.drawBox(0, 58, 128, 5);
  }
}


void drawPlayer(){
  // Player tetap di x=0, y naik/turun sesuai playerMove
  panel.drawXBM(2, 47 - playerMove, 9, 11, image_player_bits);
}

void drawCloud(){
  panel.drawXBM(2, 2, 24, 12, image_cloud_2right_bits);
  panel.drawXBM(39, 10, 17, 16, image_cloud_bits);
  panel.drawXBM(96, 3, 26, 15, image_Layer_2left_bits);
  panel.drawXBM(66, 1, 17, 16, image_cloud_bits);
}

void drawShape (int shape, int x) {
  switch (shape) {
    case 0: panel.drawXBM(x, 42, 19, 16, image_car_bits); break;
    case 1: panel.drawXBM(x, 42, 17, 16, image_bike_bits); break;
  }
}

void drawObstacles () {
  drawShape (obstacles[0], obstaclex[0]);
  drawShape (obstacles[1], obstaclex[1]);
}

void checkCollision () {
  int obx = obstaclex[0];
  int obw, obh;

  switch (obstacles[0]) {
    // Hanya ada case 0 dan 1
    case 0: obw = 19; obh = 16; break; // Ukuran mobil
    case 1: obw = 17; obh = 16; break; // Ukuran motor
  }

  // Logika pengecekan
  if (obx > 15 || obx + obw < 5 || playerMove > obh - 3) {
    // Aman
  } else {
    gameStatus = gameEnd;
    tone(buzzer, 125, 100);
    delay(150);
    tone(buzzer, 125, 100);
  }
}
