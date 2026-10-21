#pragma once

#include <ctime>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "board/board_selection.h"
#include "board/clock.h"
#include "provider/departure_provider.h"
#include "view/board_view.h"

constexpr time_t kStaleLimit = 10 * 60;  // last good board older than this → No connection
constexpr int kFullRefreshEvery = 30;    // every 30th render is Refresh::Full

class BoardController {
 public:
  BoardController(DepartureProvider& provider, BoardView& view, Clock& clock,
                  std::vector<std::string> platforms)
      : provider_(provider), view_(view), clock_(clock), platforms_(std::move(platforms)) {}

  // Fetches and renders on the first tick of each minute (hh:mm:00); call from loop().
  // Returns the fetch error, if this tick fetched and the fetch failed, so the caller can log it.
  std::optional<FetchError> tick() {
    const time_t now = clock_.now();
    if (lastFetchAt_ && now / 60 == *lastFetchAt_ / 60) {
      return std::nullopt;
    }
    lastFetchAt_ = now;
    FetchResult result = provider_.fetch();
    const FetchError* error = std::get_if<FetchError>(&result);
    if (!error) {
      lastGood_ = std::get<DepartureBoard>(std::move(result));
      lastGoodAt_ = now;
    }
    Refresh refresh = renders_ % kFullRefreshEvery == 0 ? Refresh::Full : Refresh::Partial;
    renders_++;
    if (!lastGood_ || now < lastGoodAt_ || now - lastGoodAt_ > kStaleLimit) {
      lastGood_.reset();
      view_.renderNoConnection(now, refresh);
    } else {
      DepartureBoard board{lastGood_->stopName, lastGood_->timestamp,
                           selectDepartures(lastGood_->departures, platforms_, now, view_.rows())};
      view_.render(board, now, error ? std::optional<time_t>(lastGoodAt_) : std::nullopt, refresh);
    }
    return error ? std::optional<FetchError>(*error) : std::nullopt;
  }

 private:
  DepartureProvider& provider_;
  BoardView& view_;
  Clock& clock_;
  std::vector<std::string> platforms_;
  std::optional<time_t> lastFetchAt_;
  std::optional<DepartureBoard> lastGood_;
  time_t lastGoodAt_ = 0;
  int renders_ = 0;
};
