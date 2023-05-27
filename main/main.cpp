#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#include "simple_button.hpp"
#include "Lcd_Simple_Driver.h"

#include "menu.hpp"
#include "game_play.hpp"

#define CLOCK_SPEED 26 * 1000 * 1000
#define MAX_TRANSFER 256 * 128 * 3

#define FILE_PATH "/spiffs/images/"

/* for TTGO LILYGO */
const gpio_num_t BUTTON1PIN = GPIO_NUM_21;
const gpio_num_t BUTTON2PIN = GPIO_NUM_22;
const gpio_num_t BUTTON3PIN = GPIO_NUM_17;
const gpio_num_t BUTTON4PIN = GPIO_NUM_15;
const gpio_num_t BUTTON5PIN = GPIO_NUM_13;
const gpio_num_t BUTTON6PIN = GPIO_NUM_12;

/********** global variables ******************/

button button_1; /*  move left / enter game */
button button_2; /*  move forward/backward */
button button_3; /*  move right */
button button_4; /*  rotate left */
button button_5; /*  shoot / choose game */
button button_6; /*  rotate right */


static const char *TAG = "Raycaster";

/************** main ***************/
extern "C" void app_main()
{
    esp_err_t ret;
    spi_device_handle_t spi;
    device_tft tft = NULL;

    spi_bus_config_t buscfg = {};
        buscfg.miso_io_num = PIN_NUM_MISO,
        buscfg.mosi_io_num = PIN_NUM_MOSI,
        buscfg.sclk_io_num = PIN_NUM_CLK,
        buscfg.quadwp_io_num = -1,
        buscfg.quadhd_io_num = -1,
        buscfg.max_transfer_sz = MAX_TRANSFER;
    
    spi_device_interface_config_t devcfg = {};
        devcfg.clock_speed_hz = CLOCK_SPEED,           /* Clock out at 26 MHz */
        devcfg.mode = 0,                               /* SPI mode 0 */
        devcfg.spics_io_num = PIN_NUM_CS,              /* CS pin */
        devcfg.queue_size = 7,                         /* We want to be able to queue 7 transactions at a time */
        devcfg.pre_cb = vLcd_Spi_Pretransfer_Callback, /* Specify pre-transfer callback to handle D/C line */
    
    /*  Initialize the SPI bus */
    ret = spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    /*  Attach the LCD to the SPI bus */
    ret = spi_bus_add_device(LCD_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
    /*  Initialize the LCD */
    vLcd_Init(spi);
    tft = (device_tft)spi;
    /*  Initialise SPIFFS */
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};
    ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    /************** Initialise buttons **************/
    button_1.init(BUTTON1PIN, STICKING_KEY);
    button_2.init(BUTTON2PIN, STICKING_KEY);
    button_3.init(BUTTON3PIN, STICKING_KEY);
    button_4.init(BUTTON4PIN, STICKING_KEY);
    button_5.init(BUTTON5PIN, SINGLE_CLICK);
    button_6.init(BUTTON6PIN, STICKING_KEY);

    vTaskDelay(500 / portTICK_PERIOD_MS);

    /*********** Run *************/
    ESP_LOGI(TAG, "Starting..\n");
    vSet_Screen_Rotation(spi, LANDSCAPE_FLIP);
    {
        start_screen_t start_screen;
        start_screen.draw(tft);
    }
    ESP_LOGI(TAG, "Minimal stack size = %d\n", configMINIMAL_STACK_SIZE); /*debug*/
    
    /************** Main task **************/
    int next_task = 0;
    for (;;) {
        if (next_task == 0){
            menu_t menu;
            next_task = menu.loop(tft);
		} else {
            game_t game(tft, next_task);
            next_task = game.loop();
		}
    }
}