#include "WiFi.h"
#include "WebServer.h"


const char* ssid = "Omar";
const char* password = "10007000";

WebServer server(80);  // Create a web server object that listens on port 80

void handleRoot() {
  String html = "<html><head>";
  html += "<link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\">";
  html += "</head><body>";
  html += "<div class='bg'></div>";
  html += "<div class='navbar'><h1>The Calf Feeding Machine</h1></div>";
  html += "<div class='content'>";
  html += "<div class='button-container'>";
  html += "<p><a href=\"/command?cmd=start\"><button>Start</button></a></p>";
  html += "</div>";
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}


void handleAdjust() {
  String html = "<html><head>";
  html += "<link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\">";
  html += "<style>";
  html += ".content { font-size: 1.5em; color: black; text-align: center; }";  // Center text and change color
  html += "label { font-size: 1.2em; color: black; }";
  html += "select { font-size: 1.5em; padding: 10px; margin: 10px; width: 80%; text-align: center; }";  // Adjust select styles and center text
  html += "input[type=submit] { font-size: 1.5em; padding: 15px 30px; margin: 10px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; }";  // Increase size of submit button
  html += "input[type=submit]:hover { background-color: #45a049; }";  // Add hover effect to submit button
  html += "</style></head><body>";
  html += "<div class='bg'></div>";
  html += "<div class='content'>";
  html += "<h1>Adjust Settings</h1>";
  html += "<form action=\"/submit_adjustment\">";
  html += "<label for=\"adjustment\">Choose an adjustment:</label><br>";
  html += "<select name=\"adjustment\" id=\"adjustment\">";
  html += "<option value=\"temp\">Temp</option>";
  html += "<option value=\"water\">Water</option>";
  html += "<option value=\"milk\">Milk</option>";
  html += "</select><br><br>";
  html += "<input type=\"submit\" value=\"Submit\">";
  html += "</form>";
  html += "<p><a href=\"/command?cmd=start\"><button>Go Back</button></a></p>"; // Change the link to go back to the start page
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}



void handleAdjustmentValue() {
  String adjustment = server.arg("adjustment");

  String html = "<html><head>";
  html += "<link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\">";
  html += "<style>";
  html += ".content { font-size: 1.5em; color: black; }";  // Increase font size and change text color
  html += "label { font-size: 1.2em; }";
  html += "input[type=text], select { font-size: 1.5em; padding: 10px; margin: 10px; width: 80%; }";  // Adjust input and select styles
  html += "</style></head><body>";
  html += "<div class='bg'></div>";
  html += "<div class='content'>";
  html += "<h1>Set New Value</h1>";
  html += "<form action=\"/submit_value\">";
  html += "<label for=\"value\">Set new value for " + adjustment + ":</label><br>";
  html += "<input type=\"hidden\" name=\"adjustment\" value=\"" + adjustment + "\">";
  html += "<input type=\"text\" id=\"value\" name=\"value\"><br><br>";
  html += "<input type=\"submit\" value=\"Submit\">";
  html += "</form>";
  html += "<p><a href=\"/command?cmd=start\"><button>Go Back</button></a></p>"; // Change the link to go back to the start page
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

void handleSubmitValue() {
  String adjustment = server.arg("adjustment");
  String value = server.arg("value");

  String message = "<html><head>";
  message += "<link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\">";
  message += "</head><body>";
  message += "<div class='bg'></div>";
  message += "<div class='content'>";
  message += "<h1>Adjustment Submitted</h1>";
  message += "<p>Selected adjustment: " + adjustment + "</p>";
  message += "<p>New value: " + value + "</p>";
  message += "<p><a href=\"/command?cmd=start\"><button>Go Back</button></a></p>"; // Change the link to go back to the start page
  message += "</div></body></html>";
  server.send(200, "text/html", message);

  // Handle the new value for the selected adjustment here
  Serial.println("Adjustment: " + adjustment + ", New Value: " + value);
}

void handleCommand() {
  String cmd = server.arg("cmd");

  if (cmd == "auto") {
    // Handle the Auto command
    server.send(200, "text/html", "<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\"></head><body><div class='bg'></div><div class='content'><h1>Auto Mode Activated</h1><p><a href=\"/\"><button>Go Back</button></a></p></div></body></html>");
  } else if (cmd == "adjust") {
    // Handle the Adjust command
    handleAdjust();
  } else if (cmd == "start") {
    // Handle the Start command
    String html = "<html><head>";
    html += "<link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\">";
    html += "</head><body>";
    html += "<div class='bg'></div>";
    html += "<div class='content'>";
    html += "<h1>Start Command Activated</h1>";
    // Add the text area here
    html += "<textarea id='sensorValues' readonly>Sensor values will be displayed here...</textarea>";
    html += "<div class='button-container'>";
    // Add the Adjust button to the bottom left side
    html += "<p style='float: left;'><a href=\"/command?cmd=adjust\"><button>Adjust</button></a></p>";
    // Add the Stop button to the bottom right side with red color
    html += "<p style='float: right;'><a href=\"/command?cmd=stop\"><button style='background-color: red;'>Stop</button></a></p>";
    html += "</div>";
    html += "<p><a href=\"/\"><button>Go Back</button></a></p>";
    html += "</div></body></html>";
    server.send(200, "text/html", html);
  } else if (cmd == "stop") {
    // Handle the Stop command
    server.send(200, "text/html", "<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\"></head><body><div class='bg'></div><div class='content'><h1>Stop Command Activated</h1><p><a href=\"/\"><button>Go Back</button></a></p></div></body></html>");
  } else {
    server.send(200, "text/html", "<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\"></head><body><div class='bg'></div><div class='content'><h1>Invalid Command</h1><p><a href=\"/\"><button>Go Back</button></a></p></div></body></html>");
  }
}




void handleCSS() {
  String css = "body { font-family: Arial, sans-serif; text-align: center; color: white; background-color: #333; margin: 0; padding: 0; }";
  css += ".bg { position: fixed; top: 0; left: 0; width: 100%; height: 100%; background-image: url('https://www.renews.co.nz/assets/9-v3__ResizedImageWzEwMDAsNTYzXQ.-Calves-feeding.jpg'); background-size: cover; filter: blur(3px); z-index: -1; }";
  css += ".navbar { background-color: brown; padding: 20px; text-align: center; }";
  css += ".navbar h1 { margin: 0; font-size: 2em; color: white; }";
  css += ".content { position: relative; z-index: 1; margin-top: 100px; }";
  css += ".button-container { display: flex; justify-content: center; flex-wrap: wrap; }";
  css += "button { font-size: 1.5em; padding: 10px 20px; margin: 10px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; width: 150px; }";
  css += "button:hover { background-color: #45a049; }";
  css += "textarea { font-size: 1.5em; padding: 10px; margin: 20px; width: 80%; height: 100px; resize: none; }";
  server.send(200, "text/css", css);
}

void setup() {
  Serial.begin(115200);

  Serial.println("Initializing WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to ");
  Serial.print(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected to WiFi network.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Define routes
  server.on("/", handleRoot);
  server.on("/command", handleCommand);
  server.on("/submit_adjustment", handleAdjustmentValue);
  server.on("/submit_value", handleSubmitValue);
  server.on("/style.css", handleCSS);

  server.begin();  // Start the server
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();  // Handle client requests
}
