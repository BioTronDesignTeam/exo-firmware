#include "drivers.hpp"
// Credit: WARG efs-zeropilot for code structure!

// All the hardware handles
extern FDCAN_HandleTypeDef hfdcan1;
extern I2C_HandleTypeDef hi2c1;

// Following best practices, we statically allocate memory for each driver class
// Note that we just put the class name in the brackets as alignas(className) is
// equivalent to alignas(alignof(className)) which aligns everything correctly!
alignas(ODRIVES1) static uint8_t odrives1Storage[sizeof(ODRIVES1)];
alignas(BNO085) static uint8_t bno085Storage[sizeof(BNO085)];

// Now we create global handles that we can use anywhere!
ODRIVES1 *odriveS1Handle = nullptr;
BNO085 *bno085Handle = nullptr;

// Driver Initialization
void initDrivers() {
	odriveS1Handle = new (&odrives1Storage) ODRIVES1(&hfdcan1);
	bno085Handle = new (&bno085Storage) BNO085(&hi2c1);
}
