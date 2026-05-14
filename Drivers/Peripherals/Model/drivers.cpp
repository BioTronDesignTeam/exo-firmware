#include "drivers.hpp"
// Credit: WARG efs-zeropilot for code structure!

// All the hardware handles
extern FDCAN_HandleTypeDef hfdcan1;

// Following best practices, we statically allocate memory for each driver class
// Note that we just put the class name in the brackets as alignas(className) is
// equivalent to alignas(alignof(className)) which aligns everything correctly!
alignas(ODRIVES1) static uint8_t odrives1Storage[sizeof(ODRIVES1)];

// Now we create global handles that we can use anywhere!
ODRIVES1 *odriveS1Handle = nullptr;

// Driver Initialization
void initDrivers() {
	odriveS1Handle = new (&odrives1Storage) ODRIVES1(&hfdcan1);
}
