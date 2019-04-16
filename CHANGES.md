Here is a list of changes which can break your existing code based on ESP Jumpstart.

### Factory NVS variables changed from string to blob encoding type.

*commit 4fbc0560df16b82b12b86dc1e13436ffce7cac06*

7\_mfg/mfg\_config.csv now uses NVS blobs instead of strings since NVS strings cannot
be larger than 1984 bytes. In some cases the certificate size can go beyond this number.

### conn\_mgr\_prov : API changes for more clarity, flexibility and control

*commit ff311c666a24ccb0fffa06da399f016fce66bd7e*
    
List of changes :

* new APIs:
  * `conn_mgr_prov_config_t` : Configuration structure for specifying:
    * the provisioning `scheme` of type `conn_mgr_prov_scheme_t` which has 2 ready to use values:
      * `conn_mgr_prov_scheme_ble` : for provisioning over BLE transport + GATT server
      * `conn_mgr_prov_scheme_softap` : for provisioning over SoftAP transport + HTTP server
    * `scheme_event_handler` for specifying an event handler which may be required for conditioning the state of the program prior to starting the user app after provisioning is complete. When using `conn_mgr_prov_scheme_ble` one can choose one of the following available scheme specific handlers :
      * `CMP_EVENT_HANDLER_NONE` : No scheme specific event handling
      * `CMP_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM` : Free BT/BLE memory (when user app doesn't require these after provisioning is over)
      * `CMP_SCHEME_BLE_EVENT_HANDLER_FREE_BT` : Free only BT memory (when user app requires BLE to function even after provisioning is over)
      * `CMP_SCHEME_BLE_EVENT_HANDLER_FREE_BLE` : Free only BLE memory (when user app requires BT to function)
    * `app_event_handler` for specifying application specific event handler
  * `conn_mgr_prov_init()` : For initializing the manager with a specified configuration
  * `conn_mgr_prov_wait()` : Blocking wait for provisioning to complete
  * `conn_mgr_prov_deinit()` : For de-initializing the manager and freeing its resources
  * `conn_mgr_prov_stop_provisioning()` : For stopping an active provisioning service
* modified APIs:
  * `conn_mgr_prov_start_provisioning()` : Doesn't accept prov_type anymore. Instead the mode (BLE/SoftAP) is configured during init
  * `conn_mgr_prov_event_handler()` : Returns error when provisioning is not running
  * `conn_mgr_prov_t` rename to `conn_mgr_prov_scheme_t`
  * `conn_mgr_prov_mode_ble` rename to `conn_mgr_prov_scheme_ble`
  * `conn_mgr_prov_mode_softap` renamed to `conn_mgr_prov_scheme_softap`
  * Macros with prefix `CM_*` renamed to that with prefix `CMP_*`
* Examples `4_network_config`, `5_cloud`, `6_ota` and `7_mfg` modified as per new APIs. Releasing of BT/BLE memory pre/post provisioning is taken care of by manager as scheme handler `CMP_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM` is used, even in the case when provisioning is not started. If provisioning service is started, de-initialization is triggered by the application specific event handler `prov_event_handler()` after provisioning finishes. This frees the main app from the burden of waiting for provisioning to finish and then calling manager de-init.
