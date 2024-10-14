#pragma once

#include <cstdint>
#include <string>

#include "GUI/UI/UIHouseEnums.h"
#include "Utility/IndexedArray.h"

#include "Engine/Data/BuildingData.h"

class Blob;

void initializeBuildings(const Blob &buildings);

// TODO(captainurist): move HOUSE_* to Engine/Data
extern IndexedArray<BuildingData, HOUSE_FIRST, HOUSE_LAST> buildingTable;
