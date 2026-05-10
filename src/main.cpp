#include <iostream>
#include <BSPReader.hpp>

int main(void) {
	const char* path = "C:/Users/jrk/Desktop/half_life_KINTEK.bsp";
	BSPReader bsp(path);

	bsp.printHeader();
}