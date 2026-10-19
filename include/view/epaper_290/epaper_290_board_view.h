#pragma once

#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>

#include <functional>
#include <optional>
#include <string>

#include "view/board_view.h"

// Waveshare 2.9" V2 e-paper (GxEPD2_290_T94_V2), landscape 296x128.
class Epaper290BoardView : public BoardView {
 public:
  using Display = GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT>;

  explicit Epaper290BoardView(Display& display);

  void begin();
  void render(const DepartureBoard& board, time_t now, std::optional<time_t> staleSince,
              Refresh refresh) override;
  void renderNoConnection(time_t now, Refresh refresh) override;

 private:
  void paint(Refresh refresh, const std::function<void()>& drawContent);
  // One full-screen page (the buffer holds the whole screen), drawn in the given colours.
  void drawPage(uint16_t ink, uint16_t paper, const std::function<void()>& drawContent);
  void drawHeader(const std::string& stopName, time_t now, std::optional<time_t> staleSince);
  void drawRow(int row, const Departure& departure, time_t now);
  void drawCentered(int baseline, const std::string& text);
  void drawRight(int right, int baseline, const std::string& text);
  std::string fitToWidth(const std::string& text, int maxWidth);
  int textWidth(const std::string& text);

  Display& display_;
  U8G2_FOR_ADAFRUIT_GFX u8g2_;
  uint16_t ink_ = GxEPD_BLACK;
};
