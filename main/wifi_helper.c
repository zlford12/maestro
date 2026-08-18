#include "wifi_helper.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "lwip/ip4_addr.h"
#include "freertos/event_groups.h"

#define USE_DHCP 1

static constexpr char TAG[] = "WIFI";
esp_err_t wifi_ret;

static EventGroupHandle_t s_wifi_event_group;
#define WIFI_GOT_IP_BIT BIT0

static void on_got_ip(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)data;
    ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    xEventGroupSetBits(s_wifi_event_group, WIFI_GOT_IP_BIT);
}

wifi_config_t wifi_config = {
    .sta = {
        .ssid = "MNDT",
        .password = "1bmhkchMNDT"
    }
};

void WifiConfig()
{
    ESP_LOGI(TAG, "Configuring WiFi...");
    const wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    // enable non volatile memory
    wifi_ret = nvs_flash_init();
    if (wifi_ret == ESP_ERR_NVS_NO_FREE_PAGES || wifi_ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        wifi_ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(wifi_ret);

    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif == NULL) {
        netif = esp_netif_create_default_wifi_sta();
    }

    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL);

#if USE_DHCP
    // DHCP is enabled by default, nothing to do
#else
    if (netif) {
        esp_netif_dhcpc_stop(netif);
        esp_netif_ip_info_t ip_info;
        IP4_ADDR(&ip_info.ip, 192, 168, 88, 8);
        IP4_ADDR(&ip_info.gw, 192, 168, 88, 1);
        IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
        esp_netif_set_ip_info(netif, &ip_info);
    }
#endif

    // configure wifi
    ESP_ERROR_CHECK(wifi_ret = esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(wifi_ret = esp_wifi_start());
}

void WifiConnect()
{
    ESP_LOGI(TAG, "Connecting...");
    wifi_ret = esp_wifi_connect();
    ESP_ERROR_CHECK(wifi_ret);
}

bool WifiConnected()
{
    wifi_ap_record_t ap_info;
    wifi_ret = esp_wifi_sta_get_ap_info(&ap_info);

    if (wifi_ret != ESP_OK)
    {
        ESP_LOGI(TAG, "WiFi not connected");
        return false;
    }

    ESP_LOGI(TAG, "Connected to %s, waiting for IP...", ap_info.ssid);
    xEventGroupWaitBits(s_wifi_event_group, WIFI_GOT_IP_BIT, false, true, portMAX_DELAY);
    return true;
}
