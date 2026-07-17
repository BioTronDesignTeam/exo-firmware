#pragma once
#include <new>
#include "odriveS1.hpp"
#include "BNO085.hpp"

extern ODRIVES1 *odriveS1Handle;
extern BNO085 *bno085Handle;

void initDrivers();
