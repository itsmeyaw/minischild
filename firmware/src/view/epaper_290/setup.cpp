#include "setup.h"

#include <SPI.h>

#include "config.h"
#include "view/epaper_290/epaper_290_board_view.h"

BoardView& setupBoardView() {
  static Epaper290BoardView::Display display(GxEPD2_290_T94_V2(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY));
  static Epaper290BoardView view(display);
  SPI.begin(PIN_CLK, -1, PIN_DIN, PIN_CS);
  view.begin();
  return view;
}
