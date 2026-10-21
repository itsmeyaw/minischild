#pragma once

#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include <string>

#include "provider/departure_provider.h"

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

// GETs url over TLS pinned to rootCa and stream-parses the body; HTTP failures become a FetchError
// in Result.
template <typename Result, typename Parse>
Result getJson(const char* rootCa, const std::string& url, Parse parse) {
  WiFiClientSecure client;
  client.setCACert(rootCa);
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
