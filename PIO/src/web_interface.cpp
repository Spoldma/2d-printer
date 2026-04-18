#include "web_interface.h"

#include <WiFi.h>
#include <WebServer.h>

#include "config.h"

namespace {
WebServer g_webServer(Config::WEB_PORT);
WebInterface::CommandExecutor g_executor = nullptr;

const char WEB_UI[] PROGMEM = R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Plotter</title>
  <style>
    *{box-sizing:border-box;margin:0;padding:0}
    body{background:#0d0d0f;min-height:100vh;padding:2rem 1.5rem;font-family:'Courier New',monospace;color:#d0d0d8}
    .header{display:flex;align-items:center;gap:10px;margin-bottom:.4rem}
    .dot{width:8px;height:8px;border-radius:50%;background:#22c55e}
    h1{font-size:15px;font-weight:500;letter-spacing:.05em;text-transform:uppercase}
    .sub{font-size:12px;color:#777;margin-bottom:2rem;font-family:Arial,sans-serif}
    .label{font-size:11px;color:#666;letter-spacing:.1em;text-transform:uppercase;margin-bottom:8px;font-family:Arial,sans-serif}
    .card{background:#111114;border:1px solid #333;border-radius:10px;padding:1rem;margin-bottom:1rem}
    .row{display:flex;gap:8px}
    input[type=text],textarea{width:100%;background:#0a0a0c;border:1px solid #3a3a44;border-radius:6px;color:#d0d0d8;font-family:'Courier New',monospace;font-size:13px;padding:10px 12px;outline:none;resize:vertical}
    input[type=text]{height:40px;resize:none}
    input[type=text]:focus,textarea:focus{border-color:#6666dd}
    textarea{height:140px;line-height:1.6}
    .btn{background:#1e1e38;border:1px solid #5555cc;border-radius:6px;color:#aaaaff;font-size:12px;font-family:Arial,sans-serif;padding:0 16px;cursor:pointer;white-space:nowrap;height:40px}
    .btn:hover{background:#28285a;color:#ccccff}
    .btn-sm{font-size:11px;padding:0 12px;height:32px}
    .flex-end{display:flex;justify-content:flex-end;margin-top:8px}
    .status-bar{display:flex;align-items:center;justify-content:space-between;margin-bottom:6px}
    .status-txt{font-size:11px;color:#666;font-family:Arial,sans-serif}
    .clear-btn{font-size:11px;color:#666;background:none;border:none;cursor:pointer;font-family:Arial,sans-serif}
    .clear-btn:hover{color:#aaa}
    .console{background:#080809;border:1px solid #2a2a2a;border-radius:8px;padding:1rem;min-height:160px;font-size:12px;line-height:1.8;color:#22c55e;overflow-y:auto;max-height:240px;white-space:pre-wrap;word-break:break-all}
    .hint{font-size:11px;color:#555;margin-top:8px;font-family:Arial,sans-serif}
    .chip{background:#111120;border:1px solid #3a3a66;border-radius:4px;padding:1px 6px;font-size:11px;color:#8888cc;margin:2px}
  </style>
</head>
<body>
  <div class="header"><div class="dot"></div><h1>ESP32 Plotter</h1></div>
  <p class="sub">Serial command interface — send instructions to the plotter over Wi-Fi</p>
  <div class="card">
    <div class="label">Single command</div>
    <div class="row">
      <input type="text" id="single" placeholder="HOME">
      <button class="btn" onclick="sendSingle()">Send</button>
    </div>
    <p class="hint">Try: <span class="chip">HOME</span> <span class="chip">MOV:020,050</span> <span class="chip">LINE:020,050,180,100</span></p>
  </div>
  <div class="card">
    <div class="label">Batch commands</div>
    <textarea id="batch" placeholder="HOME&#10;MOV:020,050&#10;LINE:020,050,180,100"></textarea>
    <div class="flex-end"><button class="btn btn-sm" onclick="sendBatch()">Run Batch</button></div>
  </div>
  <div class="card">
    <div class="status-bar">
      <span class="status-txt" id="stxt">No output yet</span>
      <button class="clear-btn" onclick="clearOut()">Clear</button>
    </div>
    <div class="console" id="out" style="color:#444">Awaiting commands...</div>
  </div>
  <script>
    const out=document.getElementById('out'),stxt=document.getElementById('stxt');
    function clearOut(){out.textContent='Awaiting commands...';out.style.color='#444';stxt.textContent='No output yet';}
    function ts(){return new Date().toTimeString().slice(0,8);}
    async function post(cmd){const r=await fetch('/api/print',{method:'POST',headers:{'Content-Type':'text/plain'},body:cmd});return await r.text();}
    async function sendSingle(){
      const cmd=document.getElementById('single').value.trim();if(!cmd)return;
      stxt.textContent='Sending...';
      try{const res=await post(cmd);out.style.color='#22c55e';out.textContent='['+ts()+'] '+cmd+'\n=> '+res;stxt.textContent='Last: '+cmd;}
      catch(e){out.textContent='[ERR] '+e.message;stxt.textContent='Error';}
    }
    async function sendBatch(){
      const lines=document.getElementById('batch').value.split(/\r?\n/).map(x=>x.trim()).filter(Boolean);
      if(!lines.length)return;stxt.textContent='Running '+lines.length+' command(s)...';
      let r='';
      for(const cmd of lines){try{r+='['+ts()+'] '+cmd+'\n=> '+(await post(cmd))+'\n\n';}catch(e){r+='[ERR] '+cmd+'\n=> '+e.message+'\n\n';}}
      out.style.color='#22c55e';out.textContent=r.trim();stxt.textContent='Done — '+lines.length+' command(s)';
    }
  </script>
</body>
</html>
)HTML";

const char *statusToText(StatusCode code) {
  switch (code) {
    case StatusCode::OK:
      return "OK";
    case StatusCode::ERR_PARSE:
      return "ERR_PARSE";
    case StatusCode::ERR_RANGE:
      return "ERR_RANGE";
    case StatusCode::ERR_UNSUPPORTED:
      return "ERR_UNSUPPORTED";
    default:
      return "ERR_UNKNOWN";
  }
}

void handleRoot() { g_webServer.send(200, "text/html", WEB_UI); }

void handlePrint() {
  String command = g_webServer.arg("plain");
  command.trim();

  if (command.length() == 0) {
    command = g_webServer.arg("cmd");
    command.trim();
  }

  if (command.length() == 0 || g_executor == nullptr) {
    g_webServer.send(400, "text/plain", "ERR_PARSE");
    return;
  }

  StatusCode status = g_executor(command, "[WIFI]");
  g_webServer.send(status == StatusCode::OK ? 200 : 400, "text/plain",
                   statusToText(status));
}
}  // namespace

namespace WebInterface {
void begin(CommandExecutor executor) {
  g_executor = executor;

  Serial.println("[WIFI] Starting Access Point");
  WiFi.mode(WIFI_STA);
  WiFi.begin(Config::WIFI_AP_SSID, Config::WIFI_AP_PASSWORD);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  g_webServer.on("/", HTTP_GET, handleRoot);
  g_webServer.on("/api/print", HTTP_POST, handlePrint);
  g_webServer.on("/api/print", HTTP_GET, handlePrint);
  g_webServer.begin();
  Serial.println("[WIFI] Web server started");
}

void handleClient() { g_webServer.handleClient(); }
}  // namespace WebInterface
