#include <unity.h>

#include <optional>
#include <string>
#include <vector>

#include "board/board_controller.h"

// 2026-10-19 12:00:00 UTC
static const time_t NOON = 1792411200;

class FakeProvider : public DepartureProvider {
 public:
  FetchResult result = DepartureBoard{};
  int fetches = 0;

  FetchResult fetch() override {
    fetches++;
    return result;
  }
};

class FakeView : public BoardView {
 public:
  size_t rowCount = 5;
  int renders = 0;
  int noConnections = 0;
  std::vector<Refresh> refreshes;
  // Details of the most recent render call.
  bool lastWasNoConnection = false;
  std::vector<std::string> lastRoutes;
  std::optional<time_t> lastStaleSince;
  Refresh lastRefresh = Refresh::Partial;
  time_t lastNow = 0;

  size_t rows() const override { return rowCount; }

  void render(const DepartureBoard& board, time_t now, std::optional<time_t> staleSince,
              Refresh refresh) override {
    renders++;
    refreshes.push_back(refresh);
    lastWasNoConnection = false;
    lastRoutes.clear();
    for (const Departure& d : board.departures) lastRoutes.push_back(d.routeShortName);
    lastStaleSince = staleSince;
    lastRefresh = refresh;
    lastNow = now;
  }

  void renderNoConnection(time_t now, Refresh refresh) override {
    noConnections++;
    refreshes.push_back(refresh);
    lastWasNoConnection = true;
    lastRoutes.clear();
    lastStaleSince = std::nullopt;
    lastRefresh = refresh;
    lastNow = now;
  }
};

class FakeClock : public Clock {
 public:
  time_t current = NOON;

  time_t now() override { return current; }
};

static Departure scheduledAt(const char* route, time_t t) {
  return Departure{route, "Freising", "1", t, ScheduleRelationship::Scheduled, std::nullopt};
}

static DepartureBoard boardWith(std::vector<Departure> departures) {
  return DepartureBoard{"Hauptbahnhof", NOON, std::move(departures)};
}

static FakeProvider provider;
static FakeView view;
static FakeClock clock_;

void setUp() {
  provider = FakeProvider{};
  view = FakeView{};
  clock_ = FakeClock{};
}

void tearDown() {}

void test_first_tick_fetches_and_renders_fresh_board_fully() {
  provider.result = boardWith({scheduledAt("S1", NOON + 5 * 60)});
  BoardController controller(provider, view, clock_, {});

  controller.tick();

  TEST_ASSERT_EQUAL(1, provider.fetches);
  TEST_ASSERT_EQUAL(1, view.renders);
  TEST_ASSERT_FALSE(view.lastStaleSince.has_value());
  TEST_ASSERT_EQUAL(Refresh::Full, view.lastRefresh);
}

void test_tick_within_the_same_minute_neither_fetches_nor_renders() {
  provider.result = boardWith({scheduledAt("S1", NOON + 5 * 60)});
  BoardController controller(provider, view, clock_, {});
  controller.tick();

  clock_.current = NOON + 30;
  controller.tick();

  TEST_ASSERT_EQUAL(1, provider.fetches);
  TEST_ASSERT_EQUAL(1, view.renders);
}

void test_tick_in_the_next_minute_fetches_and_renders_partially() {
  provider.result = boardWith({scheduledAt("S1", NOON + 5 * 60)});
  BoardController controller(provider, view, clock_, {});
  controller.tick();

  clock_.current = NOON + 60;
  controller.tick();

  TEST_ASSERT_EQUAL(2, provider.fetches);
  TEST_ASSERT_EQUAL(2, view.renders);
  TEST_ASSERT_EQUAL(Refresh::Partial, view.lastRefresh);
}

void test_refresh_lands_on_the_next_full_minute_not_sixty_seconds_later() {
  provider.result = boardWith({scheduledAt("S1", NOON + 5 * 60)});
  clock_.current = NOON + 45;
  BoardController controller(provider, view, clock_, {});
  controller.tick();

  clock_.current = NOON + 59;
  controller.tick();
  TEST_ASSERT_EQUAL(1, provider.fetches);

  clock_.current = NOON + 60;
  controller.tick();
  TEST_ASSERT_EQUAL(2, provider.fetches);
}

void test_full_refresh_on_first_and_every_thirtieth_render() {
  provider.result = boardWith({scheduledAt("S1", NOON + 5 * 60)});
  BoardController controller(provider, view, clock_, {});

  for (int i = 0; i < 31; i++) {
    controller.tick();
    clock_.current += 60;
  }

  TEST_ASSERT_EQUAL(31, view.renders);
  for (int i = 0; i < 31; i++) {
    Refresh expected = (i == 0 || i == 30) ? Refresh::Full : Refresh::Partial;
    TEST_ASSERT_EQUAL(expected, view.refreshes[i]);
  }
}

void test_failed_fetch_renders_last_good_board_as_stale() {
  provider.result = boardWith({scheduledAt("S1", NOON + 5 * 60)});
  BoardController controller(provider, view, clock_, {});
  controller.tick();

  clock_.current = NOON + 60;
  provider.result = FetchError{"boom"};
  std::optional<FetchError> error = controller.tick();

  TEST_ASSERT_TRUE(error.has_value());
  TEST_ASSERT_EQUAL_STRING("boom", error->message.c_str());
  TEST_ASSERT_EQUAL(2, view.renders);
  TEST_ASSERT_EQUAL(0, view.noConnections);
  TEST_ASSERT_EQUAL(1, view.lastRoutes.size());
  TEST_ASSERT_TRUE(view.lastStaleSince.has_value());
  TEST_ASSERT_EQUAL(NOON, *view.lastStaleSince);
}

void test_last_good_board_exactly_ten_minutes_old_is_still_stale() {
  provider.result = boardWith({scheduledAt("S1", NOON + 5 * 60)});
  BoardController controller(provider, view, clock_, {});
  controller.tick();

  clock_.current = NOON + 10 * 60;
  provider.result = FetchError{"boom"};
  controller.tick();

  TEST_ASSERT_EQUAL(2, view.renders);
  TEST_ASSERT_EQUAL(0, view.noConnections);
}

void test_last_good_board_older_than_ten_minutes_shows_no_connection() {
  provider.result = boardWith({scheduledAt("S1", NOON + 5 * 60)});
  BoardController controller(provider, view, clock_, {});
  controller.tick();

  clock_.current = NOON + 10 * 60 + 1;
  provider.result = FetchError{"boom"};
  controller.tick();

  TEST_ASSERT_EQUAL(1, view.noConnections);
  TEST_ASSERT_TRUE(view.lastWasNoConnection);
}

void test_failure_without_good_board_shows_no_connection() {
  provider.result = FetchError{"boom"};
  BoardController controller(provider, view, clock_, {});

  std::optional<FetchError> error = controller.tick();

  TEST_ASSERT_TRUE(error.has_value());
  TEST_ASSERT_EQUAL(1, view.noConnections);
  TEST_ASSERT_TRUE(view.lastWasNoConnection);
}

void test_recovery_after_failure_renders_fresh_board() {
  provider.result = boardWith({scheduledAt("S1", NOON + 5 * 60)});
  BoardController controller(provider, view, clock_, {});
  controller.tick();

  clock_.current = NOON + 60;
  provider.result = FetchError{"boom"};
  controller.tick();

  clock_.current = NOON + 120;
  provider.result = boardWith({scheduledAt("S1", NOON + 5 * 60)});
  std::optional<FetchError> error = controller.tick();

  TEST_ASSERT_FALSE(error.has_value());
  TEST_ASSERT_EQUAL(3, view.renders);
  TEST_ASSERT_FALSE(view.lastStaleSince.has_value());
}

void test_board_with_no_departures_after_selection_renders_empty() {
  provider.result = boardWith({scheduledAt("P", NOON - 2 * 60)});
  BoardController controller(provider, view, clock_, {});

  controller.tick();

  TEST_ASSERT_EQUAL(1, view.renders);
  TEST_ASSERT_EQUAL(0, view.lastRoutes.size());
}

void test_stale_render_reselects_departures_against_current_time() {
  provider.result = boardWith({scheduledAt("A", NOON + 2 * 60), scheduledAt("B", NOON + 8 * 60)});
  BoardController controller(provider, view, clock_, {});
  controller.tick();

  clock_.current = NOON + 5 * 60;
  provider.result = FetchError{"boom"};
  controller.tick();

  TEST_ASSERT_EQUAL(2, view.renders);
  TEST_ASSERT_EQUAL(1, view.lastRoutes.size());
  TEST_ASSERT_EQUAL_STRING("B", view.lastRoutes[0].c_str());
}

void test_clock_jumping_backwards_fetches_again() {
  provider.result = boardWith({scheduledAt("S1", NOON + 5 * 60)});
  BoardController controller(provider, view, clock_, {});
  controller.tick();

  clock_.current = NOON - 30;
  controller.tick();

  TEST_ASSERT_EQUAL(2, provider.fetches);
  TEST_ASSERT_EQUAL(2, view.renders);
}

void test_failure_after_clock_jumped_before_last_good_fetch_renders_no_connection() {
  provider.result = boardWith({scheduledAt("S1", NOON - 60 * 60)});
  BoardController controller(provider, view, clock_, {});
  controller.tick();

  provider.result = FetchError{"boom"};
  clock_.current = NOON - 60 * 60;
  controller.tick();

  TEST_ASSERT_TRUE(view.lastWasNoConnection);
}

void test_board_too_stale_to_show_is_never_shown_again() {
  provider.result = boardWith({scheduledAt("S1", NOON + 5 * 60)});
  BoardController controller(provider, view, clock_, {});
  controller.tick();

  clock_.current = NOON + 10 * 60 + 1;
  provider.result = FetchError{"boom"};
  controller.tick();

  clock_.current = NOON + 5 * 60;
  controller.tick();

  TEST_ASSERT_EQUAL(1, view.renders);
  TEST_ASSERT_EQUAL(2, view.noConnections);
  TEST_ASSERT_TRUE(view.lastWasNoConnection);
}

void test_view_receives_only_as_many_departures_as_it_has_rows() {
  view.rowCount = 2;
  provider.result = boardWith({scheduledAt("A", NOON + 5 * 60), scheduledAt("B", NOON + 2 * 60),
                               scheduledAt("C", NOON + 8 * 60)});
  BoardController controller(provider, view, clock_, {});

  controller.tick();

  TEST_ASSERT_EQUAL(2, view.lastRoutes.size());
  TEST_ASSERT_EQUAL_STRING("B", view.lastRoutes[0].c_str());
  TEST_ASSERT_EQUAL_STRING("A", view.lastRoutes[1].c_str());
}

void test_fresh_render_shows_only_departures_on_listed_platforms() {
  Departure other = scheduledAt("B", NOON + 2 * 60);
  other.platformCode = "2";
  provider.result = boardWith({scheduledAt("A", NOON + 5 * 60), other});
  BoardController controller(provider, view, clock_, {"1"});

  controller.tick();

  TEST_ASSERT_EQUAL(1, view.lastRoutes.size());
  TEST_ASSERT_EQUAL_STRING("A", view.lastRoutes[0].c_str());
}

void test_stale_render_shows_only_departures_on_listed_platforms() {
  Departure other = scheduledAt("B", NOON + 2 * 60);
  other.platformCode = "2";
  provider.result = boardWith({scheduledAt("A", NOON + 5 * 60), other});
  BoardController controller(provider, view, clock_, {"1"});
  controller.tick();

  clock_.current = NOON + 60;
  provider.result = FetchError{"boom"};
  controller.tick();

  TEST_ASSERT_EQUAL(2, view.renders);
  TEST_ASSERT_EQUAL(1, view.lastRoutes.size());
  TEST_ASSERT_EQUAL_STRING("A", view.lastRoutes[0].c_str());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_first_tick_fetches_and_renders_fresh_board_fully);
  RUN_TEST(test_tick_within_the_same_minute_neither_fetches_nor_renders);
  RUN_TEST(test_tick_in_the_next_minute_fetches_and_renders_partially);
  RUN_TEST(test_refresh_lands_on_the_next_full_minute_not_sixty_seconds_later);
  RUN_TEST(test_full_refresh_on_first_and_every_thirtieth_render);
  RUN_TEST(test_failed_fetch_renders_last_good_board_as_stale);
  RUN_TEST(test_last_good_board_exactly_ten_minutes_old_is_still_stale);
  RUN_TEST(test_last_good_board_older_than_ten_minutes_shows_no_connection);
  RUN_TEST(test_failure_without_good_board_shows_no_connection);
  RUN_TEST(test_recovery_after_failure_renders_fresh_board);
  RUN_TEST(test_board_with_no_departures_after_selection_renders_empty);
  RUN_TEST(test_stale_render_reselects_departures_against_current_time);
  RUN_TEST(test_clock_jumping_backwards_fetches_again);
  RUN_TEST(test_failure_after_clock_jumped_before_last_good_fetch_renders_no_connection);
  RUN_TEST(test_board_too_stale_to_show_is_never_shown_again);
  RUN_TEST(test_view_receives_only_as_many_departures_as_it_has_rows);
  RUN_TEST(test_fresh_render_shows_only_departures_on_listed_platforms);
  RUN_TEST(test_stale_render_shows_only_departures_on_listed_platforms);
  return UNITY_END();
}
