#pragma once

#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>

#include <functional>
#include <string>

#include "view/board_view.h"

class EpaperBoardView : public BoardView {
 public:
  using Display = GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT>;

  explicit EpaperBoardView(Display& display);

  void begin();
  void render(const DepartureBoard& board, time_t now) override;
  void renderNoConnection(time_t now) override;

 private:
  void paint(const std::function<void()>& drawContent);
  void drawHeader(const std::string& stopName, time_t now);
  void drawRow(int row, const Departure& departure, time_t now);
  void drawCentered(int baseline, const std::string& text);
  void drawRight(int right, int baseline, const std::string& text);
  std::string fitToWidth(const std::string& text, int maxWidth);
  int textWidth(const std::string& text);

  Display& display_;
  U8G2_FOR_ADAFRUIT_GFX u8g2_;
};
