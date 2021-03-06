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
#define PIN_RESET  23   // CPU ~RESET端子(3)
#define PIN_NMI     4   // CPU  NMI(VSYNC)

#define PIN_GND         // EXT. 1 GND
#define PIN_LATCH   2   // EXT.12 Strobe    (4021 P/S)
#define PIN_OUT    18   // EXT.13 PAD DATA  
#define PIN_CLK     5   // EXT.14 ~OE CLOCK (4021 CLK)

#define INT0      PIN_LATCH
#define INT1      PIN_NMI
#define INT2      PIN_CLK
#else
  #error ESP32 only supported
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
//#include "HappyLee_SMB_TAS_latch.h"
//#include "adelikat-transformers_latch.h"
};

const int PADDATASIZE = sizeof(PADDATA)/sizeof(PADDATA[0]);

int  frame = 0;
int  latch = 0;
int  clock = 0;
int  old_latch = -1;
int  old_frame = -1;

int  data_pos   = 0;
int  data_count = 0;
byte pad        = 0x00;
int  bit = 0;

int  time = 0;
int  old_time = 0;

// ファミコン本体リセット
void NES_reset()
{
  digitalWrite(PIN_RESET, HIGH);
  digitalWrite(PIN_RESET,  LOW);  delay(100);
  digitalWrite(PIN_RESET, HIGH); 
}

void NES_Update()
{
  // RLEデコード処理
  if ( data_count == 0 ) {
    pad        = ~PADDATA[data_pos++]; // 負論理へ
    data_count = PADDATA[data_pos++];
  } 
  data_count--;
}

// 割り込み処理(NMI)
void NES_INT_NMI()
{
  NES_Update();

  // 4021的な処理
  bit = 0;
  digitalWrite(PIN_OUT, pad & (1<<bit));
  ++bit;

  frame++;
}

// 割り込み処理(ラッチ)
void NES_INT_Latch()
{
  latch++; 
}

void NES_INT_CLK()
{
  // 4021的な処理
  digitalWrite(PIN_OUT, pad & (1<<bit));
  ++bit;

  ++clock;
}

// セットアップ
void setup()
{
  pinMode(PIN_OUT,   OUTPUT);
  pinMode(PIN_RESET, OUTPUT);

  // ファミコン本体リセット
  NES_reset();

  // 割り込み開始
  attachInterrupt(INT0, NES_INT_Latch, FALLING);
  attachInterrupt(INT1, NES_INT_NMI, RISING);
  attachInterrupt(INT2, NES_INT_CLK, RISING);

  time = micros();
}

// ログ出力
void outputLog()
{
  printf("%5d:%5d:|0|%c%c%c%c%c%c%c%c| [%3d (%5d)] - %5.2fms (%5.2fms) %d\n"
    ,frame, latch
    ,pad & PAD_RIGHT  ? '.':'R'
    ,pad & PAD_LEFT   ? '.':'L'
    ,pad & PAD_DOWN   ? '.':'D'
    ,pad & PAD_UP     ? '.':'U'
    ,pad & PAD_START  ? '.':'T'
    ,pad & PAD_SELECT ? '.':'S'
    ,pad & PAD_B      ? '.':'B'
    ,pad & PAD_A      ? '.':'A'
    ,data_pos/2-1, data_count+1
    ,time*0.001f, (time - old_time)*0.001f
    ,clock
    );
}

// メインループ
void loop() {
  if ( latch != old_latch ) {
    old_latch = latch;
    old_time  = time;
    time      = micros();
  }

  if ( frame != old_frame ) {
    old_frame = frame;
    if (data_pos == PADDATASIZE) {
      detachInterrupt(INT0);
      detachInterrupt(INT1);
      detachInterrupt(INT2);
    }  
    outputLog();
  }
}

