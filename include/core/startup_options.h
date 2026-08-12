#pragma once

namespace bijoy::core {

struct StartupOptions {
  int defaultLayout = 0;
  int mainWindowLeft = 250;
  int mainWindowTop = 0;
  bool layoutActivationMode = false;
  bool trayMode = false;
  int applicationMode = 1;
};

StartupOptions LoadStartupOptions();
bool SaveStartupOptions(const StartupOptions& options);

} // namespace bijoy::core
