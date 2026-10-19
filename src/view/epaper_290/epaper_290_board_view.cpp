#include "view/epaper_290/epaper_290_board_view.h"

#include <ctime>
#include <string>

#include "util/truncate.h"
#include "view/countdown.h"

namespace {
constexpr int kWidth = 296;
constexpr int kMargin = 2;
constexpr int kGap = 8;
constexpr int kRows = 5;
constexpr int kHeaderBaseline = 16;
constexpr int kRuleY = 22;
constexpr int kFirstRowBaseline = 40;
constexpr int kRowHeight = 20;
constexpr int kHeadsignX = 48;
constexpr int kHeadsignWidth = 150;
constexpr int kDelayRight = 228;
constexpr int kMessageBaseline = kFirstRowBaseline + 2 * kRowHeight;

std::string clockText(time_t t) {
  tm local;
  localtime_r(&t, &local);
  char text[6];
  strftime(text, sizeof text, "%H:%M", &local);
  return text;
}
}  // namespace

Epaper290BoardView::Epaper290BoardView(Display& display) : display_(display) {}

void Epaper290BoardView::begin() {
  display_.init(115200);
  display_.setRotation(1);
  u8g2_.begin(display_);
  u8g2_.setFont(u8g2_font_calibration_gothic_nbp_t_all);
  u8g2_.setFontMode(1);
  u8g2_.setForegroundColor(GxEPD_BLACK);
}

void Epaper290BoardView::render(const DepartureBoard& board, time_t now,
                                std::optional<time_t> staleSince, Refresh refresh) {
  paint(refresh, [&] {
    drawHeader(board.stopName, now, staleSince);
    if (board.departures.empty()) {
      drawCentered(kMessageBaseline, "No departures");
      return;
    }
    int row = 0;
    for (const Departure& departure : board.departures) {
      if (row == kRows) break;
      drawRow(row++, departure, now);
    }
  });
}

void Epaper290BoardView::renderNoConnection(time_t now, Refresh refresh) {
  paint(refresh, [&] {
    drawHeader("", now, std::nullopt);
    drawCentered(kMessageBaseline, "No connection");
  });
}

void Epaper290BoardView::paint(Refresh refresh, const std::function<void()>& drawContent) {
  if (refresh == Refresh::Full) {
    display_.setFullWindow();
  } else {
    display_.setPartialWindow(0, 0, display_.width(), display_.height());
  }
  display_.firstPage();
  do {
    display_.fillScreen(GxEPD_WHITE);
    drawContent();
  } while (display_.nextPage());
  // Not hibernate(): the next partial refresh diffs against the controller RAM kept here.
  display_.powerOff();
}

void Epaper290BoardView::drawHeader(const std::string& stopName, time_t now,
                                    std::optional<time_t> staleSince) {
  std::string clock = clockText(now);
  if (staleSince) clock = "!" + clockText(*staleSince) + "  " + clock;

  const int clockWidth = textWidth(clock);
  drawRight(kWidth - kMargin, kHeaderBaseline, clock);
  u8g2_.drawUTF8(kMargin, kHeaderBaseline,
                 fitToWidth(stopName, kWidth - 2 * kMargin - clockWidth - kGap).c_str());
  display_.drawFastHLine(0, kRuleY, kWidth, GxEPD_BLACK);
}

void Epaper290BoardView::drawRow(int row, const Departure& departure, time_t now) {
  const int baseline = kFirstRowBaseline + row * kRowHeight;
  u8g2_.drawUTF8(kMargin, baseline, departure.routeShortName.c_str());
  u8g2_.drawUTF8(kHeadsignX, baseline, fitToWidth(departure.tripHeadsign, kHeadsignWidth).c_str());
  drawRight(kWidth - kMargin, baseline, countdownLabel(departure, now));
  if (departure.departure && departure.departure->delay > 0) {
    const int32_t minutes = (departure.departure->delay + 59) / 60;
    drawRight(kDelayRight, baseline, "+" + std::to_string(minutes));
  }
}

void Epaper290BoardView::drawCentered(int baseline, const std::string& text) {
  u8g2_.drawUTF8((kWidth - textWidth(text)) / 2, baseline, text.c_str());
}

void Epaper290BoardView::drawRight(int right, int baseline, const std::string& text) {
  u8g2_.drawUTF8(right - textWidth(text), baseline, text.c_str());
}

std::string Epaper290BoardView::fitToWidth(const std::string& text, int maxWidth) {
  return truncateToWidth(text, maxWidth, [this](const std::string& s) { return textWidth(s); });
}

int Epaper290BoardView::textWidth(const std::string& text) {
  return u8g2_.getUTF8Width(text.c_str());
}
