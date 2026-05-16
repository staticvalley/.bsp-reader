#include <print>
#include <BSPFile.hpp>

int main(void) {
	const char* path = "C:/Users/jrk/Desktop/de_dust2.bsp";
	BSPFile bsp;

	bsp.load(path);

	for (BSPTexture t : bsp.textures()) {
		std::println("{} ({}x{}) {}", t.name, t.height, t.width, t.external);
	}
}