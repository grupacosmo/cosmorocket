#pragma once
class AsyncWebServer;

namespace LoadCell {
    bool initHX711();
    void initLoadCellEndpoints(AsyncWebServer &server);
}