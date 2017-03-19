//
// NESBot-mini by @GOROman
//
// based on http://www.instructables.com/id/NESBot-Arduino-Powered-Robot-beating-Super-Mario-/
//

// ターゲット
#define USE_ESP32
//#define USE_ARDUINO_UNO

//------------------------------------------------------------------------------------------------
// ESP32
//------------------------------------------------------------------------------------------------
#ifdef USE_ESP32
// ピンアサイン 
//      ESP32          -- CPU
#define PIN_RESET  15  // CPUの~RESET端子(3)
#define PIN_NMI     0  // CPU NMI
//      ESP32          -- P1 Pad
#define PIN_RIGHT   4  // BIT.7 4021  7
#define PIN_LEFT   16  // BIT.6 4021  6
#define PIN_DOWN   17  // BIT.5 4021  5
#define PIN_UP      5  // BIT.4 4021  4
#define PIN_START  18  // BIT.3 4021 13
#define PIN_SELECT 19  // BIT.2 4021 14
#define PIN_B      22  // BIT.1 4021 15
#define PIN_A      23  // BIT.0 4021  1
#define PIN_GND        // GND   4021  8
#define PIN_LATCH  2   // P/S   4021  9
#define PIN_LED   13   // LED
#define PIN_CLK    2
#define PIN_OUT   32

#define INT0      PIN_LATCH
#define INT1      PIN_NMI
#define INT2      PIN_CLK
#endif

int bit = 0;

//------------------------------------------------------------------------------------------------
//      Arduino Uno
//------------------------------------------------------------------------------------------------
#ifdef ARDUINO_UNO
//      Arduino Uno   -- CPU
#define PIN_RESET 11  // CPUの~RESET端子(3)
//      Arduino Uno   -- P1 Pad
#define PIN_RIGHT 10  // BIT.7 4021  7
#define PIN_LEFT   9  // BIT.6 4021  6
#define PIN_DOWN   8  // BIT.5 4021  5
#define PIN_UP     7  // BIT.4 4021  4
#define PIN_START  6  // BIT.3 4021 13
#define PIN_SELECT 5  // BIT.2 4021 14
#define PIN_B      4  // BIT.1 4021 15
#define PIN_A      3  // BIT.0 4021  1
#define PIN_5V        // VCC   4021 16
#define PIN_GND       // GND   4021  8
#define PIN_LATCH  2  // P/S   4021  9
#define PIN_LED   13  // LED

#define INT0      PIN_LATCH
#define INT1      PIN_NMI
#endif

// パッドデータ
#define PAD_RIGHT  (1<<7)
#define PAD_LEFT   (1<<6)
#define PAD_DOWN   (1<<5)
#define PAD_UP     (1<<4)
#define PAD_START  (1<<3)
#define PAD_SELECT (1<<2)
#define PAD_B      (1<<1)
#define PAD_A      (1<<0)

const byte PADDATA[] = {
#include "HappyLee_SMB_TAS.h"
//#include "adelikat-transformers.h"
};

const int PADDATASIZE = sizeof(PADDATA)/sizeof(PADDATA[0]);

int  frame = 0;
int  latch = 0;
int  old_latch = -1;
int  old_frame = -1;

int  data_pos   = 0;
int  data_count = 0;
byte pad        = 0x00;

int time = 0;
int old_time = 0;

// ファミコン本体リセット
void NES_reset()
{
  digitalWrite(PIN_RESET, HIGH);  delay(10);
  digitalWrite(PIN_RESET,  LOW);  delay(10);
  digitalWrite(PIN_RESET, HIGH);  
}
byte outpad = 0x00;
int aaa = 0;
volatile byte padout;

int  bufcnt = 0;
byte buf[3] = {0};

// 割り込み処理(NMI)
void NES_INT_NMI()
{

      // RLEデコード処理
    if ( data_count == 0 ) {
      pad        = PADDATA[data_pos++];
      data_count = PADDATA[data_pos++];

//      buf[0] = 0x55;
//      buf[1] = 0xaa;
buf[1] = buf[2];
buf[0] = buf[1];
      buf[0] = ~pad;
     } 
    data_count--;

  frame++;

  
  // パッド出力
  writeButtons(pad);

  bit = 0;
  digitalWrite(PIN_OUT, (buf[0]) & (1<<bit));
  ++bit;

}


// 割り込み処理(ラッチ)
void NES_INT_Latch()
{
  latch++; 

}
int clock= 0;
void NES_INT_CLK()
{
    digitalWrite(PIN_OUT, (buf[0]) & (1<<bit));
    ++bit;

  ++clock;
}

// セットアップ
void setup() {
  pinMode(PIN_RESET,  OUTPUT);
  pinMode(PIN_RIGHT,  OUTPUT);
  pinMode(PIN_LEFT,   OUTPUT);
  pinMode(PIN_DOWN,   OUTPUT);
  pinMode(PIN_UP,     OUTPUT);
  pinMode(PIN_START,  OUTPUT);
  pinMode(PIN_SELECT, OUTPUT);
  pinMode(PIN_B,      OUTPUT);
  pinMode(PIN_A,      OUTPUT);

  pinMode(PIN_OUT,    OUTPUT);
//  pinMode(PIN_CLK,    INPUT);

  writeButtons(0x00);

  // ファミコン本体リセット
  NES_reset();

  // 割り込み開始
  attachInterrupt(INT1, NES_INT_NMI, RISING);   // 負論理
  attachInterrupt(INT0, NES_INT_Latch, FALLING); // 正論理
  attachInterrupt(INT2, NES_INT_CLK, RISING);  // 負論理

  time = micros();
}

// ボタン状態出力
void writeButtons(byte buttons)
{
  buttons = ~buttons; // 負論理へ

  digitalWrite(PIN_A,      buttons & PAD_A     );
  digitalWrite(PIN_B,      buttons & PAD_B     );
  digitalWrite(PIN_LEFT,   buttons & PAD_LEFT  );
  digitalWrite(PIN_RIGHT,  buttons & PAD_RIGHT );

  digitalWrite(PIN_SELECT, buttons & PAD_SELECT);
  digitalWrite(PIN_START,  buttons & PAD_START );
  digitalWrite(PIN_UP,     buttons & PAD_UP    );
  digitalWrite(PIN_DOWN,   buttons & PAD_DOWN  );
}

void outputLog()
{
  printf("%5d:%5d:|0|%c%c%c%c%c%c%c%c| [%3d (%5d)] - %5.2fms (%5.2fms) %d\n"
    ,frame, latch
    ,pad & PAD_RIGHT  ? 'R':'.'
    ,pad & PAD_LEFT   ? 'L':'.'
    ,pad & PAD_DOWN   ? 'D':'.'
    ,pad & PAD_UP     ? 'U':'.'
    ,pad & PAD_START  ? 'T':'.'
    ,pad & PAD_SELECT ? 'S':'.'
    ,pad & PAD_B      ? 'B':'.'
    ,pad & PAD_A      ? 'A':'.'
    ,data_pos/2-1, data_count+1
    ,time*0.001f, (time - old_time)*0.001f
    ,clock
    );
    old_frame = frame;
}

// メインループ
void loop() {
  if ( latch != old_latch ) {
    old_time  = time;
    time = micros();
    old_latch = latch;
  }

  if ( frame != old_frame ) {

    
    if (data_pos == PADDATASIZE) {
      writeButtons(0);
      detachInterrupt(INT0);
      detachInterrupt(INT1);
    }  

    outputLog();
  }

//  delayMicroseconds(100);
}

