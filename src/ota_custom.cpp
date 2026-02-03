#include "common.h"
#include "ota_custom.h"

// 最終的なHTMLを生成する関数
String getOtaHtml()
{
    String specific = String(SPECIFIC_PART);

    specific.replace("{{JUDGEONTIME}}", String(judgeOnTime));
    specific.replace("{{STARTTIME}}", String(startTime));
    specific.replace("{{CONTINUETIME}}", String(continueTime));
    specific.replace("{{HISTORYDATA}}", getHistoryData());

    return specific;
}

void ota_setup_custom(WebServer &webOtaServer)
{
    // 判別時間 (seconds)
    webOtaServer.on("/judgeOnTimeUp", []()
                    { handleConfigUpdate(judgeOnTime, 100, 100, 10000); });
    webOtaServer.on("/judgeOnTimeDown", []()
                    { handleConfigUpdate(judgeOnTime, -100, 100, 10000); });

    // 開始時間 (minutes)
    webOtaServer.on("/startTimeUp", []()
                    { handleConfigUpdate(startTime, 1, 1, 65535); });
    webOtaServer.on("/startTimeDown", []()
                    { handleConfigUpdate(startTime, -1, 1, 65535); });

    // 継続時間 (minutes)
    webOtaServer.on("/continueTimeUp", []()
                    { handleConfigUpdate(continueTime, 1, 1, 65535); });
    webOtaServer.on("/continueTimeDown", []()
                    { handleConfigUpdate(continueTime, -1, 1, 65535); });
    webOtaServer.begin();
}

String otaProcessor_custom(String html)
{
    html.replace("{{STARTTIME}}", String(startTime));
    html.replace("{{CONTINUETIME}}", String(continueTime));
    html.replace("{{JUDGEONTIME}}", String(judgeOnTime));
    html.replace("{{HISTORYDATA}}", getHistoryData());

    return html;
}