/* Connection Manager for Provisioning

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include <esp_log.h>
#include <esp_err.h>
#include <esp_wifi.h>
#include <esp_timer.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <protocomm.h>
#include <protocomm_security0.h>
#include <protocomm_security1.h>
#include <protocomm_ble.h>
#include <protocomm_httpd.h>

#include "conn_mgr_prov_priv.h"

static const char *TAG = "conn_mgr_prov";

typedef enum {
    CMP_STATE_IDLE = 0,
    CMP_STATE_STARTING = 1,
    CMP_STATE_STARTED = 2,
    CMP_STATE_CRED_RECV = 3,
    CMP_STATE_FAIL = 4,
    CMP_STATE_SUCCESS = 5,
    CMP_STATE_STOPPED = 6
} conn_mgr_prov_state_t;

/**
 * @brief  Context data for provisioning manager
 */
struct conn_mgr_prov_ctx {
    /* Provisioning manager configuration */
    conn_mgr_prov_config_t mgr_config;

    /* State of the provisioning service */
    conn_mgr_prov_state_t prov_state;

    /* Provisioning scheme configuration */
    void *prov_scheme_config;

    /* Protocomm handle */
    protocomm_t *pc;

    /* Type of security to use with protocomm */
    int security;

    /* Pointer to proof of possession */
    protocomm_security_pop_t pop;

    /* Handle to timer */
    esp_timer_handle_t timer;

    /* State of WiFi Station */
    wifi_prov_sta_state_t wifi_state;

    /* Code for WiFi station disconnection (if disconnected) */
    wifi_prov_sta_fail_reason_t wifi_disconnect_reason;
};

/* Mutex to lock/unlock access to provisioning singleton
 * context data. This is allocated only once on first init
 * and never deleted as conn_mgr_prov is a singleton */
static SemaphoreHandle_t prov_ctx_lock = NULL;

/* Pointer to provisioning context data */
static struct conn_mgr_prov_ctx *prov_ctx;

/* Count of used endpoint UUIDs */
static int endpoint_uuid_used = 0;

/* This function is called only after locking the control mutex */
static esp_err_t execute_event_cb(conn_mgr_prov_cb_event_t event_id)
{
    ESP_LOGD(TAG, "execute_event_cb : %d", event_id);
    esp_err_t err = ESP_OK;
    if (prov_ctx) {
        conn_mgr_prov_cb_func_t app_cb = prov_ctx->mgr_config.app_event_handler.event_cb;
        void *app_data = prov_ctx->mgr_config.app_event_handler.user_data;

        conn_mgr_prov_cb_func_t scheme_cb = prov_ctx->mgr_config.scheme_event_handler.event_cb;
        void *scheme_data = prov_ctx->mgr_config.scheme_event_handler.user_data;

        if (scheme_cb) {
            /* Unlock mutex first so that user has the freedom to
             * call conn_mgr_prov APIs from inside the callbacks
             * without causing deadlock */
            xSemaphoreGive(prov_ctx_lock);
            err = scheme_cb(scheme_data, event_id);
            xSemaphoreTake(prov_ctx_lock, portMAX_DELAY);
        }

        if (app_cb) {
            /* Unlock mutex first so that user has the freedom to
             * call conn_mgr_prov APIs from inside the callbacks
             * without causing deadlock */
            xSemaphoreGive(prov_ctx_lock);
            err = app_cb(app_data, event_id);
            xSemaphoreTake(prov_ctx_lock, portMAX_DELAY);
        }
    }
    return err;
}

/* This function is called only after locking the control mutex */
static esp_err_t conn_mgr_prov_start_service(const char *service_name, const char *service_key)
{
    const conn_mgr_prov_scheme_t *scheme = &prov_ctx->mgr_config.scheme;
    esp_err_t ret;

    /* Create new protocomm instance */
    prov_ctx->pc = protocomm_new();
    if (prov_ctx->pc == NULL) {
        ESP_LOGE(TAG, "Failed to create new protocomm instance");
        return ESP_FAIL;
    }

    prov_ctx->prov_scheme_config = scheme->new_config();
    if (prov_ctx->prov_scheme_config == NULL) {
        ESP_LOGE(TAG, "Failed to allocate provisioning mode config");
        protocomm_delete(prov_ctx->pc);
        return ESP_ERR_NO_MEM;
    }

    ret = scheme->set_config_service(prov_ctx->prov_scheme_config, service_name, service_key);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure service");
        scheme->delete_config(prov_ctx->prov_scheme_config);
        protocomm_delete(prov_ctx->pc);
        return ret;
    }

    ret = scheme->set_config_endpoint(prov_ctx->prov_scheme_config, "prov-session", 0xFF51);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure security endpoint");
        scheme->delete_config(prov_ctx->prov_scheme_config);
        protocomm_delete(prov_ctx->pc);
        return ret;
    }

    ret = scheme->set_config_endpoint(prov_ctx->prov_scheme_config, "prov-config", 0xFF52);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure wifi configuration endpoint");
        scheme->delete_config(prov_ctx->prov_scheme_config);
        protocomm_delete(prov_ctx->pc);
        return ret;
    }

    ret = scheme->set_config_endpoint(prov_ctx->prov_scheme_config, "proto-ver", 0xFF53);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure version endpoint");
        scheme->delete_config(prov_ctx->prov_scheme_config);
        protocomm_delete(prov_ctx->pc);
        return ret;
    }

    /* Maintain count of used up UUIDs */
    endpoint_uuid_used = 0xFF53;

    /* Execute user registered callback handler */
    ret = execute_event_cb(CMP_ENDPOINT_CONFIG);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error occurred while executing CMP_ENDPOINT_CONFIG event");
        scheme->delete_config(prov_ctx->prov_scheme_config);
        protocomm_delete(prov_ctx->pc);
        return ret;
    }

    /* Start provisioning */
    ret = scheme->prov_start(prov_ctx->pc, prov_ctx->prov_scheme_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start provisioning");
        scheme->delete_config(prov_ctx->prov_scheme_config);
        protocomm_delete(prov_ctx->pc);
        return ret;
    }

    /* Execute user registered callback handler */
    ret = execute_event_cb(CMP_PROV_START);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error occurred while executing CMP_PROV_START event");
        scheme->prov_stop(prov_ctx->pc);
        scheme->delete_config(prov_ctx->prov_scheme_config);
        protocomm_delete(prov_ctx->pc);
        return ret;
    }

    /* Set protocomm version verification endpoint for protocol */
    ret = protocomm_set_version(prov_ctx->pc, "proto-ver", "V0.1");
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set version endpoint");
        scheme->prov_stop(prov_ctx->pc);
        scheme->delete_config(prov_ctx->prov_scheme_config);
        protocomm_delete(prov_ctx->pc);
        return ret;
    }

    /* Set protocomm security type for endpoint */
    if (prov_ctx->security == 0) {
        ret = protocomm_set_security(prov_ctx->pc, "prov-session", &protocomm_security0, NULL);
    } else if (prov_ctx->security == 1) {
        ret = protocomm_set_security(prov_ctx->pc, "prov-session", &protocomm_security1, &prov_ctx->pop);
    } else {
        ESP_LOGE(TAG, "Unsupported protocomm security version %d", prov_ctx->security);
        ret = ESP_ERR_INVALID_ARG;
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set security endpoint");
        scheme->prov_stop(prov_ctx->pc);
        scheme->delete_config(prov_ctx->prov_scheme_config);
        protocomm_delete(prov_ctx->pc);
        return ret;
    }

    /* Add endpoint for provisioning to set wifi station config */
    ret = protocomm_add_endpoint(prov_ctx->pc, "prov-config",
                                 wifi_prov_config_data_handler, (void *) &wifi_prov_handlers);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set provisioning endpoint");
        scheme->prov_stop(prov_ctx->pc);
        scheme->delete_config(prov_ctx->prov_scheme_config);
        protocomm_delete(prov_ctx->pc);
        return ret;
    }

    /* Execute user registered callback handler */
    ret = execute_event_cb(CMP_ENDPOINT_ADD);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error occurred while executing CMP_ENDPOINT_ADD event");
        scheme->prov_stop(prov_ctx->pc);
        scheme->delete_config(prov_ctx->prov_scheme_config);
        protocomm_delete(prov_ctx->pc);
        return ret;
    }

    ESP_LOGI(TAG, "Provisioning started with : \n\tservice name = %s \n\tservice key = %s",
             service_name ? service_name : "<NULL>",
             service_key ? service_key : "<NULL>");
    return ESP_OK;
}

esp_err_t conn_mgr_prov_endpoint_configure(const char *ep_name)
{
    if (!prov_ctx_lock) {
        ESP_LOGE(TAG, "Provisioning manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = ESP_OK;

    xSemaphoreTake(prov_ctx_lock, portMAX_DELAY);
    if (prov_ctx && prov_ctx->prov_scheme_config) {
        err = prov_ctx->mgr_config.scheme.set_config_endpoint(
                prov_ctx->prov_scheme_config, ep_name, endpoint_uuid_used + 1);
    }
    xSemaphoreGive(prov_ctx_lock);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure provisioning endpoint");
    } else {
        endpoint_uuid_used++;
    }
    return err;
}

esp_err_t conn_mgr_prov_endpoint_add(const char *ep_name, protocomm_req_handler_t handler, void *user_ctx)
{
    if (!prov_ctx_lock) {
        ESP_LOGE(TAG, "Provisioning manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = ESP_OK;

    xSemaphoreTake(prov_ctx_lock, portMAX_DELAY);
    if (prov_ctx && prov_ctx->pc) {
        err = protocomm_add_endpoint(prov_ctx->pc, ep_name, handler, user_ctx);
    }
    xSemaphoreGive(prov_ctx_lock);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set provisioning endpoint");
    }
    return err;
}

void conn_mgr_prov_endpoint_remove(const char *ep_name)
{
    if (!prov_ctx_lock) {
        ESP_LOGE(TAG, "Provisioning manager not initialized");
        return;
    }

    xSemaphoreTake(prov_ctx_lock, portMAX_DELAY);
    if (prov_ctx && prov_ctx->pc) {
        protocomm_remove_endpoint(prov_ctx->pc, ep_name);
    }
    xSemaphoreGive(prov_ctx_lock);
}

/* This function is called only after locking the control mutex */
static void conn_mgr_prov_stop_service(void)
{
    if (!prov_ctx || !prov_ctx->pc) {
        return;
    }

    const conn_mgr_prov_scheme_t *scheme = &prov_ctx->mgr_config.scheme;

    ESP_LOGI(TAG, "Stopping provisioning");

    /* All the extra application added endpoints are also
     * removed automatically when prov_stop is called.
     * Hence, no need to give a callback with CMP_ENDPOINT_REMOVE. */
    scheme->prov_stop(prov_ctx->pc);

    /* Free config data */
    scheme->delete_config(prov_ctx->prov_scheme_config);

    /* Delete protocomm instance */
    protocomm_delete(prov_ctx->pc);

    prov_ctx->prov_scheme_config = NULL;
    prov_ctx->pc = NULL;

    /* Timer not needed anymore */
    if (prov_ctx->timer) {
        esp_timer_delete(prov_ctx->timer);
        prov_ctx->timer = NULL;
    }

    /* Free provisioning process data */
    free((void *)prov_ctx->pop.data);
    prov_ctx->pop.data = NULL;

    prov_ctx->prov_state = CMP_STATE_STOPPED;
    ESP_LOGI(TAG, "Provisioning stopped");
    execute_event_cb(CMP_PROV_END);
}

/* Task spawned by timer callback or by wifi_prov_done() */
static void stop_prov_task(void *arg)
{
    xSemaphoreTake(prov_ctx_lock, portMAX_DELAY);

    /* This delay is so that the phone app is notified first and then
     * the provisioning is stopped. Generally 100ms is enough. */
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    conn_mgr_prov_stop_service();
    esp_wifi_set_mode(WIFI_MODE_STA);

    xSemaphoreGive(prov_ctx_lock);
    vTaskDelete(NULL);
}

/* Callback to be invoked by timer */
static void _stop_prov_cb(void *arg)
{
    xTaskCreate(&stop_prov_task, "stop_prov", 2048, NULL, tskIDLE_PRIORITY, NULL);
}

/* Call this if provisioning is completed before the timeout occurs */
esp_err_t wifi_prov_done(void)
{
    if (!prov_ctx_lock) {
        ESP_LOGE(TAG, "Provisioning manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(prov_ctx_lock, portMAX_DELAY);
    if (prov_ctx && prov_ctx->timer) {
        esp_timer_stop(prov_ctx->timer);
        xTaskCreate(&stop_prov_task, "stop_prov", 2048, NULL, tskIDLE_PRIORITY, NULL);
    }
    xSemaphoreGive(prov_ctx_lock);
    return ESP_OK;
}

/* Event handler for starting/stopping provisioning.
 * To be called from within the context of the main
 * event handler.
 */
esp_err_t conn_mgr_prov_event_handler(void *ctx, system_event_t *event)
{
    /* For accessing reason codes in case of disconnection */
    system_event_info_t *info = &event->event_info;

    if (!prov_ctx_lock) {
        ESP_LOGE(TAG, "Provisioning manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    xSemaphoreTake(prov_ctx_lock, portMAX_DELAY);

    /* If pointer to provisioning application data is NULL
     * then provisioning manager is not running, therefore
     * return with error to allow the global handler to act */
    if (!prov_ctx) {
        xSemaphoreGive(prov_ctx_lock);
        return ESP_ERR_INVALID_STATE;
    }

    /* Only handle events when credential is received and
     * WiFi STA is yet to complete trying the connection */
    if (prov_ctx->prov_state != CMP_STATE_CRED_RECV) {
        xSemaphoreGive(prov_ctx_lock);
        return ESP_OK;
    }

    esp_err_t ret = ESP_OK;
    switch (event->event_id) {
    case SYSTEM_EVENT_STA_START:
        ESP_LOGI(TAG, "STA Start");
        /* Once configuration is received through protocomm,
         * device is started as station. Once station starts,
         * wait for connection to establish with configured
         * host SSID and password */
        prov_ctx->wifi_state = WIFI_PROV_STA_CONNECTING;
        break;

    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "STA Got IP");
        /* Station got IP. That means configuration is successful.
         * Schedule timer to stop provisioning app after 30 seconds. */
        prov_ctx->wifi_state = WIFI_PROV_STA_CONNECTED;
        prov_ctx->prov_state = CMP_STATE_SUCCESS;
        if (prov_ctx->timer) {
            esp_timer_start_once(prov_ctx->timer, 30000 * 1000U);
        }
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGE(TAG, "STA Disconnected");
        /* Station couldn't connect to configured host SSID */
        prov_ctx->wifi_state = WIFI_PROV_STA_DISCONNECTED;
        ESP_LOGE(TAG, "Disconnect reason : %d", info->disconnected.reason);

        /* Set code corresponding to the reason for disconnection */
        switch (info->disconnected.reason) {
        case WIFI_REASON_AUTH_EXPIRE:
        case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
        case WIFI_REASON_BEACON_TIMEOUT:
        case WIFI_REASON_AUTH_FAIL:
        case WIFI_REASON_ASSOC_FAIL:
        case WIFI_REASON_HANDSHAKE_TIMEOUT:
            ESP_LOGE(TAG, "STA Auth Error");
            ESP_LOGE(TAG, "Please reset to factory and retry provisioning");
            prov_ctx->wifi_disconnect_reason = WIFI_PROV_STA_AUTH_ERROR;
            prov_ctx->prov_state = CMP_STATE_FAIL;
            break;
        case WIFI_REASON_NO_AP_FOUND:
            ESP_LOGE(TAG, "STA AP Not found");
            ESP_LOGE(TAG, "Please reset to factory and retry provisioning");
            prov_ctx->wifi_disconnect_reason = WIFI_PROV_STA_AP_NOT_FOUND;
            prov_ctx->prov_state = CMP_STATE_FAIL;
            break;
        default:
            /* If none of the expected reasons,
             * retry connecting to host SSID */
            prov_ctx->wifi_state = WIFI_PROV_STA_CONNECTING;
            esp_wifi_connect();
        }
        break;

    default:
        /* This event is not intended to be handled by this handler.
         * Return ESP_FAIL to signal global event handler to take
         * control */
        ret = ESP_FAIL;
        break;
    }

    xSemaphoreGive(prov_ctx_lock);
    return ret;
}

esp_err_t wifi_prov_get_wifi_state(wifi_prov_sta_state_t *state)
{
    if (!prov_ctx_lock) {
        ESP_LOGE(TAG, "Provisioning manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(prov_ctx_lock, portMAX_DELAY);
    if (prov_ctx == NULL || state == NULL) {
        xSemaphoreGive(prov_ctx_lock);
        return ESP_FAIL;
    }

    *state = prov_ctx->wifi_state;
    xSemaphoreGive(prov_ctx_lock);
    return ESP_OK;
}

esp_err_t wifi_prov_get_wifi_disconnect_reason(wifi_prov_sta_fail_reason_t *reason)
{
    if (!prov_ctx_lock) {
        ESP_LOGE(TAG, "Provisioning manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(prov_ctx_lock, portMAX_DELAY);
    if (prov_ctx == NULL || reason == NULL) {
        xSemaphoreGive(prov_ctx_lock);
        return ESP_FAIL;
    }

    if (prov_ctx->wifi_state != WIFI_PROV_STA_DISCONNECTED) {
        xSemaphoreGive(prov_ctx_lock);
        return ESP_FAIL;
    }

    *reason = prov_ctx->wifi_disconnect_reason;
    xSemaphoreGive(prov_ctx_lock);
    return ESP_OK;
}

esp_err_t conn_mgr_prov_is_provisioned(bool *provisioned)
{
    if (!provisioned) {
        return ESP_ERR_INVALID_ARG;
    }

    *provisioned = false;

    if (!prov_ctx_lock) {
        ESP_LOGE(TAG, "Provisioning manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(prov_ctx_lock, portMAX_DELAY);
    if (prov_ctx &&
        prov_ctx->prov_state > CMP_STATE_IDLE &&
        prov_ctx->prov_state < CMP_STATE_FAIL) {
        xSemaphoreGive(prov_ctx_lock);
        return ESP_OK;
    }
    xSemaphoreGive(prov_ctx_lock);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&cfg) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init wifi");
        return ESP_FAIL;
    }

    /* Get WiFi Station configuration */
    wifi_config_t wifi_cfg;
    if (esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_cfg) != ESP_OK) {
        return ESP_FAIL;
    }

    if (strlen((const char *) wifi_cfg.sta.ssid)) {
        *provisioned = true;
        ESP_LOGI(TAG, "Found ssid %s",     (const char *) wifi_cfg.sta.ssid);
        ESP_LOGI(TAG, "Found password %s", (const char *) wifi_cfg.sta.password);
    }
    return ESP_OK;
}

esp_err_t wifi_prov_configure_sta(wifi_config_t *wifi_cfg)
{
    if (!prov_ctx_lock) {
        ESP_LOGE(TAG, "Provisioning manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(prov_ctx_lock, portMAX_DELAY);
    if (!prov_ctx) {
        ESP_LOGE(TAG, "Invalid state of Provisioning app");
        xSemaphoreGive(prov_ctx_lock);
        return ESP_FAIL;
    }
    if (prov_ctx->prov_state >= CMP_STATE_CRED_RECV) {
        ESP_LOGE(TAG, "WiFi credentials already received by provisioning app");
        xSemaphoreGive(prov_ctx_lock);
        return ESP_FAIL;
    }
    xSemaphoreGive(prov_ctx_lock);

    /* Initialize WiFi with default config */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&cfg) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init WiFi");
        return ESP_FAIL;
    }

    /* Configure WiFi as both AP and/or Station */
    xSemaphoreTake(prov_ctx_lock, portMAX_DELAY);
    if (esp_wifi_set_mode(prov_ctx->mgr_config.scheme.wifi_mode) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set WiFi mode");
        xSemaphoreGive(prov_ctx_lock);
        return ESP_FAIL;
    }
    xSemaphoreGive(prov_ctx_lock);

    /* Configure WiFi station with host credentials
     * provided during provisioning */
    if (esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_cfg) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set WiFi configuration");
        return ESP_FAIL;
    }
    /* (Re)Start WiFi */
    if (esp_wifi_start() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set WiFi configuration");
        return ESP_FAIL;
    }

    /* Connect to AP */
    if (esp_wifi_connect() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to connect WiFi");
        return ESP_FAIL;
    }

    /* Reset wifi station state for provisioning app */
    xSemaphoreTake(prov_ctx_lock, portMAX_DELAY);
    prov_ctx->wifi_state = WIFI_PROV_STA_CONNECTING;
    prov_ctx->prov_state = CMP_STATE_CRED_RECV;
    xSemaphoreGive(prov_ctx_lock);
    return ESP_OK;
}

esp_err_t conn_mgr_prov_init(conn_mgr_prov_config_t config)
{
    if (!prov_ctx_lock) {
       /* Create mutex if this is the first time init is being called.
        * This is created only once and never deleted because if some
        * other thread is trying to take this mutex while it is being
        * deleted from another thread then the reference may become
        * invalid and cause exception */
        prov_ctx_lock = xSemaphoreCreateMutex();
    }

    xSemaphoreTake(prov_ctx_lock, portMAX_DELAY);
    if (prov_ctx) {
        ESP_LOGE(TAG, "Provisioning manager already initialized");
        xSemaphoreGive(prov_ctx_lock);
        return ESP_ERR_INVALID_STATE;
    }

    /* Allocate memory for provisioning app data */
    prov_ctx = (struct conn_mgr_prov_ctx *) calloc(1, sizeof(struct conn_mgr_prov_ctx));
    if (!prov_ctx) {
        ESP_LOGE(TAG, "Error allocating memory for singleton instance");
        xSemaphoreGive(prov_ctx_lock);
        return ESP_ERR_NO_MEM;
    }

    prov_ctx->mgr_config = config;
    prov_ctx->prov_state = CMP_STATE_IDLE;

    execute_event_cb(CMP_INIT);
    xSemaphoreGive(prov_ctx_lock);
    return ESP_OK;
}

void conn_mgr_prov_wait(void)
{
    if (!prov_ctx_lock) {
        ESP_LOGE(TAG, "Provisioning manager not initialized");
        return;
    }

    while (1) {
        xSemaphoreTake(prov_ctx_lock, portMAX_DELAY);
        if (prov_ctx &&
            prov_ctx->prov_state > CMP_STATE_IDLE &&
            prov_ctx->prov_state < CMP_STATE_STOPPED) {
            xSemaphoreGive(prov_ctx_lock);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        break;
    }
    xSemaphoreGive(prov_ctx_lock);
}

void conn_mgr_prov_deinit(void)
{
    if (!prov_ctx_lock) {
        ESP_LOGE(TAG, "Provisioning manager not initialized");
        return;
    }

    /* Wait for conn_mgr_prov_start_provisioning() to finish,
     * else its possible that deinit gets called by another thread
     * while endpoint configure/add callbacks are being executed,
     * which will cause unwanted behavior. Also if deinit is called
     * in any of the endpoint related callbacks it will be stuck
     * forever. TODO : This issue must be eliminated in future by
     * getting rid of the add endpoint/configure endpoint events and
     * instead calling the related APIs before provisioning is started */
    while (1) {
        xSemaphoreTake(prov_ctx_lock, portMAX_DELAY);
        if (prov_ctx &&
            prov_ctx->prov_state == CMP_STATE_STARTING) {
            xSemaphoreGive(prov_ctx_lock);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        break;
    }

    if (!prov_ctx) {
        ESP_LOGE(TAG, "Provisioning manager not initialized");
        xSemaphoreGive(prov_ctx_lock);
        return;
    }

    if (prov_ctx->timer) {
        esp_timer_stop(prov_ctx->timer);
    }

    if (prov_ctx->prov_state >= CMP_STATE_STARTED &&
        prov_ctx->prov_state < CMP_STATE_STOPPED) {
        conn_mgr_prov_stop_service();
    }

    /* Extract the callbacks to be called post deinit */
    conn_mgr_prov_cb_func_t app_cb = prov_ctx->mgr_config.app_event_handler.event_cb;
    void *app_data = prov_ctx->mgr_config.app_event_handler.user_data;

    conn_mgr_prov_cb_func_t scheme_cb = prov_ctx->mgr_config.scheme_event_handler.event_cb;
    void *scheme_data = prov_ctx->mgr_config.scheme_event_handler.user_data;

    /* Free manager context */
    free(prov_ctx);
    prov_ctx = NULL;
    xSemaphoreGive(prov_ctx_lock);

    /* Execute deinit event callbacks */
    if (scheme_cb) {
        scheme_cb(scheme_data, CMP_DEINIT);
    }
    if (app_cb) {
        app_cb(app_data, CMP_DEINIT);
    }
}

esp_err_t conn_mgr_prov_start_provisioning(int security, const char *pop,
                                           const char *service_name, const char *service_key)
{
    if (!prov_ctx_lock) {
        ESP_LOGE(TAG, "Provisioning manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(prov_ctx_lock, portMAX_DELAY);
    if (!prov_ctx) {
        ESP_LOGE(TAG, "Provisioning manager not initialized");
        xSemaphoreGive(prov_ctx_lock);
        return ESP_ERR_INVALID_STATE;
    }

    if (prov_ctx->prov_state != CMP_STATE_IDLE) {
        ESP_LOGE(TAG, "Provisioning service already started");
        xSemaphoreGive(prov_ctx_lock);
        return ESP_ERR_INVALID_STATE;
    }

    /* Initialize app data */
    if (pop) {
        prov_ctx->pop.len = strlen(pop);
        prov_ctx->pop.data = malloc(prov_ctx->pop.len);
        if (!prov_ctx->pop.data) {
            ESP_LOGI(TAG, "Unable to allocate PoP data");
            xSemaphoreGive(prov_ctx_lock);
            return ESP_ERR_NO_MEM;
        }
        memcpy((void *)prov_ctx->pop.data, pop, prov_ctx->pop.len);
    }
    prov_ctx->security = security;

    /* Create timer object as a member of app data */
    esp_timer_create_args_t timer_conf = {
        .callback = _stop_prov_cb,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "stop_softap_tm"
    };
    esp_err_t err = esp_timer_create(&timer_conf, &prov_ctx->timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create timer");
        free((void *)prov_ctx->pop.data);
        xSemaphoreGive(prov_ctx_lock);
        return err;
    }

    /* Start provisioning service */
    prov_ctx->prov_state = CMP_STATE_STARTING;
    err = conn_mgr_prov_start_service(service_name, service_key);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Provisioning failed to start");
        esp_timer_delete(prov_ctx->timer);
        free((void *)prov_ctx->pop.data);
        xSemaphoreGive(prov_ctx_lock);
        return err;
    }
    prov_ctx->prov_state = CMP_STATE_STARTED;
    xSemaphoreGive(prov_ctx_lock);
    return ESP_OK;
}

void conn_mgr_prov_stop_provisioning(void)
{
    if (!prov_ctx_lock) {
        ESP_LOGE(TAG, "Provisioning manager not initialized");
        return;
    }

    /* Wait for conn_mgr_prov_start_provisioning() to finish.
     * See conn_mgr_prov_deinit() for reason. */
    while (1) {
        xSemaphoreTake(prov_ctx_lock, portMAX_DELAY);
        if (prov_ctx &&
            prov_ctx->prov_state == CMP_STATE_STARTING) {
            xSemaphoreGive(prov_ctx_lock);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        break;
    }

    if (!prov_ctx) {
        ESP_LOGE(TAG, "Provisioning manager not initialized");
        xSemaphoreGive(prov_ctx_lock);
        return;
    }

    if (prov_ctx->timer) {
        esp_timer_stop(prov_ctx->timer);
    }

    if (prov_ctx->prov_state >= CMP_STATE_STARTED &&
        prov_ctx->prov_state < CMP_STATE_STOPPED) {
        conn_mgr_prov_stop_service();
    }

    xSemaphoreGive(prov_ctx_lock);
}
