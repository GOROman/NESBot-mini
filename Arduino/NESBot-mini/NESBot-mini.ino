//
// NESBot-mini by @GOROman
//
// based on http://www.instructables.com/id/NESBot-Arduino-Powered-Robot-beating-Super-Mario-/
//

// ターゲット
#define USE_ESP32

//------------------------------------------------------------------------------------------------
// ESP32
//------------------------------------------------------------------------------------------------
#ifdef USE_ESP32
// ピンアサイン 
//      ESP32          -- CPU
#define PIN_RESET  15  // CPUの~RESET端子(3)

#define PIN_GND    23  // GND   4021  8

#define PIN_NMI    4   // CPU   NMI(VSYNC)
#define PIN_LATCH  2   // P/S   4021  9
#define PIN_CLK    5   // CLOCK

#define PIN_OUT   18

#define PIN_LED   13   // LED

#define INT0      PIN_LATCH
#define INT1      PIN_NMI
#define INT2      PIN_CLK
#else
#error ESP32 only supported
#endif

int bit = 0;


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
#include "HappyLee_SMB_TAS_latch.h"
//#include "adelikat-transformers_nmi.h"
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
  digitalWrite(PIN_RESET, HIGH);  delay(10);
}
byte outpad = 0x00;
int aaa = 0;
volatile byte padout;

int  bufcnt = 0;
byte buf[3] = {0xff};



void NES_Update()
{
  // RLEデコード処理
  if ( data_count == 0 ) {
    pad        = PADDATA[data_pos++];
    data_count = PADDATA[data_pos++];
    
    buf[1] = buf[2];
    buf[0] = buf[1];
    buf[0] = ~pad;
  } 
  data_count--;
  

}

// 割り込み処理(NMI)
void NES_INT_NMI()
{

  
  frame++;

}
// 割り込み処理(ラッチ)
void NES_INT_Latch()
{
  NES_Update();
  bit = 0;
  digitalWrite(PIN_OUT, (buf[0]) & (1<<bit));
  ++bit;


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

  pinMode(PIN_OUT,   OUTPUT);
  pinMode(PIN_RESET, OUTPUT);

  // ファミコン本体リセット
  NES_reset();

  // 割り込み開始
  attachInterrupt(INT0, NES_INT_Latch, FALLING); // 正論理
  attachInterrupt(INT1, NES_INT_NMI, RISING);   // 負論理
  attachInterrupt(INT2, NES_INT_CLK, RISING);  // 負論理

  time = micros();
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
    
    outputLog();

    if (data_pos == PADDATASIZE) {
      detachInterrupt(INT0);
      detachInterrupt(INT1);
      detachInterrupt(INT2);
    }  
  }

//  delayMicroseconds(100);
}

