#include "ota_common.h"

// 換気扇システム固有のHTMLパーツ
const char SPECIFIC_PART[] PROGMEM = R"rawliteral(
<div class='config-grid'>
    <div class='config-card'>
        <div>人感知時間(ミリ秒)</div>
        <div style='font-size:20px; font-weight:bold;' id='val_judge'>{{JUDGEONTIME}}</div>
        <div class='btn-group'>
            <button class='btn-sm' onclick='location.href="/judgeOnTimeDown"'>-</button>
            <button class='btn-sm' onclick='location.href="/judgeOnTimeUp"'>+</button>
        </div>
    </div>
    <div class='config-card'>
        <div>警報開始時間(分)</div>
        <div style='font-size:20px; font-weight:bold;' id='val_start'>{{STARTTIME}}</div>
        <div class='btn-group'>
            <button class='btn-sm' onclick='location.href="/startTimeDown"'>-</button>
            <button class='btn-sm' onclick='location.href="/startTimeUp"'>+</button>
        </div>
    </div>
    <div class='config-card' style='grid-column: span 2;'>
        <div>換気扇継続時間(時間)</div>
        <div style='font-size:20px; font-weight:bold;' id='val_continue'>{{CONTINUETIME}}</div>
        <div class='btn-group'>
            <button class='btn-sm' onclick='location.href="/continueTimeDown"'>-</button>
            <button class='btn-sm' onclick='location.href="/continueTimeUp"'>+</button>
        </div>
    </div>
</div>
<details style='margin-bottom: 20px;'>
    <summary style='cursor: pointer; padding: 10px; background: #f0f4ff; border-radius: 8px; font-weight: bold;'>
        📜 ログ履歴
    </summary>
    <div class='history-box' id='history'>{{HISTORYDATA}}</div>
</details>
)rawliteral";