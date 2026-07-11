#include "alexaTask.h"
#include "logging.h"

#define MAX_QUE2NUM 1
#define MAX_QUE2SIZE 1

const String AcceptGrant_Code = "SRuDQFGcxbxdhUHiBXdz"; // 未使用（メモ）
const String REFRESH_TOKEN = ALEXA_REFRESH_TOKEN;
const String ALEXA_CLIENT_ID_STR = ALEXA_CLIENT_ID;
const String ALEXA_CLIENT_SECRET_STR = ALEXA_CLIENT_SECRET;
const char *HOST_REFRESH_TOKEN = ALEXA_HOST_REFRESH_TOKEN;
const char *HOST_CHANGEREPORT = ALEXA_HOST_CHANGEREPORT;

QueueHandle_t queue2;
bool nowMusicFlg = 0;

// このファイル内でのみ使用する内部関数
static String refleshtoken();
static void changereport(String access_token, bool detected);
static String getIso8601UtcTimeStr();

void alexasetup()
{
  // alexa用のキューを生成
  queue2 = xQueueCreate(MAX_QUE2NUM, MAX_QUE2SIZE);
}

void alexa_task(void *pvParameters)
{
  String amazonToken;
  time_t pre_getTokenTime = 0;

  logprintln("alexa_task START!!");
  delay(100); // 各タスク起動待ち

  while (1)
  {
    bool detect;

    if (xQueueReceive(queue2, &detect, 0))
    {
#ifdef DEBUG
#else
      if (difftime(time(NULL), pre_getTokenTime) >= (25 * 60))
      {
        amazonToken = refleshtoken();
        if (amazonToken != "")
          pre_getTokenTime = time(NULL);
      }
      changereport(amazonToken, detect);
#endif
    }
    delay(100);
  }
  vTaskDelete(NULL);
}

static String refleshtoken()
{
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  if (!http.begin(client, String("https://") + HOST_REFRESH_TOKEN + "/auth/o2/token"))
  {
    logprintln("[Alexa] refleshtoken: begin failed");
    return "";
  }
  http.addHeader("Content-Type", "application/x-www-form-urlencoded;Accept-Charset=UTF-8");

  String postData = "grant_type=refresh_token&refresh_token=" + REFRESH_TOKEN +
                     "&client_id=" + ALEXA_CLIENT_ID_STR +
                     "&client_secret=" + ALEXA_CLIENT_SECRET_STR;

  int httpCode = http.POST(postData);
  if (httpCode != HTTP_CODE_OK)
  {
    logprintln("[Alexa] refleshtoken: request failed (code " + String(httpCode) + ")");
    http.end();
    return "";
  }

  String res = http.getString();
  http.end();

  String serchWord = "access_token";
  int po_start_serchWord = res.indexOf(serchWord);
  if (po_start_serchWord < 0)
    return "";

  int po_start_access_token = res.indexOf("\"", po_start_serchWord + serchWord.length() + 1);
  int po_end_access_token = res.indexOf("\"", po_start_access_token + 1);
  String access_token = res.substring(po_start_access_token + 1, po_end_access_token);

  logprintln("access_token : " + access_token);
  return access_token;
}

// 現在時刻をAlexa ChangeReport用のISO8601(UTC)文字列に変換
static String getIso8601UtcTimeStr()
{
  time_t t = time(NULL);
  struct tm tm;
  gmtime_r(&t, &tm);
  char str[32];
  strftime(str, sizeof(str), "%Y-%m-%dT%H:%M:%S.00Z", &tm);
  return String(str);
}

static void changereport(String access_token, bool detected)
{
  if (access_token == "")
    return;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  if (!http.begin(client, String("https://") + HOST_CHANGEREPORT + "/v3/events"))
  {
    logprintln("[Alexa] changereport: begin failed");
    return;
  }
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + access_token);

  String sensor_state = detected ? "DETECTED" : "NOT_DETECTED";
  String postData2 = "{\"context\":{},\"event\":{\"header\":{\"messageId\":\"tkr-123-def-456\",\"namespace\":\"Alexa\",\"name\":\"ChangeReport\",\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"" +
                       access_token +
                       "\"},\"endpointId\":\"sensor-001\"},\"payload\":{\"change\":{\"cause\":{\"type\":\"PHYSICAL_INTERACTION\"},\"properties\":[{\"namespace\":\"Alexa.MotionSensor\",\"name\":\"detectionState\",\"value\":\"" +
                       sensor_state +
                       "\",\"timeOfSample\":\"" + getIso8601UtcTimeStr() +
                       "\",\"uncertaintyInMilliseconds\":0}]}}}}";

  int httpCode = http.POST(postData2);
  if (httpCode <= 0)
  {
    logprintln("[Alexa] changereport: request failed (" + http.errorToString(httpCode) + ")");
    http.end();
    return;
  }

  logprintln("[Alexa] changereport response(" + String(httpCode) + "): " + http.getString());
  http.end();

  nowMusicFlg = detected;
}

void alexaChangeReport(bool detected)
{
#ifdef DEBUG
#else
  xQueueSend(queue2, &detected, 0);
#endif
}
