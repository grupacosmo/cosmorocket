#include "Arduino.h"
#include "ESPAsyncWebServer.h"
#include "INA226.h"
#include "SPIFFS.h"
#include "WiFi.h"
#include "Wire.h"
#include "config.h"
#include "loadcell.h"
#include "pressure.h"
#include <Arduino_JSON.h>

JSONVar readings;

INA226 ina(0x40);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

Pressure last_pressure = Pressure::empty();

auto init_wifi() -> void;
auto init_spiffs() -> void;
auto init_ina() -> void;
auto init_server() -> void;
auto init_web_socket() -> void;
auto init_pressure() -> void;
auto print_info() -> void;

auto get_sensor_readings() -> String;
auto notify_clients(String sensorReadings) -> void;
auto handle_web_socket_message(void *arg, uint8_t *data, size_t len) -> void;
auto on_event(AsyncWebSocket *server, AsyncWebSocketClient *client,
              AwsEventType type, void *arg, uint8_t *data, size_t len) -> void;

auto setup() -> void {
    Serial.begin(Config::baud_rate);
    Wire.begin(Config::sda, Config::scl);

    init_ina();
    init_wifi();
    init_spiffs();
    init_server();
    init_pressure();
    LoadCell::init_hx711();
    print_info();
}

auto loop() -> void {
    ina.waitConversionReady();

    float current_ma = ina.getCurrent_mA();
    last_pressure = Pressure::relative(current_ma);
    Serial.printf("Current: %.4fmA | Pascals: %.4f\r", current_ma,
                  last_pressure.Pa());

    notify_clients(get_sensor_readings());

    ws.cleanupClients();
}

auto init_ina() -> void {
    if (!ina.begin()) {
        Serial.println("Could not connect. Fix and Reboot");
    }

    ina.setMaxCurrentShunt(Config::max_current_amperes, Config::shunt_resistor,
                           false);
    ina.setAverage(Config::avg);
}

auto init_wifi() -> void {
    WiFi.mode(WIFI_STA);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname("pressure-sensor");
    WiFi.begin(Config::ssid, Config::password);
    Serial.printf("Connecting to WiFi (%s)..", Config::ssid);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println();
}

auto init_spiffs() -> void {
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
    }
}

auto init_server() -> void {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });

    LoadCell::init_load_cell_endpoints(server);
    server.serveStatic("/", SPIFFS, "/");

    server.begin();

    ws.onEvent(on_event);
    server.addHandler(&ws);
}

auto init_pressure() -> void {
    ina.waitConversionReady();
    float min_current = ina.getCurrent_mA();

    for (std::size_t i = 1; i < 10; ++i) {
        ina.waitConversionReady();
        float current = ina.getCurrent_mA();

        if (current < min_current) {
            min_current = current;
        }

        Serial.printf("Calculating minimum current %zu/10\r", i + 1);
    }

    Pressure::set_min_current(min_current);
}

auto print_info() -> void {
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Hostname: ");
    Serial.println(WiFi.getHostname());
    Serial.printf("Min current: %f\n", Pressure::get_min_current());
}

auto get_sensor_readings() -> String {
    readings["Pa"] = String(last_pressure.Pa());
    readings["psi"] = String(last_pressure.psi());
    readings["bar"] = String(last_pressure.bar());
    readings["atm"] = String(last_pressure.atm());

    String json_string = JSON.stringify(readings);
    return json_string;
}

auto notify_clients(String sensorReadings) -> void {
    ws.textAll(sensorReadings);
}

auto handle_web_socket_message(void *arg, uint8_t *data, size_t len) -> void {
    auto *info = static_cast<AwsFrameInfo *>(arg);
    if (info->final && info->index == 0 && info->len == len &&
        info->opcode == WS_TEXT) {
        String sensor_readings = get_sensor_readings();
        Serial.print(sensor_readings);
        notify_clients(sensor_readings);
    }
}

auto on_event(AsyncWebSocket *server, AsyncWebSocketClient *client,
              AwsEventType type, void *arg, uint8_t *data, size_t len) -> void {
    switch (type) {
    case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(),
                      client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        handle_web_socket_message(arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}
