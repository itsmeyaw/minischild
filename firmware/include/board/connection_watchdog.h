#pragma once

enum class WatchdogAction { None, Reconnect, Restart };

constexpr unsigned long kReconnectEvery = 30UL * 1000;     // retry WiFi this often while it is down
constexpr unsigned long kRestartAfter = 10UL * 60 * 1000;  // WiFi down this long → restart

class ConnectionWatchdog {
 public:
  // Decides how to recover WiFi; call from loop() with millis().
  WatchdogAction check(bool connected, unsigned long nowMs) {
    if (connected) {
      down_ = false;
      return WatchdogAction::None;
    }
    if (!down_) {
      down_ = true;
      outageStart_ = nowMs;
      lastAttempt_ = nowMs;
      return WatchdogAction::None;
    }
    if (nowMs - outageStart_ >= kRestartAfter) return WatchdogAction::Restart;
    if (nowMs - lastAttempt_ >= kReconnectEvery) {
      lastAttempt_ = nowMs;
      return WatchdogAction::Reconnect;
    }
    return WatchdogAction::None;
  }

 private:
  bool down_ = false;
  unsigned long outageStart_ = 0;
  unsigned long lastAttempt_ = 0;
};
