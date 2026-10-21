#pragma once

#include <algorithm>
#include <string>
#include <vector>

// Route Direction ID: "<route name>|<destination id>", the destination empty when unknown.
inline std::string routeDirectionId(const std::string& routeName,
                                    const std::string& destinationId) {
  return routeName + "|" + destinationId;
}

// An empty selection means every route direction is selected.
inline bool isSelectedRouteDirection(const std::vector<std::string>& selectedIds,
                                     const std::string& id) {
  return selectedIds.empty() ||
         std::find(selectedIds.begin(), selectedIds.end(), id) != selectedIds.end();
}
