#include <print>
#include <BSPReader.hpp>
#include <BSPLumpDefs.hpp>

int main(void) {
	const char* path = "C:/Users/jrk/Desktop/de_dust2.bsp";
	BSPReader bsp(path);

	bsp.printHeader();
	bsp.processAllLumps();

	for (BSPTexture t : bsp.textures) {
		std::println("{} ({}x{}) {}", t.name, t.height, t.width, t.external);
	}
}