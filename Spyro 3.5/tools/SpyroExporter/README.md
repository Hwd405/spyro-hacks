A tool used to export sections of memory for use in Spyro 3.5. For this you'll need a copy of SpyroEdit and ePSXe. For more information on texture hacking in SpyroEdit, see [this video](https://www.youtube.com/watch?v=hwNgLdwGWOQ).

When editing textures, skyboxes, and lighting / far LOD, the following steps were taken:
* Open Spyro 3 in SpyroEdit, using ePSXe.
* Export the textures, edit them as required, and import them into the game using SpyroEdit.
* Edit the world lighting and skybox as required in SpyroEdit.
* Export the skybox to a file using SpyroEdit. This file will be used directly when importing the skybox back into the game.
* Update the epsxeVramAddress to wherever VRAM starts in ePSXe's memory.
  * I don't know of a simple way to find this, I would usually search for the byte array "53 46 22 31 41 41 22 62 33 33 66 E6 22 11 41 31" whilst in Sunrise Spring Home (the start of Sunrise Spring Home's VRAM), and this address would be the "epsxe.exe+0xXXXXX" address, if I recall correctly.
* Build and run SpyroExporter. Press enter when the program prompts you to.
* Files for the VRAM and environment (called "leveldata" by this tool) are exported (amongst other sections of memory). The VRAM will be useful as-is, but sections of the environment will need to be split out into their own files for them to be useful.
* The environment file is built from "components", each of which start with a 4-byte Little Endian value indicating the length of the component (or in other words the distance to the next component). The first of these components (with the length at the start removed) will serve as the texture file, and the second of these components (again with the length removed) will be used as the lighting / far LOD for the level.

The memory addresses will need to modified if you are using anything except Spyro 3 NTSC-U revision 0.

An example of how these files are implemented in 3.5 can be seen [here](https://github.com/Hwd405/spyro-hacks/blob/main/Spyro%203.5/src/UpdateMobys.cpp#L1785).

Some levels, such as Bamboo Terrace, also need an [additional update to each sublevel](https://github.com/Hwd405/spyro-hacks/blob/main/Spyro%203.5/src/UpdateMobys.cpp#L1792) due to some textures being swapped in each area. This can be done by exporting an extra VRAM file in each sublevel (making any additional texture edits to each one as required) and importing each sublevel's VRAM as shown, or by just importing the VRAM file already obtained for all sublevels instead (which might result in some weird textures).

Some levels have additional skyboxes for each sublevel. For these levels, you'll need to use the Skybox file as before but as seen [here](https://github.com/Hwd405/spyro-hacks/blob/main/Spyro%203.5/src/UpdateMobys.cpp#L3191).
