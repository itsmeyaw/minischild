#include "provider/mvv/departure_provider.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include <ctime>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "provider/mvv/available_lines_parser.h"
#include "provider/mvv/departure_parser.h"
#include "provider/mvv/departures_url.h"
#include "provider/mvv/stop_name_parser.h"

namespace {
// USERTrust RSA root; anchors www.mvv-muenchen.de's Sectigo chain. Expires 2038-01-18.
constexpr char kUserTrustRsaRootCa[] = R"(-----BEGIN CERTIFICATE-----
MIIF3jCCA8agAwIBAgIQAf1tMPyjylGoG7xkDjUDLTANBgkqhkiG9w0BAQwFADCB
iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl
cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV
BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAw
MjAxMDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBiDELMAkGA1UEBhMCVVMxEzARBgNV
BAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0plcnNleSBDaXR5MR4wHAYDVQQKExVU
aGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNVBAMTJVVTRVJUcnVzdCBSU0EgQ2Vy
dGlmaWNhdGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIK
AoICAQCAEmUXNg7D2wiz0KxXDXbtzSfTTK1Qg2HiqiBNCS1kCdzOiZ/MPans9s/B
3PHTsdZ7NygRK0faOca8Ohm0X6a9fZ2jY0K2dvKpOyuR+OJv0OwWIJAJPuLodMkY
tJHUYmTbf6MG8YgYapAiPLz+E/CHFHv25B+O1ORRxhFnRghRy4YUVD+8M/5+bJz/
Fp0YvVGONaanZshyZ9shZrHUm3gDwFA66Mzw3LyeTP6vBZY1H1dat//O+T23LLb2
VN3I5xI6Ta5MirdcmrS3ID3KfyI0rn47aGYBROcBTkZTmzNg95S+UzeQc0PzMsNT
79uq/nROacdrjGCT3sTHDN/hMq7MkztReJVni+49Vv4M0GkPGw/zJSZrM233bkf6
c0Plfg6lZrEpfDKEY1WJxA3Bk1QwGROs0303p+tdOmw1XNtB1xLaqUkL39iAigmT
Yo61Zs8liM2EuLE/pDkP2QKe6xJMlXzzawWpXhaDzLhn4ugTncxbgtNMs+1b/97l
c6wjOy0AvzVVdAlJ2ElYGn+SNuZRkg7zJn0cTRe8yexDJtC/QV9AqURE9JnnV4ee
UB9XVKg+/XRjL7FQZQnmWEIuQxpMtPAlR1n6BB6T1CZGSlCBst6+eLf8ZxXhyVeE
Hg9j1uliutZfVS7qXMYoCAQlObgOK6nyTJccBz8NUvXt7y+CDwIDAQABo0IwQDAd
BgNVHQ4EFgQUU3m/WqorSs9UgOHYm8Cd8rIDZsswDgYDVR0PAQH/BAQDAgEGMA8G
A1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAFzUfA3P9wF9QZllDHPF
Up/L+M+ZBn8b2kMVn54CVVeWFPFSPCeHlCjtHzoBN6J2/FNQwISbxmtOuowhT6KO
VWKR82kV2LyI48SqC/3vqOlLVSoGIG1VeCkZ7l8wXEskEVX/JJpuXior7gtNn3/3
ATiUFJVDBwn7YKnuHKsSjKCaXqeYalltiz8I+8jRRa8YFWSQEg9zKC7F4iRO/Fjs
8PRF/iKz6y+O0tlFYQXBl2+odnKPi4w2r78NBc5xjeambx9spnFixdjQg3IM8WcR
iQycE0xyNN+81XHfqnHd4blsjDwSXWXavVcStkNr/+XeTWYRUc+ZruwXtuhxkYze
Sf7dNXGiFSeUHM9h4ya7b6NnJSFd5t0dCy5oGzuCr+yDZ4XUmFF0sbmZgIn/f3gZ
XHlKYC6SQK5MNyosycdiyA5d9zZbyuAlJQG03RoHnHcAP9Dc1ew91Pq7P8yF1m9/
qS3fuQL39ZeatTXaw2ewh0qpKJ4jjv9cJ2vhsE/zB+4ALtRZh8tSQZXq9EfX7mRB
VXyNWQKV3WKdwrnuWih0hKWbt5DHDAff9Yk2dDLWKMGwsAvgnEzDHNb842m1R0aB
L6KCq9NjRHDEjf8tM7qtj3u1cIiuPhnPQCjY/MiQu12ZIvVS5ljFH4gxQ+6IHdfG
jjxDah2nGN59PRbxYvnKkKj9
-----END CERTIFICATE-----
)";

// Observed on Arduino core 3.x: TLS reads only see newly arrived data if the caller yields while
// waiting, which neither NetworkClient::readBytes nor Stream::timedRead does.
struct WaitingReader {
  Stream& stream;
  int read() {
    const unsigned long start = millis();
    while (stream.available() <= 0) {
      if (millis() - start >= stream.getTimeout()) return -1;
      delay(1);
    }
    return stream.read();
  }
  size_t readBytes(char* buffer, size_t length) {
    size_t count = 0;
    for (int c; count < length && (c = read()) >= 0; count++) buffer[count] = static_cast<char>(c);
    return count;
  }
};

// GETs url and stream-parses the body; HTTP failures become a FetchError in Result.
template <typename Result, typename Parse>
Result getJson(const std::string& url, Parse parse) {
  WiFiClientSecure client;
  client.setCACert(kUserTrustRsaRootCa);
  HTTPClient http;
  // HTTP/1.0 has no chunked encoding, so the body can be stream-parsed.
  http.useHTTP10(true);
  if (!http.begin(client, url.c_str())) return FetchError{"HTTP begin failed"};
  const int code = http.GET();
  if (code < 200 || code >= 300) {
    std::string message = "HTTP " + std::to_string(code);
    if (code < 0) message += ": " + std::string(HTTPClient::errorToString(code).c_str());
    http.end();
    return FetchError{message};
  }
  WaitingReader body{http.getStream()};
  Result result = parse(body);
  http.end();
  return result;
}
}  // namespace

MvvDepartureProvider::MvvDepartureProvider(std::string stationId,
                                           std::vector<std::string> routeDirectionIds)
    : stationId_(std::move(stationId)),
      routeDirections_(std::move(routeDirectionIds),
                       [this] {
                         return getJson<RouteDirectionsResult>(
                             mvvAvailableLinesUrl(stationId_),
                             [](WaitingReader& body) { return parseMvvAvailableLines(body); });
                       }),
      stopName_([this] {
        return getJson<StopNameResult>(mvvStopFinderUrl(stationId_), [this](WaitingReader& body) {
          return parseMvvStopName(body, stationId_);
        });
      }) {}

FetchResult MvvDepartureProvider::fetch() {
  FetchResult result = fetchBoard();
  // A Station with no departures has no name on its board; look it up instead.
  if (DepartureBoard* board = std::get_if<DepartureBoard>(&result);
      board && board->stopName.empty()) {
    board->stopName = stopName_.get();
  }
  return result;
}

void MvvDepartureProvider::select(const BoardSelection& selection) {
  stationId_ = selection.stationId;
  routeDirections_.select(selection.routeDirectionIds);
  stopName_.reset();
}

FetchResult MvvDepartureProvider::fetchBoard() {
  const RouteDirectionsResult routeDirections = routeDirections_.get();
  if (const FetchError* error = std::get_if<FetchError>(&routeDirections)) return *error;
  const std::vector<std::string>& ids = std::get<std::vector<std::string>>(routeDirections);
  const time_t now = time(nullptr);
  // MVV rejects a departures request without lines; a Station without Route Directions has none.
  if (ids.empty()) return DepartureBoard{"", now, {}};
  return getJson<FetchResult>(mvvDeparturesUrl(stationId_, ids, now),
                              [now](WaitingReader& body) { return parseMvvDepartures(body, now); });
}
