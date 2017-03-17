//
// NESBot-mini by @GOROman
//
// based on http://www.instructables.com/id/NESBot-Arduino-Powered-Robot-beating-Super-Mario-/
//

// FlashROM プログラム領域アクセス用
#include <avr/pgmspace.h>

// ピンアサイン 
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

// パッドデータ
#define PAD_RIGHT  (1<<7)
#define PAD_LEFT   (1<<6)
#define PAD_DOWN   (1<<5)
#define PAD_UP     (1<<4)
#define PAD_START  (1<<3)
#define PAD_SELECT (PAD_UP|PAD_DOWN)  // SELECTはパッド上下同時押し扱い
#define PAD_B      (1<<1)
#define PAD_A      (1<<0)

#define PAD_RLEFLAG (1<<2)            // RLE有効フラグ(この次の1バイト分同じ入力が連続する)

// Super Mario Bros. TASクリア パッドデータ
const PROGMEM unsigned char PADDATA[] = {
#include "HappyLee_SMB_TAS.h"
};
const int movie_length = sizeof(PADDATA);

// パッドデータ展開用バッファ(ダブルバッファ)
#define WORKSIZE 8
byte buffer_data[2][WORKSIZE] = { 0 };

volatile int  buffer_index  = 0;          // ダブルバッファインデクス
volatile int  buffer_offset = 0;          // FlashROMオフセット位置
volatile bool buffer_read_request = true; // FlashROMからのロードリクエスト

volatile int  pos   = 0;
volatile int  frame = 0;
volatile int  count = 0;
volatile byte pad   = 0x00;

// ファミコン本体リセット
void NES_reset()
{
  writeButtons(0x00);

  digitalWrite(PIN_RESET, HIGH);
  delay (100);
  digitalWrite(PIN_RESET, LOW);
  delay (100);
  digitalWrite(PIN_RESET, HIGH);
}

// パッドデータ読み出し
byte NES_read_buffer()
{
  byte data = buffer_data[buffer_index][pos];
  pos++;
  if ( pos >= WORKSIZE ) {
    pos = 0;
    buffer_index ^= 1;
    buffer_read_request = true;
  }
  return data;
}
// 割り込み処理(ラッチ)
void NES_latch_pulse()
{
  // RLEデコード処理
  if ( count == 0 ) {
    pad = NES_read_buffer();
    if ( pad & PAD_RLEFLAG ) { // RLE flag?
      pad &= ~PAD_RLEFLAG;
      count = NES_read_buffer();
    } else {
      count = 1;
    }
  }

  // パッド出力
  writeButtons(pad);

  count--;
  frame++;

  if (pos == movie_length) {
    writeButtons(0);
    detachInterrupt(0);
  }  
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

  // FlashROM領域からSRAMへプリロードしておく
  buffer_offset = readFromFlash( buffer_data[0], buffer_offset, WORKSIZE );

  // ファミコン本体リセット
  NES_reset();

  // 割り込み開始
  attachInterrupt(0, NES_latch_pulse, FALLING);
}

// ボタン状態出力
void writeButtons(byte buttons)
{
  buttons = ~buttons; // 負論理へ

  digitalWrite(PIN_A,      buttons & PAD_A     );
  digitalWrite(PIN_B,      buttons & PAD_B     );
  digitalWrite(PIN_SELECT, (buttons & PAD_SELECT) == PAD_SELECT);
  digitalWrite(PIN_START,  buttons & PAD_START );
  digitalWrite(PIN_UP,     buttons & PAD_UP    );
  digitalWrite(PIN_DOWN,   buttons & PAD_DOWN  );
  digitalWrite(PIN_LEFT,   buttons & PAD_LEFT  );
  digitalWrite(PIN_RIGHT,  buttons & PAD_RIGHT );
}

// フラッシュメモリから読み込み
int readFromFlash( byte* work, int offset, int size )
{
  for ( int i=0; i<size; ++i ) {
    work[i] = pgm_read_byte_near( PADDATA + offset + i );
  }
  return offset + size;
}
// メインループ
void loop() {
  // FlashROMからSRAMへの読み出しリクエストがあった場合はロードする
  if ( buffer_read_request ) {
    buffer_offset = readFromFlash( buffer_data[buffer_index^1], buffer_offset, WORKSIZE );
    buffer_read_request = false;
  }
}

