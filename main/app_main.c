
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <nvs.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "http.h"
#include "driver/i2s.h"

#include "ui.h"
#include "spiram_fifo.h"
#include "audio_renderer.h"
#include "web_radio.h"
#include "playerconfig.h"
#include "app_main.h"
#include "mdns_task.h"
#ifdef CONFIG_BT_SPEAKER_MODE
#include "bt_speaker.h"
#endif

/////////////////////////////////////////////////////
///////////////////////////
#include "bt_config.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
//#include "esp_wifi.h"
#include "xi2c.h"
#include "fonts.h"
#include "ssd1306.h"
#include "nvs_flash.h"
//#define BLINK_GPIO 4
#define I2C_EXAMPLE_MASTER_SCL_IO    14    /*!< gpio number for I2C master clock */////////////
#define I2C_EXAMPLE_MASTER_SDA_IO    13    /*!< gpio number for I2C master data  *//////////////
#define I2C_EXAMPLE_MASTER_NUM I2C_NUM_1   /*!< I2C port number for master dev */
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE   0   /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE   0   /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_FREQ_HZ    100000     /*!< I2C master clock frequency */

/* */

static uint8_t station_no = 0;
const char *stations[] = {
  "http://wbgo.streamguys.net/wbgo96",
  "http://wbgo.streamguys.net/thejazzstream",
  "http://icecast.omroep.nl/3fm-sb-mp3",
};

char* play_url() {
  return (char*)stations[station_no];
}

xSemaphoreHandle print_mux;

static void i2c_test(void)
{
    char *url = play_url();
  
    SSD1306_GotoXY(40, 4);
    SSD1306_Puts("ESP32", &Font_11x18, SSD1306_COLOR_WHITE);
    
    SSD1306_GotoXY(2, 20);
    #ifdef CONFIG_BT_SPEAKER_MODE /////bluetooth speaker mode/////
    SSD1306_Puts("PCM5102 BT speaker", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(2, 30);
    SSD1306_Puts("my device name is", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(2, 39);
    SSD1306_Puts(dev_name, &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(16, 53);
    SSD1306_Puts("Yeah! Speaker!", &Font_7x10, SSD1306_COLOR_WHITE);
    #else ////////for webradio mode display////////////////
    SSD1306_Puts("PCM5102A webradio", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_GotoXY(2, 30);
    
    SSD1306_Puts(url, &Font_7x10, SSD1306_COLOR_WHITE);
    if (strlen(url) > 18)  {
      SSD1306_GotoXY(2, 39);
      SSD1306_Puts(url + 18, &Font_7x10, SSD1306_COLOR_WHITE);
    }
    SSD1306_GotoXY(16, 53);

	tcpip_adapter_ip_info_t ip_info;
	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);
	SSD1306_GotoXY(2, 53);
	SSD1306_Puts("IP:", &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_Puts(ip4addr_ntoa(&ip_info.ip), &Font_7x10, SSD1306_COLOR_WHITE);    
    #endif

    
    

    /* Update screen, send changes to LCD */
    SSD1306_UpdateScreen();
    
    //    while (1) {
    //      /* Invert pixels */
    //    SSD1306_ToggleInvert();
    
    /* Update screen */
    //  SSD1306_UpdateScreen();
    
    /* Make a little delay */
    //   vTaskDelay(50);
    //  }
    
}

/**
 * @brief i2c master initialization
 */
static void i2c_example_master_init()
{
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode,
                       I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                       I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
}

/*
 void app_main()
 {
 print_mux = xSemaphoreCreateMutex();
 i2c_example_master_init();
 SSD1306_Init();
 
 xTaskCreate(i2c_test, "i2c_test", 1024, NULL, 10, NULL);
 }
 */


//////////////////////////////////////////////////////////////////


#define WIFI_LIST_NUM   10


#define TAG "main"








//Priorities of the reader and the decoder thread. bigger number = higher prio
#define PRIO_READER configMAX_PRIORITIES -3
#define PRIO_MQTT configMAX_PRIORITIES - 3
#define PRIO_CONNECT configMAX_PRIORITIES -1



/* event handler for pre-defined wifi events */
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    EventGroupHandle_t wifi_event_group = ctx;

    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;

    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;

    default:
        break;
    }

    return ESP_OK;
}

static void initialise_wifi(EventGroupHandle_t wifi_event_group)
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, wifi_event_group) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_FLASH) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}


static void set_wifi_credentials()
{
    wifi_config_t current_config;
    esp_wifi_get_config(WIFI_IF_STA, &current_config);

    // no changes? return and save a bit of startup time
    if(strcmp( (const char *) current_config.sta.ssid, WIFI_AP_NAME) == 0 &&
       strcmp( (const char *) current_config.sta.password, WIFI_AP_PASS) == 0)
    {
        ESP_LOGI(TAG, "keeping wifi config: %s", WIFI_AP_NAME);
        return;
    }

    // wifi config has changed, update
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_AP_NAME,
            .password = WIFI_AP_PASS,
            .bssid_set = 0,
        },
    };

    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    esp_wifi_disconnect();
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    ESP_LOGI(TAG, "connecting");
    esp_wifi_connect();
}

static void init_hardware()
{
    nvs_flash_init();

    // init UI
    // ui_init(GPIO_NUM_32);

    //Initialize the SPI RAM chip communications and see if it actually retains some bytes. If it
    //doesn't, warn user.
    if (!spiRamFifoInit()) {
        printf("\n\nSPI RAM chip fail!\n");
        while(1);
    }

    ESP_LOGI(TAG, "hardware initialized");
}

static void start_wifi()
{
    ESP_LOGI(TAG, "starting network");

    /* FreeRTOS event group to signal when we are connected & ready to make a request */
    EventGroupHandle_t wifi_event_group = xEventGroupCreate();

    /* init wifi */
    ui_queue_event(UI_CONNECTING);
    initialise_wifi(wifi_event_group);
    set_wifi_credentials();

    /* start mDNS */
    // xTaskCreatePinnedToCore(&mdns_task, "mdns_task", 2048, wifi_event_group, 5, NULL, 0);

    /* Wait for the callback to set the CONNECTED_BIT in the event group. */
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);

    ui_queue_event(UI_CONNECTED);
}

static renderer_config_t *create_renderer_config()
{
    renderer_config_t *renderer_config = calloc(1, sizeof(renderer_config_t));

    renderer_config->bit_depth = I2S_BITS_PER_SAMPLE_16BIT;
    renderer_config->i2s_num = I2S_NUM_0;
    renderer_config->sample_rate = 44100;
    renderer_config->sample_rate_modifier = 1.0;
    renderer_config->output_mode = AUDIO_OUTPUT_MODE;

    if(renderer_config->output_mode == I2S_MERUS) {
        renderer_config->bit_depth = I2S_BITS_PER_SAMPLE_32BIT;
    }

    if(renderer_config->output_mode == DAC_BUILT_IN) {
        renderer_config->bit_depth = I2S_BITS_PER_SAMPLE_16BIT;
    }

    return renderer_config;
}

void init_station(int sw) {
  nvs_handle h;
  const char *key = "Station No";
  nvs_open("STATION", NVS_READWRITE, &h);
  if (nvs_get_u8(h, key, &station_no) != ESP_OK || station_no >= sizeof(stations)/sizeof(char*)) {
    station_no = 0;
  }
  ESP_LOGI(TAG, "%s:%d %s", key, (int)station_no, stations[station_no]);
  nvs_set_u8(h, key, station_no + (sw ? 1: 0));
  nvs_commit(h);
  nvs_close(h);
}

static void start_web_radio()
{
    init_station(0);

    // init web radio
    web_radio_t *radio_config = calloc(1, sizeof(web_radio_t));
    radio_config->url = play_url(); /* PLAY_URL; */

    // init player config
    radio_config->player_config = calloc(1, sizeof(player_t));
    radio_config->player_config->command = CMD_NONE;
    radio_config->player_config->decoder_status = UNINITIALIZED;
    radio_config->player_config->decoder_command = CMD_NONE;
    radio_config->player_config->buffer_pref = BUF_PREF_SAFE;
    radio_config->player_config->media_stream = calloc(1, sizeof(media_stream_t));

    // init renderer
    renderer_init(create_renderer_config());

    // start radio
    web_radio_init(radio_config);
    web_radio_start(radio_config);
}

/**
 * entry point
 */
void app_main()
{
    print_mux = xSemaphoreCreateMutex();
    ESP_LOGI(TAG, "starting app_main()");
    ESP_LOGI(TAG, "RAM left: %u", esp_get_free_heap_size());

    init_hardware();

#ifdef CONFIG_BT_SPEAKER_MODE
    bt_speaker_start(create_renderer_config());
#else
    start_wifi();
    start_web_radio();
#endif
    i2c_example_master_init();
    SSD1306_Init();
    i2c_test();
    ESP_LOGI(TAG, "RAM left %d", esp_get_free_heap_size());
    // ESP_LOGI(TAG, "app_main stack: %d\n", uxTaskGetStackHighWaterMark(NULL));
}
