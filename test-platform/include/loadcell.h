#pragma once
class AsyncWebServer;

namespace LoadCell {
auto init_hx711() -> bool;
auto init_load_cell_endpoints(AsyncWebServer &server) -> void;
auto loadCellTask(void *params) -> void;
} // namespace LoadCell
