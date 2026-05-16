# BSP30 reader

a simple reader of the .bsp format of goldsrc map files. Created as a hobby parser for using .bsp files created and compiled in the J.A.C.K editor into my own 3D opengl engine.

#### Usage:

The reader can be used as so:

```
#include <print>
#include <BSPFile.hpp>

int main(void) {
	const char* path = "scene/assets/de_dust2.bsp";
	BSPFile bsp;

	bsp.load(path);

	for (BSPTexture t : bsp.textures()) {
		std::println("{} ({}x{}) {}", t.name, t.height, t.width, t.external);
	}
}
```

#### Todo/Known Issues:

- invalid file handling needs work: missing a few edge cases and could more "gracefully" handle errors.
- only mipmap level 0 (unscaled) is parsed into the `BSPTexture` type (missing 1/2, 1/4, 1/8 textures)