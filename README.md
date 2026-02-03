# Izotrox
 An Experimental GUI Engine for Android and Linux devices.

 On Android, it renders all the graphics by sending them to the device framebuffer (located at `/dev/graphics/fb0`). All the rendering and rasterisation is done on the CPU itself. On Linux, it uses SDL2 (which probably uses hardware acceleration under the hood so it's slightly faster for operations like alpha blending). To check if your device can run Izotrox, see [Supported Android Devices](docs/supported_android_devices.md).
 
## Features
- (TODO) *Scriptable using AngelScript*
- INI-based theming engine with hot theme reloading, and a handy Theme Editor
- Works on Android and Desktop Linux
- Basic but extensible widget collection

## Limitations
- Only works on Linux-based systems due to the usage of Linux-specific APIs. As a result, Izotrox is not supported on Windows or macOS.
- No hardware acceleration on Android due to all rendering being done on the CPU itself.
- Unoptimized rendering method might cause lag on complex layouts.

## Libraries Used
- **[inifile-cpp](https://github.com/Rookfighter/inifile-cpp)** - for INI file parsing.
- **[Magic Enum C++](https://github.com/Neargye/magic_enum/)** - for enum name resolution.
- **[stb](https://github.com/nothings/stb)**
    - **[stb_image](https://github.com/nothings/stb/blob/master/stb_image.h)** - for reading and decoding image data.
    - **[stb_image_write](https://github.com/nothings/stb/blob/master/stb_image_write.h)** - for encoding and writing image data.
    - **[stb_truetype](https://github.com/nothings/stb/blob/master/stb_truetype.h)** - for reading TrueType fonts.

## Screenshots

- ### Izotrox
<img width="800" height="628" alt="izotrox01" src="https://github.com/user-attachments/assets/0c5a7247-93b3-4799-b17b-94e023ecb68d" />
<img width="800" height="628" alt="izotrox02" src="https://github.com/user-attachments/assets/ca3c69a9-ab42-4573-a417-ec5f12c80a41" />

- ### Theme Editor
<img width="800" height="661" alt="themeeditor01" src="https://github.com/user-attachments/assets/458fdb9f-ba31-48a8-a512-0db23e500d08" />
<img width="600" height="428" alt="themeeditor02" src="https://github.com/user-attachments/assets/e9e7ea15-56e0-4bd0-ad2b-aa9ad983b8f9" />

## License
Licensed under [Mozilla Public License 2.0](https://www.mozilla.org/en-US/MPL/2.0/). See [LICENSE](LICENSE) for more information
