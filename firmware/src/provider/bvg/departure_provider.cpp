#include "provider/bvg/departure_provider.h"

#include <ctime>
#include <string>
#include <utility>
#include <vector>

#include "provider/bvg/departure_parser.h"
#include "provider/bvg/departures_url.h"
#include "util/https_json.h"

namespace {
// ISRG Root X2 (ECC P-384): the Let's Encrypt chain of v6.bvg.transport.rest (YE2, Root YE) ends
// at it. Pinned to the root, not the leaf or intermediates, which rotate. Expires 2040-09-17.
constexpr char kIsrgRootX2[] = R"(-----BEGIN CERTIFICATE-----
MIICGzCCAaGgAwIBAgIQQdKd0XLq7qeAwSxs6S+HUjAKBggqhkjOPQQDAzBPMQsw
CQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJuZXQgU2VjdXJpdHkgUmVzZWFyY2gg
R3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBYMjAeFw0yMDA5MDQwMDAwMDBaFw00
MDA5MTcxNjAwMDBaME8xCzAJBgNVBAYTAlVTMSkwJwYDVQQKEyBJbnRlcm5ldCBT
ZWN1cml0eSBSZXNlYXJjaCBHcm91cDEVMBMGA1UEAxMMSVNSRyBSb290IFgyMHYw
EAYHKoZIzj0CAQYFK4EEACIDYgAEzZvVn4CDCuwJSvMWSj5cz3es3mcFDR0HttwW
+1qLFNvicWDEukWVEYmO6gbf9yoWHKS5xcUy4APgHoIYOIvXRdgKam7mAHf7AlF9
ItgKbppbd9/w+kHsOdx1ymgHDB/qo0IwQDAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0T
AQH/BAUwAwEB/zAdBgNVHQ4EFgQUfEKWrt5LSDv6kviejM9ti6lyN5UwCgYIKoZI
zj0EAwMDaAAwZQIwe3lORlCEwkSHRhtFcP9Ymd70/aTSVaYgLXTWNLxBo1BfASdW
tL4ndQavEi51mI38AjEAi/V3bNTIZargCyzuFJ0nN6T5U6VR5CmD1/iQMVtCnwr1
/q4AaOeMSQ+2b1tbFfLn
-----END CERTIFICATE-----
)";
}  // namespace

BvgDepartureProvider::BvgDepartureProvider(std::string stationId,
                                           std::vector<std::string> routeDirectionIds)
    : stationId_(std::move(stationId)), routeDirectionIds_(std::move(routeDirectionIds)) {}

FetchResult BvgDepartureProvider::fetch() {
  const time_t now = time(nullptr);
  return getJson<FetchResult>(kIsrgRootX2, bvgDeparturesUrl(stationId_),
                              [now, this](WaitingReader& body) {
                                return parseBvgDepartures(body, now, routeDirectionIds_);
                              });
}
