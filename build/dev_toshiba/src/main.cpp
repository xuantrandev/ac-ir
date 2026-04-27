// Auto generated code by esphome
// ========== AUTO GENERATED INCLUDE BLOCK BEGIN ===========
#include "esphome.h"
using namespace esphome;
using std::isnan;
using std::min;
using std::max;
using namespace sensor;
using namespace text_sensor;
using namespace button;
using namespace climate;
using namespace binary_sensor;
logger::Logger *logger_logger_id;
web_server_base::WebServerBase *web_server_base_webserverbase_id;
captive_portal::CaptivePortal *captive_portal_captiveportal_id;
wifi::WiFiComponent *wifi_wificomponent_id;
mdns::MDNSComponent *mdns_mdnscomponent_id;
web_server::WebServerOTAComponent *web_server_webserverotacomponent_id;
esphome::ESPHomeOTAComponent *ota_esphome;
safe_mode::SafeModeComponent *safe_mode_safemodecomponent_id;
api::APIServer *api_apiserver_id;
using namespace api;
preferences::IntervalSyncer *preferences_intervalsyncer_id;
internal_temperature::InternalTemperatureSensor *internal_temperature_internaltemperaturesensor_id;
wifi_signal::WiFiSignalSensor *wifi_signal_wifisignalsensor_id;
uptime::UptimeSecondsSensor *uptime_uptimesecondssensor_id;
wifi_info::IPAddressWiFiInfo *wifi_info_ipaddresswifiinfo_id;
restart::RestartButton *restart_restartbutton_id;
factory_reset::FactoryResetButton *factory_reset_button;
esp32::ESP32InternalGPIOPin *esp32_esp32internalgpiopin_id;
remote_transmitter::RemoteTransmitterComponent *ir_transmitter_id;
esp32::ESP32InternalGPIOPin *esp32_esp32internalgpiopin_id_2;
remote_receiver::RemoteReceiverComponent *ir_receiver_id;
remote_base::RawDumper *remote_base_rawdumper_id;
toshiba_ac::ToshibaAcClimate *my_toshiba_ac;
// ========== AUTO GENERATED INCLUDE BLOCK END ==========="

void setup() {
  // ========== AUTO GENERATED CODE BEGIN ===========
  App.reserve_text_sensor(1);
  App.reserve_sensor(3);
  App.reserve_climate(1);
  App.reserve_button(2);
  // network:
  //   enable_ipv6: false
  //   min_ipv6_addr_count: 0
  // esphome:
  //   project:
  //     name: solarbox.toshiba
  //     version: 2026.3.4
  //   min_version: 2025.7.5
  //   name: toshiba
  //   name_add_mac_suffix: false
  //   build_path: ../build/dev_toshiba
  //   friendly_name: ''
  //   platformio_options: {}
  //   includes: []
  //   libraries: []
  //   debug_scheduler: false
  //   areas: []
  //   devices: []
  App.pre_setup("toshiba", "", "", __DATE__ ", " __TIME__, false);
  App.reserve_components(19);
  // sensor:
  // text_sensor:
  // button:
  // climate:
  // binary_sensor:
  // logger:
  //   level: DEBUG
  //   id: logger_logger_id
  //   baud_rate: 115200
  //   tx_buffer_size: 512
  //   deassert_rts_dtr: false
  //   task_log_buffer_size: 768
  //   hardware_uart: USB_SERIAL_JTAG
  //   logs: {}
  logger_logger_id = new logger::Logger(115200, 512);
  logger_logger_id->create_pthread_key();
  logger_logger_id->init_log_buffer(768);
  logger_logger_id->set_log_level(ESPHOME_LOG_LEVEL_DEBUG);
  logger_logger_id->set_uart_selection(logger::UART_SELECTION_USB_SERIAL_JTAG);
  logger_logger_id->pre_setup();
  logger_logger_id->set_component_source("logger");
  App.register_component(logger_logger_id);
  // web_server_base:
  //   id: web_server_base_webserverbase_id
  web_server_base_webserverbase_id = new web_server_base::WebServerBase();
  web_server_base_webserverbase_id->set_component_source("web_server_base");
  App.register_component(web_server_base_webserverbase_id);
  web_server_base::global_web_server_base = web_server_base_webserverbase_id;
  // captive_portal:
  //   id: captive_portal_captiveportal_id
  //   web_server_base_id: web_server_base_webserverbase_id
  captive_portal_captiveportal_id = new captive_portal::CaptivePortal(web_server_base_webserverbase_id);
  captive_portal_captiveportal_id->set_component_source("captive_portal");
  App.register_component(captive_portal_captiveportal_id);
  // wifi:
  //   networks:
  //     - ssid: xuantran
  //       password: 01227379368
  //       id: wifi_wifiap_id
  //       priority: 0.0
  //   ap:
  //     ssid: solarbox.uk
  //     id: wifi_wifiap_id_2
  //     ap_timeout: 1min
  //   id: wifi_wificomponent_id
  //   domain: .local
  //   reboot_timeout: 15min
  //   power_save_mode: LIGHT
  //   fast_connect: false
  //   enable_btm: false
  //   enable_rrm: false
  //   passive_scan: false
  //   enable_on_boot: true
  //   use_address: toshiba.local
  wifi_wificomponent_id = new wifi::WiFiComponent();
  wifi_wificomponent_id->set_use_address("toshiba.local");
  {
  wifi::WiFiAP wifi_wifiap_id = wifi::WiFiAP();
  wifi_wifiap_id.set_ssid("xuantran");
  wifi_wifiap_id.set_password("01227379368");
  wifi_wifiap_id.set_priority(0.0f);
  wifi_wificomponent_id->add_sta(wifi_wifiap_id);
  }
  {
  wifi::WiFiAP wifi_wifiap_id_2 = wifi::WiFiAP();
  wifi_wifiap_id_2.set_ssid("solarbox.uk");
  wifi_wificomponent_id->set_ap(wifi_wifiap_id_2);
  }
  wifi_wificomponent_id->set_ap_timeout(60000);
  wifi_wificomponent_id->set_reboot_timeout(900000);
  wifi_wificomponent_id->set_power_save_mode(wifi::WIFI_POWER_SAVE_LIGHT);
  wifi_wificomponent_id->set_fast_connect(false);
  wifi_wificomponent_id->set_passive_scan(false);
  wifi_wificomponent_id->set_enable_on_boot(true);
  wifi_wificomponent_id->set_component_source("wifi");
  App.register_component(wifi_wificomponent_id);
  // mdns:
  //   id: mdns_mdnscomponent_id
  //   disabled: false
  //   services: []
  mdns_mdnscomponent_id = new mdns::MDNSComponent();
  mdns_mdnscomponent_id->set_component_source("mdns");
  App.register_component(mdns_mdnscomponent_id);
  // ota:
  // ota.web_server:
  //   platform: web_server
  //   id: web_server_webserverotacomponent_id
  web_server_webserverotacomponent_id = new web_server::WebServerOTAComponent();
  // ota.esphome:
  //   platform: esphome
  //   id: ota_esphome
  //   password: what is that
  //   version: 2
  //   port: 3232
  ota_esphome = new esphome::ESPHomeOTAComponent();
  ota_esphome->set_port(3232);
  ota_esphome->set_auth_password("what is that");
  ota_esphome->set_component_source("esphome.ota");
  App.register_component(ota_esphome);
  // safe_mode:
  //   id: safe_mode_safemodecomponent_id
  //   boot_is_good_after: 1min
  //   disabled: false
  //   num_attempts: 10
  //   reboot_timeout: 5min
  safe_mode_safemodecomponent_id = new safe_mode::SafeModeComponent();
  safe_mode_safemodecomponent_id->set_component_source("safe_mode");
  App.register_component(safe_mode_safemodecomponent_id);
  if (safe_mode_safemodecomponent_id->should_enter_safe_mode(10, 300000, 60000)) return;
  web_server_webserverotacomponent_id->set_component_source("web_server.ota");
  App.register_component(web_server_webserverotacomponent_id);
  // api:
  //   id: api_apiserver_id
  //   port: 6053
  //   password: ''
  //   reboot_timeout: 15min
  //   batch_delay: 100ms
  //   custom_services: false
  api_apiserver_id = new api::APIServer();
  api_apiserver_id->set_component_source("api");
  App.register_component(api_apiserver_id);
  api_apiserver_id->set_port(6053);
  api_apiserver_id->set_reboot_timeout(900000);
  api_apiserver_id->set_batch_delay(100);
  // substitutions:
  //   dongletype: toshiba
  //   version: dev
  //   build_date: local
  // esp32:
  //   board: esp32-c3-devkitm-1
  //   framework:
  //     version: 5.3.2
  //     sdkconfig_options: {}
  //     advanced:
  //       compiler_optimization: SIZE
  //       enable_lwip_assert: true
  //       ignore_efuse_custom_mac: false
  //       enable_lwip_mdns_queries: true
  //       enable_lwip_bridge_interface: false
  //     components: []
  //     platform_version: https:github.com/pioarduino/platform-espressif32/releases/download/53.03.13/platform-espressif32.zip
  //     source: pioarduino/framework-espidf@https:github.com/pioarduino/esp-idf/releases/download/v5.3.2/esp-idf-v5.3.2.zip
  //     type: esp-idf
  //   flash_size: 4MB
  //   variant: ESP32C3
  //   cpu_frequency: 160MHZ
  // preferences:
  //   id: preferences_intervalsyncer_id
  //   flash_write_interval: 60s
  preferences_intervalsyncer_id = new preferences::IntervalSyncer();
  preferences_intervalsyncer_id->set_write_interval(60000);
  preferences_intervalsyncer_id->set_component_source("preferences");
  App.register_component(preferences_intervalsyncer_id);
  // external_components:
  //   - source:
  //       path: components
  //       type: local
  //     components:
  //       - toshiba_ac
  //     refresh: 1d
  // sensor.internal_temperature:
  //   platform: internal_temperature
  //   name: Internal Temperature
  //   disabled_by_default: false
  //   force_update: false
  //   id: internal_temperature_internaltemperaturesensor_id
  //   unit_of_measurement: °C
  //   accuracy_decimals: 1
  //   device_class: temperature
  //   state_class: measurement
  //   entity_category: diagnostic
  //   update_interval: 60s
  internal_temperature_internaltemperaturesensor_id = new internal_temperature::InternalTemperatureSensor();
  App.register_sensor(internal_temperature_internaltemperaturesensor_id);
  internal_temperature_internaltemperaturesensor_id->set_name("Internal Temperature");
  internal_temperature_internaltemperaturesensor_id->set_object_id("internal_temperature");
  internal_temperature_internaltemperaturesensor_id->set_disabled_by_default(false);
  internal_temperature_internaltemperaturesensor_id->set_entity_category(::ENTITY_CATEGORY_DIAGNOSTIC);
  internal_temperature_internaltemperaturesensor_id->set_device_class("temperature");
  internal_temperature_internaltemperaturesensor_id->set_state_class(sensor::STATE_CLASS_MEASUREMENT);
  internal_temperature_internaltemperaturesensor_id->set_unit_of_measurement("\302\260C");
  internal_temperature_internaltemperaturesensor_id->set_accuracy_decimals(1);
  internal_temperature_internaltemperaturesensor_id->set_force_update(false);
  internal_temperature_internaltemperaturesensor_id->set_update_interval(60000);
  internal_temperature_internaltemperaturesensor_id->set_component_source("internal_temperature.sensor");
  App.register_component(internal_temperature_internaltemperaturesensor_id);
  // sensor.wifi_signal:
  //   platform: wifi_signal
  //   name: WiFi Signal Strength
  //   disabled_by_default: false
  //   force_update: false
  //   id: wifi_signal_wifisignalsensor_id
  //   unit_of_measurement: dBm
  //   accuracy_decimals: 0
  //   device_class: signal_strength
  //   state_class: measurement
  //   entity_category: diagnostic
  //   update_interval: 60s
  wifi_signal_wifisignalsensor_id = new wifi_signal::WiFiSignalSensor();
  App.register_sensor(wifi_signal_wifisignalsensor_id);
  wifi_signal_wifisignalsensor_id->set_name("WiFi Signal Strength");
  wifi_signal_wifisignalsensor_id->set_object_id("wifi_signal_strength");
  wifi_signal_wifisignalsensor_id->set_disabled_by_default(false);
  wifi_signal_wifisignalsensor_id->set_entity_category(::ENTITY_CATEGORY_DIAGNOSTIC);
  wifi_signal_wifisignalsensor_id->set_device_class("signal_strength");
  wifi_signal_wifisignalsensor_id->set_state_class(sensor::STATE_CLASS_MEASUREMENT);
  wifi_signal_wifisignalsensor_id->set_unit_of_measurement("dBm");
  wifi_signal_wifisignalsensor_id->set_accuracy_decimals(0);
  wifi_signal_wifisignalsensor_id->set_force_update(false);
  wifi_signal_wifisignalsensor_id->set_update_interval(60000);
  wifi_signal_wifisignalsensor_id->set_component_source("wifi_signal.sensor");
  App.register_component(wifi_signal_wifisignalsensor_id);
  // sensor.uptime:
  //   platform: uptime
  //   name: Uptime
  //   disabled_by_default: false
  //   force_update: false
  //   id: uptime_uptimesecondssensor_id
  //   unit_of_measurement: s
  //   icon: mdi:timer-outline
  //   accuracy_decimals: 0
  //   device_class: duration
  //   state_class: total_increasing
  //   entity_category: diagnostic
  //   update_interval: 60s
  //   type: seconds
  uptime_uptimesecondssensor_id = new uptime::UptimeSecondsSensor();
  App.register_sensor(uptime_uptimesecondssensor_id);
  uptime_uptimesecondssensor_id->set_name("Uptime");
  uptime_uptimesecondssensor_id->set_object_id("uptime");
  uptime_uptimesecondssensor_id->set_disabled_by_default(false);
  uptime_uptimesecondssensor_id->set_icon("mdi:timer-outline");
  uptime_uptimesecondssensor_id->set_entity_category(::ENTITY_CATEGORY_DIAGNOSTIC);
  uptime_uptimesecondssensor_id->set_device_class("duration");
  uptime_uptimesecondssensor_id->set_state_class(sensor::STATE_CLASS_TOTAL_INCREASING);
  uptime_uptimesecondssensor_id->set_unit_of_measurement("s");
  uptime_uptimesecondssensor_id->set_accuracy_decimals(0);
  uptime_uptimesecondssensor_id->set_force_update(false);
  uptime_uptimesecondssensor_id->set_update_interval(60000);
  uptime_uptimesecondssensor_id->set_component_source("uptime.sensor");
  App.register_component(uptime_uptimesecondssensor_id);
  // text_sensor.wifi_info:
  //   platform: wifi_info
  //   ip_address:
  //     name: IP Address
  //     disabled_by_default: false
  //     id: wifi_info_ipaddresswifiinfo_id
  //     entity_category: diagnostic
  //     update_interval: 1s
  wifi_info_ipaddresswifiinfo_id = new wifi_info::IPAddressWiFiInfo();
  App.register_text_sensor(wifi_info_ipaddresswifiinfo_id);
  wifi_info_ipaddresswifiinfo_id->set_name("IP Address");
  wifi_info_ipaddresswifiinfo_id->set_object_id("ip_address");
  wifi_info_ipaddresswifiinfo_id->set_disabled_by_default(false);
  wifi_info_ipaddresswifiinfo_id->set_entity_category(::ENTITY_CATEGORY_DIAGNOSTIC);
  wifi_info_ipaddresswifiinfo_id->set_update_interval(1000);
  wifi_info_ipaddresswifiinfo_id->set_component_source("wifi_info.text_sensor");
  App.register_component(wifi_info_ipaddresswifiinfo_id);
  // button.restart:
  //   platform: restart
  //   name: Khởi động lại
  //   disabled_by_default: false
  //   id: restart_restartbutton_id
  //   icon: mdi:restart
  //   entity_category: config
  //   device_class: restart
  restart_restartbutton_id = new restart::RestartButton();
  restart_restartbutton_id->set_component_source("restart.button");
  App.register_component(restart_restartbutton_id);
  App.register_button(restart_restartbutton_id);
  restart_restartbutton_id->set_name("Kh\341\273\237i \304\221\341\273\231ng l\341\272\241i");
  restart_restartbutton_id->set_object_id("kh_i___ng_l_i");
  restart_restartbutton_id->set_disabled_by_default(false);
  restart_restartbutton_id->set_icon("mdi:restart");
  restart_restartbutton_id->set_entity_category(::ENTITY_CATEGORY_CONFIG);
  restart_restartbutton_id->set_device_class("restart");
  // button.factory_reset:
  //   platform: factory_reset
  //   name: Xóa dữ liệu dongle
  //   id: factory_reset_button
  //   disabled_by_default: false
  //   icon: mdi:restart-alert
  //   entity_category: config
  //   device_class: restart
  factory_reset_button = new factory_reset::FactoryResetButton();
  factory_reset_button->set_component_source("factory_reset.button");
  App.register_component(factory_reset_button);
  App.register_button(factory_reset_button);
  factory_reset_button->set_name("X\303\263a d\341\273\257 li\341\273\207u dongle");
  factory_reset_button->set_object_id("x_a_d__li_u_dongle");
  factory_reset_button->set_disabled_by_default(false);
  factory_reset_button->set_icon("mdi:restart-alert");
  factory_reset_button->set_entity_category(::ENTITY_CATEGORY_CONFIG);
  factory_reset_button->set_device_class("restart");
  // remote_transmitter:
  //   pin:
  //     number: 20
  //     mode:
  //       output: true
  //       open_drain: true
  //       input: false
  //       pullup: false
  //       pulldown: false
  //     inverted: true
  //     id: esp32_esp32internalgpiopin_id
  //     ignore_pin_validation_error: false
  //     ignore_strapping_warning: false
  //     drive_strength: 20.0
  //   carrier_duty_percent: 50
  //   id: ir_transmitter_id
  //   rmt_symbols: 48
  esp32_esp32internalgpiopin_id = new esp32::ESP32InternalGPIOPin();
  esp32_esp32internalgpiopin_id->set_pin(::GPIO_NUM_20);
  esp32_esp32internalgpiopin_id->set_inverted(true);
  esp32_esp32internalgpiopin_id->set_drive_strength(::GPIO_DRIVE_CAP_2);
  esp32_esp32internalgpiopin_id->set_flags((gpio::Flags::FLAG_OUTPUT | gpio::Flags::FLAG_OPEN_DRAIN));
  ir_transmitter_id = new remote_transmitter::RemoteTransmitterComponent(esp32_esp32internalgpiopin_id);
  ir_transmitter_id->set_rmt_symbols(48);
  ir_transmitter_id->set_eot_level(true);
  ir_transmitter_id->set_component_source("remote_transmitter");
  App.register_component(ir_transmitter_id);
  ir_transmitter_id->set_carrier_duty_percent(50);
  // remote_receiver:
  //   pin:
  //     number: 21
  //     mode:
  //       input: true
  //       pullup: true
  //       output: false
  //       open_drain: false
  //       pulldown: false
  //     inverted: true
  //     id: esp32_esp32internalgpiopin_id_2
  //     ignore_pin_validation_error: false
  //     ignore_strapping_warning: false
  //     drive_strength: 20.0
  //   dump:
  //     - raw: {}
  //       type_id: remote_base_rawdumper_id
  //   tolerance:
  //     value: 50
  //     type: percentage
  //   id: ir_receiver_id
  //   buffer_size: 10000
  //   filter: 50us
  //   idle: 10ms
  //   rmt_symbols: 96
  //   receive_symbols: 192
  esp32_esp32internalgpiopin_id_2 = new esp32::ESP32InternalGPIOPin();
  esp32_esp32internalgpiopin_id_2->set_pin(::GPIO_NUM_21);
  esp32_esp32internalgpiopin_id_2->set_inverted(true);
  esp32_esp32internalgpiopin_id_2->set_drive_strength(::GPIO_DRIVE_CAP_2);
  esp32_esp32internalgpiopin_id_2->set_flags((gpio::Flags::FLAG_INPUT | gpio::Flags::FLAG_PULLUP));
  ir_receiver_id = new remote_receiver::RemoteReceiverComponent(esp32_esp32internalgpiopin_id_2);
  ir_receiver_id->set_rmt_symbols(96);
  ir_receiver_id->set_receive_symbols(192);
  remote_base_rawdumper_id = new remote_base::RawDumper();
  // climate.toshiba_ac:
  //   platform: toshiba_ac
  //   name: Toshiba AC
  //   id: my_toshiba_ac
  //   receiver_id: ir_receiver_id
  //   transmitter_id: ir_transmitter_id
  //   disabled_by_default: false
  //   visual: {}
  my_toshiba_ac = new toshiba_ac::ToshibaAcClimate();
  my_toshiba_ac->set_component_source("toshiba_ac.climate");
  App.register_component(my_toshiba_ac);
  App.register_climate(my_toshiba_ac);
  my_toshiba_ac->set_name("Toshiba AC");
  my_toshiba_ac->set_object_id("toshiba_ac");
  my_toshiba_ac->set_disabled_by_default(false);
  ir_receiver_id->register_listener(my_toshiba_ac);
  my_toshiba_ac->set_transmitter(ir_transmitter_id);
  // socket:
  //   implementation: bsd_sockets
  // md5:
  // web_server_idf:
  //   {}
  ir_receiver_id->register_dumper(remote_base_rawdumper_id);
  ir_receiver_id->set_component_source("remote_receiver");
  App.register_component(ir_receiver_id);
  ir_receiver_id->set_tolerance(50, remote_base::TOLERANCE_MODE_PERCENTAGE);
  ir_receiver_id->set_buffer_size(10000);
  ir_receiver_id->set_filter_us(50);
  ir_receiver_id->set_idle_us(10000);
  // =========== AUTO GENERATED CODE END ============
  App.setup();
}

void loop() {
  App.loop();
}
