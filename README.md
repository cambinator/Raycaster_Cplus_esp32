### Raycaster game for ESP32

**This project has been tested with the esp-idf v5.0**

#### Specifications

* Made for **ST7789V** based TFT module 240*135 pixels in 4-wire SPI mode. This LCD is used in TTGO LilyGO
* **16-bit (RGB)** color mode is used
* Includes a minimalistic driver and graphics library for **ST7789V** based on examples in ESP IDF directory (https://github.com/espressif/esp-idf/tree/master/examples/peripherals/spi_master/lcd),
  some features are taken from loboris tft library(https://github.com/loboris/ESP32_TFT_library)

#### Grafic library

* **Driver functions**
  * **vLcd_Init** Init the LCD screen
  * **vSend_Frame** Sends an array of pixels to the screen at the specified location
  * **vBlack_Screen** Fills the screen with black color
  * **vSend_Pixel** Sends one pixel to the specified location
  * **vSet_Screen_Rotation** Rotates the screen

* **Graphics drawing functions**:
  * **vSend_Line_Horizontal** Draws a horizontal line
  * **vSend_Line_Vertical** Draws a vertical line
  * **vSend_Line** Draws thin line from x0, y0 to x1, y1
  * **vSend_Rect** Draws a rectangular frame
  * **vSend_Filled_Rect** Draws a filled rectangle

* **Text Print Functions**
  * **vSet_Font** Uses embedded fonts
  * **vSet_Font_Transparency** 
  * **vSet_Font_Color**
  * **vSet_Font_Backgr_Color** For a non-transparent font
  * **iChar_Print** Prints one char at a specified position on the screen
  * **xText_Print** Prints a null terminated string at the specified location 

* **Global variables**
  * **lcd_width** Current lcd width according to the screen rotation
  * **lcd_height**
  * **y_shift** TFT buffer is shifted, so correct coordinates must be adjusted
  * **x_shift**
  * **screen_rotation**
  * **current_font**
  * **device_tft** TFT device handle

#### Game

* Based on the raycasting process, using DDA method, the main part of the raycasting logic was taken from [lodev.org](https://lodev.org/cgtutor/index.html)
* The Game uses embedded maps stored in .c files, however, maps can be created in TileD editor and transformed directly to .c file with a python script
* Uses a minimalistic button library, button pins can be set in the main.c file. Default pins are:
  * **BUTTON1 21**
  * **BUTTON2 22**
  * **BUTTON3 17**
  * **BUTTON4 15**
  * **BUTTON5 13**
  * **BUTTON6 12**
* Uses textures, pre-loaded in the SPIFFS file system, the process of generating and flashing a SPIFFS image is described below

#### Display Connection

* If TTGO LilyGO is used, no changes are required
* If an external LCD module is used, the default pins used are:
  * **PIN_NUM_MISO 25**
  * **PIN_NUM_MOSI 23**
  * **PIN_NUM_CLK  19**
  * **PIN_NUM_CS   22**
  * **PIN_NUM_DC   21**
  * **PIN_NUM_RST  18**   Reset
  * **PIN_NUM_BCKL 5**    Backlight

**TTGO or custom board can be set using `idf.py menuconfig` in Components -> LCD Driver Configuration or directly in the Lcd_Simple_Driver.h file**

#### Building

* Clone the repository into your esp folder 
* Run `get_idf` in the project directory
* Move to "spiffs_images" directory and run `esptool.py -p <PORT> write_flash 0x200000 image.bin` to flash SPIFFS image to device
* Go back to the main directory, execute `idf.py menuconfig` and configure: 
  * **Serial flasher config** - set flash size to at least 4Mb
  * **Partition table -> Partition table -> custom partition table** select partitions.csv, included in the project
  * **Components -> LCD Driver Configuration -> LCD module type** and choose TTGO or Wroom(actually can be any other) kit.
* Build and flash the example `idf.py build && idf.py -p <PORT> flash monitor`
* To use the game, minimum two buttons must be connected
	* **Button 2 and 6 to select the game in the menu
	* **Button 6 to rotate the player and button 2 to move forward
	* **You can set and use the standard 2 buttons, that are present on TTGO and WROOM board

---
### Custom textures and maps
* To prepare custom textures create bin file from png files using Conv_png_to_bin.py script, located in **graphics** directory
* The custom texture should replace an existing texture with the same name and size, or add it manually in **texture_t::textures_load** function
* Add bin files to **spiffs_images** directory
* Run `python spiffsgen.py 1048576 image image.bin` to generate a 1Mb binary SPIFFS image from the files in **spiffs_images/image**
* The custom partition file reserves 1MB for SPIFFS.
* Custom maps can be created using the TileD editor. Example TileD maps cam be found in **graphics** directory.
* Save the map in csv format, then convert it to a .c file using the TileMapToC script. Maps in .c format are stored in **main/maps** directory
* The created map should replace an existing map with the same name, because all maps are build-in
---

### Tested on
* **TTGO LilyGO**
* **ESP32-WROOOM-KIT, ST7789V controller, 240x135**
