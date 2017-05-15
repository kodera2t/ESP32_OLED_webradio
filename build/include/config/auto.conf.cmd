deps_config := \
	/Users/kodera2t/esp/esp-idf/components/aws_iot/Kconfig \
	/Users/kodera2t/esp/esp-idf/components/bt/Kconfig \
	/Users/kodera2t/esp/esp-idf/components/esp32/Kconfig \
	/Users/kodera2t/esp/esp-idf/components/ethernet/Kconfig \
	/Users/kodera2t/esp/esp-idf/components/fatfs/Kconfig \
	/Users/kodera2t/esp/esp-idf/components/freertos/Kconfig \
	/Users/kodera2t/esp/esp-idf/components/log/Kconfig \
	/Users/kodera2t/esp/esp-idf/components/lwip/Kconfig \
	/Users/kodera2t/esp/esp-idf/components/mbedtls/Kconfig \
	/Users/kodera2t/esp/esp-idf/components/openssl/Kconfig \
	/Users/kodera2t/esp/esp-idf/components/spi_flash/Kconfig \
	/Users/kodera2t/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/Users/kodera2t/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/Users/kodera2t/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/Users/kodera2t/esp/ESP32_OLED_MP3_Decoder/main/Kconfig.projbuild \
	/Users/kodera2t/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
