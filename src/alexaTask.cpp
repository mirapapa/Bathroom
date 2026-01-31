#include "common.h"

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

String refleshtoken()
{
  WiFiClientSecure https_refleshtoken_client;

  https_refleshtoken_client.setInsecure();
  if (https_refleshtoken_client.connect(HOST_REFRESH_TOKEN, 443))
  {
    String postData = "grant_type=refresh_token&refresh_token=" + REFRESH_TOKEN +
                      "&client_id=" + ALEXA_CLIENT_ID_STR +
                      "&client_secret=" + ALEXA_CLIENT_SECRET_STR;
    String header = "POST /auth/o2/token HTTP/1.1"
                    "\n"
                    "Content-Length: " +
                    String(postData.length()) + "\n"
                                                "Host: " +
                    HOST_REFRESH_TOKEN + "\n"
                                         "Content-Type: application/x-www-form-urlencoded;Accept-Charset=UTF-8"
                                         "\n"
                                         "\n";
    https_refleshtoken_client.print(header + postData);
    Serial.print(header + postData);
  }
  else
  {
    Serial.println(F("------connection failed"));
    https_refleshtoken_client.stop();
    return "";
  }
  String res = https_refleshtoken_client.readString();
  // Serial.println(res);
  String serchWord = "access_token";
  int po_start_serchWord = res.indexOf(serchWord);
  String access_token;
  if (po_start_serchWord)
  {
    int po_start_access_token = res.indexOf("\"", po_start_serchWord + serchWord.length() + 1);
    int po_end_access_token = res.indexOf("\"", po_start_access_token + 1);
    access_token = res.substring(po_start_access_token + 1, po_end_access_token);
  }
  else
  {
    return "";
  }
  logprintln("access_token : " + access_token);
  https_refleshtoken_client.stop();

  return access_token;
}

void changereport(String access_token, bool detected)
{
  if (access_token == "")
    return;

  WiFiClientSecure https_changereport_client;
  String sensor_state = detected ? "DETECTED" : "NOT_DETECTED";
  https_changereport_client.setInsecure();
  if (https_changereport_client.connect(HOST_CHANGEREPORT, 443))
  {
    String postData2 = "{\"context\":{},\"event\":{\"header\":{\"messageId\":\"tkr-123-def-456\",\"namespace\":\"Alexa\",\"name\":\"ChangeReport\",\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"" +
                       access_token +
                       "\"},\"endpointId\":\"sensor-001\"},\"payload\":{\"change\":{\"cause\":{\"type\":\"PHYSICAL_INTERACTION\"},\"properties\":[{\"namespace\":\"Alexa.MotionSensor\",\"name\":\"detectionState\",\"value\":\"" +
                       sensor_state +
                       "\",\"timeOfSample\":\"2020-03-22T16:20:50.52Z\",\"uncertaintyInMilliseconds\":0}]}}}}";
    String header2 = "POST /v3/events HTTP/1.1"
                     "\n"
                     "Authorization: Bearer " +
                     access_token + "\n"
                                    "Content-Length: " +
                     postData2.length() + "\n"
                                          "Content-Type: application/json\nHost: api.fe.amazonalexa.com"
                                          "\n"
                                          "\n";
    https_changereport_client.print(header2 + postData2);
    Serial.print(header2 + postData2);
  }
  else
  {
    Serial.println(F("------connection failed"));
    https_changereport_client.stop();
    return;
  }
  String res2 = https_changereport_client.readString();
  Serial.println(res2);
  https_changereport_client.stop();

  nowMusicFlg = detected;
  return;
}

void alexaChangeReport(bool detected)
{
#ifdef DEBUG
#else
  xQueueSend(queue2, &detected, 0);
#endif
}
