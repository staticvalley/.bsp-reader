# BSP30 reader

a simple reader of the .bsp format of goldsrc map files. Created as a hobby parser for using .bsp files created and compiled in the J.A.C.K editor into my own 3D opengl engine.

#### Usage:

The reader can be used as so:

```
#include <BSPReader.hpp>
#include <BSPLumpDefs.hpp>

int main(void) {
	const char* path = "assets/maps/de_dust2.bsp";
	BSPReader bsp(path);

	bsp.printHeader();
	bsp.processAllLumps();

	BSPTexture someTexture = bsp.texture[0];
}
```

Or, lumps can be processed individually based on what is needed:

```
bsp.processLump(LumpType::TEXTURES);

BSPTexture someTexture = bsp.texture[3];
```

#### Todo/Known Issues:

- invalid file handling needs work: missing a few edge cases and could more "gracefully" handle errors.
- only mipmap level 0 (unscaled) is parsed into the `BSPTexture` type (missing 1/2, 1/4, 1/8 textures)
- clean up imports