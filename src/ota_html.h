#ifndef OTA_HTML_H
#define OTA_HTML_H

#include <Arduino.h>

    const char otaUploadHtml[] PROGMEM = R"rawliteral(
    <!DOCTYPE html>
    <html lang='ja'>

    <head>
        <meta charset='UTF-8'>
        <meta name='viewport' content='width=device-width, initial-scale=1.0'>
        <title>{{SYS_NAME}} - 管理パネル</title>
        <style>
            /* 既存のスタイルを継承しつつ追加 */
            body {
                font-family: 'Segoe UI', sans-serif;
                margin: 0;
                padding: 20px;
                background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
                min-height: 100vh;
                display: flex;
                justify-content: center;
                align-items: center;
            }

            .container {
                background: white;
                padding: 30px;
                border-radius: 15px;
                box-shadow: 0 10px 40px rgba(0, 0, 0, 0.2);
                max-width: 600px;
                width: 100%;
            }

            h1 {
                color: #667eea;
                text-align: center;
                font-size: 24px;
            }

            .info-box {
                background: #f0f4ff;
                padding: 15px;
                border-radius: 8px;
                margin-bottom: 15px;
                border-left: 4px solid #667eea;
            }

            .config-grid {
                display: grid;
                grid-template-columns: 1fr 1fr;
                gap: 10px;
                margin-bottom: 20px;
            }

            .config-card {
                background: #f8f9fa;
                padding: 10px;
                border-radius: 8px;
                text-align: center;
                border: 1px solid #ddd;
            }

            .btn-group {
                display: flex;
                justify-content: center;
                gap: 10px;
                margin-top: 5px;
            }

            .btn-sm {
                padding: 5px 15px;
                cursor: pointer;
                border-radius: 4px;
                border: none;
                background: #667eea;
                color: white;
                font-weight: bold;
            }

            .btn-sm:hover {
                background: #764ba2;
            }

            .history-box {
                background: #333;
                color: #0f0;
                padding: 10px;
                border-radius: 5px;
                font-family: monospace;
                font-size: 12px;
                max-height: 150px;
                overflow-y: auto;
                margin-top: 10px;
                white-space: pre-wrap;
            }

            /* OTA関連の既存スタイル... */
            .progress-bar {
                width: 100%;
                height: 20px;
                background: #eee;
                border-radius: 10px;
                overflow: hidden;
                display: none;
                margin-top: 10px;
            }

            .progress-fill {
                height: 100%;
                background: #667eea;
                width: 0%;
                transition: width 0.3s;
            }
        </style>
    </head>

    <body>
        <div class='container'>
            <h1>🏠 {{SYS_NAME}}</h1>
            <div style='text-align:center; font-size:12px; color:#666; margin-bottom:15px;'>ver:{{SYS_VER}}</div>

            <div class='info-box'>
                <p style='margin:0'>📶 RSSI: <span id='rssi'>-</span> dBm | 🕒 稼働: <span id='uptime'>-</span></p>
            </div>

            <div class='config-grid'>
                <div class='config-card'>
                    <div>判別時間(ミリ秒)</div>
                    <div style='font-size:20px; font-weight:bold;' id='val_judge'>{{JUDGEONTIME}}</div>
                    <div class='btn-group'>
                        <button class='btn-sm' onclick='location.href="/judgeOnTimeDown"'>-</button>
                        <button class='btn-sm' onclick='location.href="/judgeOnTimeUp"'>+</button>
                    </div>
                </div>
                <div class='config-card'>
                    <div>開始時間(分)</div>
                    <div style='font-size:20px; font-weight:bold;' id='val_start'>{{STARTTIME}}</div>
                    <div class='btn-group'>
                        <button class='btn-sm' onclick='location.href="/startTimeDown"'>-</button>
                        <button class='btn-sm' onclick='location.href="/startTimeUp"'>+</button>
                    </div>
                </div>
                <div class='config-card' style='grid-column: span 2;'>
                    <div>継続時間(時間)</div>
                    <div style='font-size:20px; font-weight:bold;' id='val_continue'>{{CONTINUETIME}}</div>
                    <div class='btn-group'>
                        <button class='btn-sm' onclick='location.href="/continueTimeDown"'>-</button>
                        <button class='btn-sm' onclick='location.href="/continueTimeUp"'>+</button>
                    </div>
                </div>
            </div>

            <details>
                <summary style='cursor:pointer; font-weight:bold; margin-bottom:5px;'>📜 ログ履歴</summary>
                <div class='history-box' id='history'>{{HISTORYDATA}}</div>
            </details>

            <hr style='margin:20px 0; border:0; border-top:1px solid #eee;'>

            <form id='uploadForm' enctype='multipart/form-data'>
                <input type='file' name='update' id='fileInput' accept='.bin' style='font-size:12px'>
                <button type='submit' id='uploadBtn'
                    style='margin-top: 15px; padding:10px; background:#444; color:white; border-radius:5px; width:100%; cursor:pointer;'>🔄
                    ファームウェア更新実行</button>
            </form>
            <div class='progress-bar' id='progBox'>
                <div class='progress-fill' id='progFill'></div>
            </div>
        </div>

        <script>
            // 定期的に情報を更新
            function updateInfo() {
                fetch('/info').then(r => r.json()).then(data => {
                    document.getElementById('rssi').textContent = data.rssi;
                    document.getElementById('uptime').textContent = Math.floor(data.rebootLog.uptime / 60) + '分';
                });
            }
            setInterval(updateInfo, 10000);
            updateInfo();

            // OTA処理
            document.getElementById('uploadForm').onsubmit = function (e) {
                e.preventDefault();
                const file = document.getElementById('fileInput').files[0];
                if (!file) return alert('ファイルを選択してください');

                const formData = new FormData();
                formData.append('update', file);
                document.getElementById('uploadBtn').disabled = true;
                document.getElementById('progBox').style.display = 'block';

                const xhr = new XMLHttpRequest();
                xhr.upload.onprogress = (e) => {
                    const p = Math.round((e.loaded / e.total) * 100);
                    document.getElementById('progFill').style.width = p + '%';
                };
                xhr.onload = () => { alert(xhr.status === 200 ? '成功！再起動します' : '失敗'); location.reload(); };
                xhr.open('POST', '/update');
                xhr.send(formData);
            };
        </script>
    </body>

    </html>
    )rawliteral";
    #endif