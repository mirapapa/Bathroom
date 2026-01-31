#include "common.h"

// 定数設定
// DIO-PIN設定
#define IR_SENSOR 4          // 赤外線センサ
#define REED_SW 15           // リードスイッチ
#define LED 2                // LED
#define EXHAUSTFAN_SWITCH 18 // 換気扇スイッチ
#define LIGHT_SWITCH 19      // 電気スイッチ
#define SWITCH_ON HIGH       // NC(Normally Closed)のため通常電気はONだが、
#define SWITCH_OFF LOW       // HIGHでON／LOWでOFFの動作をする
#define INTERVALTIME 100     // msec
#define DOOR_CLOSE 0         // ドア開閉状態（閉）
#define DOOR_OPEN 1          // ドア開閉状態（開）
#define DOOR_CHANGE 0x80     // ドア開閉状態（変更時）
#define BOOL_ON 1            // フラグON
#define BOOL_OFF 0           // フラグOFF

// 変数設定
uint16_t judgeOnTime = 100; // 人感知判定時間(msec) (intervalTimeの倍数で設定すること)
uint16_t startTime = 10;    // 人感知OFFからの換気扇開始時間(min)
uint16_t continueTime = 80; // 換気扇継続時間(hour)
enum PHASE
{
  PHASE_0,
  PHASE_1,
  PHASE_2,
  PHASE_3,
  PHASE_4,
  PHASE_5,
  PHASE_10,
  PHASE_11,
  PHASE_MAX
};
byte phase = PHASE_0;
time_t nowTime = 0;
time_t phaseStartTime[PHASE_MAX];
bool musicFlg = BOOL_OFF;
bool manualExhaustfanFlg = BOOL_OFF;
bool manualMusicFlg = BOOL_OFF;
bool exhaustfanState;

// 機器状態データ
DeviceState prev_deviceState;
SendRecvData recv_deviceState;
SendRecvData send_deviceState;

void sensor_setup()
{
  // DIO設定
  pinMode(IR_SENSOR, INPUT_PULLDOWN);
  pinMode(REED_SW, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  pinMode(EXHAUSTFAN_SWITCH, OUTPUT);
  pinMode(LIGHT_SWITCH, OUTPUT);
  digitalWrite(LED, LOW);
  setExhaustfanState(SWITCH_ON);
  digitalWrite(LIGHT_SWITCH, SWITCH_OFF);

  memset(&prev_deviceState, 0x01, sizeof(DeviceState));
  memset(&recv_deviceState, 0x00, sizeof(SendRecvData));
  memset(&send_deviceState, 0x00, sizeof(SendRecvData));
  // バージョン情報設定
  send_deviceState.version.id = 'V';
  sscanf(SYSTEM_VERSION, "%hhu.%hhu.%hhu",
         &send_deviceState.version.verMejor,
         &send_deviceState.version.verMinor,
         &send_deviceState.version.verPatch);

  // 設定値読み込み
  loadConfig();
}

void sensor_task(void *pvParameters)
{
  byte doorState = DOOR_CLOSE;
  byte irState = LOW;

  logprintln("main_task START!!");
  delay(100); // 各タスク起動待ち

  delay(30000); // 焦電型赤外線センサの回路安定待ち（30秒）
  logprintln("<<焦電型赤外線センサ回路安定 検知開始>>", 1);

  // アプリバージョン確定
  verifyFirmware();

  while (1)
  {
    nowTime = time(NULL);

    // ドア状態取得
    doorState = getDoorState(doorState);
    // 人感センサー状態取得
    irState = getIrState();

    // 各フェーズ処理
    switch (phase)
    {
    // PHASE_0 : 人を検知していない状態。
    // [電気：OFF、換気扇：ON 、音楽：OFF]
    case PHASE_0:
      if (doorState == (DOOR_OPEN | DOOR_CHANGE))
        migratePhase2();
      else if (doorState == (DOOR_CLOSE | DOOR_CHANGE))
      { /*NONE*/
      }
      if (irState)
        migratePhase2();
      else if (difftime(nowTime, phaseStartTime[PHASE_0]) >= (continueTime * 60 * 60))
        migratePhase1();
      break;

    // PHASE_1 : 人を検知していない状態が続き、換気扇が止まった状態。
    case PHASE_1:
      // [電気：OFF、換気扇：OFF、音楽：OFF]
      if (doorState == (DOOR_OPEN | DOOR_CHANGE))
        migratePhase2();
      else if (doorState == (DOOR_CLOSE | DOOR_CHANGE))
      { /*NONE*/
      }
      if (irState)
        migratePhase2();
      else
      { /*NONE*/
      }
      break;

    // PHASE_2 : 人を検知している状態。
    // [電気：ON 、換気扇：OFF、音楽：ON ]
    case PHASE_2:
      if (doorState == (DOOR_OPEN | DOOR_CHANGE))
      { /*NONE*/
      }
      else if (doorState == (DOOR_CLOSE | DOOR_CHANGE))
        migratePhase3();
      if (irState)
        phaseStartTime[PHASE_2] = nowTime;
      else if (difftime(nowTime, phaseStartTime[PHASE_2]) >= (3 * 60))
        migratePhase10();
      break;

    // PHASE_3 : ドアが閉まった後の判定不可能な状態。
    // [電気：-  、換気扇：-  、音楽：-  ]
    case PHASE_3:
      if (doorState == (DOOR_OPEN | DOOR_CHANGE))
        migratePhase2();
      else if (doorState == (DOOR_CLOSE | DOOR_CHANGE))
      { /*NONE*/
      }
      if (irState)
      { /*NONE*/
      }
      else if (difftime(nowTime, phaseStartTime[PHASE_3]) >= 3)
        migratePhase4();
      break;

    // PHASE_4 : ドアが閉まった後の判定中の状態。
    // [電気：-  、換気扇：-  、音楽：-  ]
    case PHASE_4:
      if (doorState == (DOOR_OPEN | DOOR_CHANGE))
        migratePhase2();
      else if (doorState == (DOOR_CLOSE | DOOR_CHANGE))
      { /*NONE*/
      }
      if (irState)
        migratePhase2();
      else if (difftime(nowTime, phaseStartTime[PHASE_4]) >= 7)
        migratePhase5();
      break;

    // PHASE_5 : ドアが閉まった後の仮判定中の状態。
    // [電気：OFF、換気扇：-  、音楽：-  ]
    case PHASE_5:
      if (doorState == (DOOR_OPEN | DOOR_CHANGE))
        migratePhase2();
      else if (doorState == (DOOR_CLOSE | DOOR_CHANGE))
      { /*NONE*/
      }
      if (irState)
        migratePhase2();
      else if (difftime(nowTime, phaseStartTime[PHASE_5]) >= 5)
        migratePhase0();
      break;

    // PHASE_10 : 確認アナウンス再生状態
    // [電気：-  、換気扇：-  、音楽：-  ]
    case PHASE_10:
      if (doorState == (DOOR_OPEN | DOOR_CHANGE))
      {
        // audio.stopSong();
        migratePhase2();
      }
      else if (doorState == (DOOR_CLOSE | DOOR_CHANGE))
      {
        // audio.stopSong();
        migratePhase3();
      }
      if (irState)
      {
        // audio.stopSong();
        // audio.connecttoFS(SPIFFS, "/Detected.mp3");
        migratePhase2();
      }
      else if (difftime(nowTime, phaseStartTime[PHASE_10]) >= 20)
        migratePhase11();
      break;

    // PHASE_11 : 警報発令状態
    // [電気：-  、換気扇：ON 、音楽：OFF]
    case PHASE_11:
      if (doorState == (DOOR_OPEN | DOOR_CHANGE))
        migratePhase2();
      else if (doorState == (DOOR_CLOSE | DOOR_CHANGE))
        migratePhase3();
      if (irState)
      { /*NONE*/
      }
      else
      { /*NONE*/
      }
      break;

    default:
      logprintln("▼▼▼フェーズ異常▼▼▼");
      break;
    }

    delay(INTERVALTIME);
  }

  vTaskDelete(NULL);
}

// ドア状態取得
byte getDoorState(byte pre_doorState)
{
  byte doorState = DOOR_CLOSE;
  byte reedDigital = digitalRead(REED_SW);
  if (reedDigital)
  {
    doorState = DOOR_OPEN;
    if ((pre_doorState & 0x7F) != reedDigital)
    {
      logprintln("=== ドア閉●⇒開○検知 ===");
      doorState |= DOOR_CHANGE;
    }
  }
  else
  {
    doorState = DOOR_CLOSE;
    if ((pre_doorState & 0x7F) != reedDigital)
    {
      logprintln("=== ドア開○⇒閉●検知 ===");
      doorState |= DOOR_CHANGE;
    }
  }
  return doorState;
}

// 人感センサー状態取得
byte getIrState()
{
  static uint16_t countOn = 0;
  static uint16_t countOff = 0;
  byte irState = LOW;

  // 人感知判定
  if (digitalRead(IR_SENSOR))
  {
    if (countOn == 0)
      logprintln(String("→→→→→ 人検知開始 countOn=") + String(countOn));
    if (countOn == (judgeOnTime / INTERVALTIME))
    {
      logprintln("=== 人感知ON判定 ===");
      digitalWrite(LED, HIGH);
      countOff = 0;
    }
    if (countOn >= (judgeOnTime / INTERVALTIME))
      irState = HIGH;
    if (countOn < 0xFFFF)
      countOn++;
  }
  else
  {
    if (countOn != 0)
      logprintln(String("人検知終了 ←←←←← countOn=") + String(countOn));
    digitalWrite(LED, LOW);
    countOn = 0;
    if (countOff < 0xFFFF)
      countOff++;
  }
  return irState;
}

// フェーズ0移行
void migratePhase0()
{
  logprintln("■フェーズ０へ移行■");
  detectOFF();
  phaseStartTime[PHASE_0] = nowTime;
  phase = PHASE_0;
}
// フェーズ1移行
void migratePhase1()
{
  logprintln("■フェーズ１へ移行■");
  logprintln("<<浴室換気完了 ⇒ 電気ＯＦＦ 換気扇ＯＦＦ>>", 1);
  digitalWrite(LIGHT_SWITCH, SWITCH_OFF);
  setExhaustfanState(SWITCH_OFF);
  phaseStartTime[PHASE_1] = nowTime;
  phase = PHASE_1;
}
// フェーズ2移行
void migratePhase2()
{
  logprintln("■フェーズ２へ移行■");
  detectON();
  phaseStartTime[PHASE_2] = nowTime;
  phase = PHASE_2;
}
// フェーズ3移行
void migratePhase3()
{
  logprintln("■フェーズ３へ移行■");
  phaseStartTime[PHASE_3] = nowTime;
  phase = PHASE_3;
}
// フェーズ4移行
void migratePhase4()
{
  logprintln("■フェーズ４へ移行■");
  phaseStartTime[PHASE_4] = nowTime;
  phase = PHASE_4;
}

// フェーズ5移行
void migratePhase5()
{
  logprintln("■フェーズ５へ移行■");
  digitalWrite(LIGHT_SWITCH, SWITCH_OFF);
  phaseStartTime[PHASE_5] = nowTime;
  phase = PHASE_5;
}

// フェーズ10移行
void migratePhase10()
{
  logprintln("■フェーズ１０へ移行■");
  // audio.connecttoFS(SPIFFS, "/Not_detected.mp3");
  phaseStartTime[PHASE_10] = nowTime;
  phase = PHASE_10;
}

// フェーズ11移行
void migratePhase11()
{
  logprintln("■フェーズ１１へ移行■");
  setExhaustfanState(SWITCH_ON);
  phaseStartTime[PHASE_11] = nowTime;
  musicFlg = BOOL_OFF;
  alexaChangeReport(BOOL_OFF);
  phase = PHASE_11;
}

// 検知ONの時に実行する内容
void detectON()
{
  logprintln("<<人感知ＯＮ　 ⇒ 電気ＯＮ　 換気扇ＯＦＦ>>", 1);
  digitalWrite(LIGHT_SWITCH, SWITCH_ON);
  setExhaustfanState(SWITCH_OFF);
  if (musicFlg == BOOL_OFF)
  {
    musicFlg = BOOL_ON;
    if (manualMusicFlg == BOOL_OFF)
      alexaChangeReport(BOOL_ON);
  }
}
// 検知OFFの時に実行する内容
void detectOFF()
{
  logprintln("<<人感知ＯＦＦ ⇒ 電気ＯＦＦ 換気扇ＯＮ　>>", 1);
  digitalWrite(LIGHT_SWITCH, SWITCH_OFF);
  setExhaustfanState(SWITCH_ON);
  musicFlg = BOOL_OFF;
  alexaChangeReport(BOOL_OFF);
}

// フェーズ０設定
void setMigratePhase0()
{
  nowTime = time(NULL);
  migratePhase0();
}

// 換気扇のON/OFF状態を設定
void setExhaustfanState(bool state)
{
  exhaustfanState = state;
  if (manualExhaustfanFlg == BOOL_OFF)
    digitalWrite(EXHAUSTFAN_SWITCH, exhaustfanState);
}

// 電気のON/OFF状態を取得
bool getLightMode()
{
  //  return digitalRead(LIGHT_SWITCH);
  return (*portOutputRegister(digitalPinToPort(LIGHT_SWITCH)) & digitalPinToBitMask(LIGHT_SWITCH));
}
// 換気扇のON/OFF状態を取得
bool getExhaustFanMode()
{
  //  return digitalRead(EXHAUSTFAN_SWITCH);
  return (*portOutputRegister(digitalPinToPort(EXHAUSTFAN_SWITCH)) & digitalPinToBitMask(EXHAUSTFAN_SWITCH));
}
// 換気扇手動モード切替
void manualExhaustfan()
{
  if (manualExhaustfanFlg == BOOL_ON)
    digitalWrite(EXHAUSTFAN_SWITCH, SWITCH_ON);
  else
    digitalWrite(EXHAUSTFAN_SWITCH, exhaustfanState);
}
// 音楽手動モード切替
void manualMusic()
{
  // 手動モードONの場合OFFにする
  if (manualMusicFlg == BOOL_ON)
  {
    if ((musicFlg == BOOL_ON) && (nowMusicFlg == BOOL_ON))
      alexaChangeReport(BOOL_OFF);
  }
  else
  {
    if ((musicFlg == BOOL_ON) && (nowMusicFlg == BOOL_OFF))
      alexaChangeReport(BOOL_ON);
  }
}

// NVSから設定を読み込む
void loadConfig()
{
  preferences.begin("system", true); // "system"という名前の領域を読み取り専用で開く
  // 保存されていなければ第2引数のデフォルト値が使われる
  judgeOnTime = preferences.getShort("judgeOnTime", 200);
  startTime = preferences.getShort("startTime", 10);
  continueTime = preferences.getShort("continueTime", 80);
  preferences.end();
}

// NVSに現在の設定を保存する
void saveConfig()
{
  preferences.begin("system", false); // 書き込みモードで開く
  preferences.putShort("judgeOnTime", judgeOnTime);
  preferences.putShort("startTime", startTime);
  preferences.putShort("continueTime", continueTime);
  preferences.end();
  logprintln("[SYSTEM] Config saved to NVS");
}