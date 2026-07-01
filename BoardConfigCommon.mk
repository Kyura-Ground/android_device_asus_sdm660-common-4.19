#
# Copyright (C) 2020 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

BOARD_VENDOR := asus
VENDOR_PATH  := device/asus/sdm660-common


# ============================================================
# Архитектура процессора
# ============================================================

# 64-bit primary (Kryo 260 Gold ≈ Cortex-A73)
TARGET_ARCH                := arm64
TARGET_ARCH_VARIANT        := armv8-a
TARGET_CPU_ABI             := arm64-v8a
TARGET_CPU_VARIANT         := generic
TARGET_CPU_VARIANT_RUNTIME := cortex-a73

# 32-bit secondary (Kryo 260 Silver ≈ Cortex-A73)
TARGET_2ND_ARCH                := arm
TARGET_2ND_ARCH_VARIANT        := armv8-a
TARGET_2ND_CPU_ABI             := armeabi-v7a
TARGET_2ND_CPU_VARIANT         := generic
TARGET_2ND_CPU_VARIANT_RUNTIME := cortex-a73


# ============================================================
# Аудио
# ============================================================

# Усилители
AUDIO_FEATURE_ENABLED_EXT_AMPLIFIER    := false
AUDIO_FEATURE_ENABLED_TFA98XX_AMPLIFIER := true

# ALSA / Sound Trigger
BOARD_USES_ALSA_AUDIO          := true
BOARD_SUPPORTS_SOUND_TRIGGER   := true
BOARD_SUPPORTS_OPENSOURCE_STHAL := true

# Вывод
AUDIO_USE_DEEP_AS_PRIMARY_OUTPUT := false

# Функции (не зависящие от AOSP-аудио)
AUDIO_FEATURE_ENABLED_ACDB_LICENSE        := true
AUDIO_FEATURE_ENABLED_ANC_HEADSET         := true
AUDIO_FEATURE_ENABLED_CUSTOMSTEREO        := true
AUDIO_FEATURE_ENABLED_DISPLAY_PORT        := true
AUDIO_FEATURE_ENABLED_DS2_DOLBY_DAP       := false
AUDIO_FEATURE_ENABLED_DYNAMIC_LOG         := false
AUDIO_FEATURE_ENABLED_FLUENCE             := true
AUDIO_FEATURE_ENABLED_GEF_SUPPORT         := true
AUDIO_FEATURE_ENABLED_HDMI_EDID           := true
AUDIO_FEATURE_ENABLED_HDMI_PASSTHROUGH    := true
AUDIO_FEATURE_ENABLED_HFP                 := true
AUDIO_FEATURE_ENABLED_HIFI_AUDIO          := true
AUDIO_FEATURE_ENABLED_INCALL_MUSIC        := true
# AUDIO_FEATURE_ENABLED_KEEP_ALIVE        := true   # отключено намеренно
AUDIO_FEATURE_ENABLED_KPI_OPTIMIZE        := true
AUDIO_FEATURE_ENABLED_MULTI_VOICE_SESSIONS := true
AUDIO_FEATURE_ENABLED_NT_PAUSE_TIMEOUT    := true
AUDIO_FEATURE_ENABLED_RAS                 := true
AUDIO_FEATURE_ENABLED_SND_MONITOR         := true
AUDIO_FEATURE_ENABLED_SOURCE_TRACKING     := true
AUDIO_FEATURE_ENABLED_SPKR_PROTECTION     := true
AUDIO_FEATURE_ENABLED_VBAT_MONITOR        := true

# Функции (только при использовании Qualcomm audio policy)
ifneq ($(TARGET_USES_AOSP_FOR_AUDIO), true)
USE_CUSTOM_AUDIO_POLICY                        := 1
AUDIO_FEATURE_QSSI_COMPLIANCE                  := true
AUDIO_FEATURE_ENABLED_AHAL_EXT                 := false
AUDIO_FEATURE_ENABLED_A2DP_OFFLOAD             := true
AUDIO_FEATURE_ENABLED_AAC_ADTS_OFFLOAD         := true
AUDIO_FEATURE_ENABLED_ALAC_OFFLOAD             := true
AUDIO_FEATURE_ENABLED_APE_OFFLOAD              := true
AUDIO_FEATURE_ENABLED_AUDIOSPHERE              := true
AUDIO_FEATURE_ENABLED_COMPRESS_CAPTURE         := false
AUDIO_FEATURE_ENABLED_COMPRESS_VOIP            := false
AUDIO_FEATURE_ENABLED_DEV_ARBI                 := false
AUDIO_FEATURE_ENABLED_DTS_EAGLE                := false
AUDIO_FEATURE_ENABLED_EXTENDED_COMPRESS_FORMAT := true
AUDIO_FEATURE_ENABLED_EXTN_FLAC_DECODER        := true
AUDIO_FEATURE_ENABLED_EXTN_FORMATS             := true
AUDIO_FEATURE_ENABLED_EXTN_RESAMPLER           := true
AUDIO_FEATURE_ENABLED_FLAC_OFFLOAD             := true
AUDIO_FEATURE_ENABLED_FM_POWER_OPT             := false
AUDIO_FEATURE_ENABLED_HDMI_SPK                 := true
AUDIO_FEATURE_ENABLED_HW_ACCELERATED_EFFECTS   := false
AUDIO_FEATURE_ENABLED_PCM_OFFLOAD              := true
AUDIO_FEATURE_ENABLED_PCM_OFFLOAD_24           := true
AUDIO_FEATURE_ENABLED_PROXY_DEVICE             := true
AUDIO_FEATURE_ENABLED_SSR                      := true
AUDIO_FEATURE_ENABLED_USB_TUNNEL               := true
AUDIO_FEATURE_ENABLED_VORBIS_OFFLOAD           := true
AUDIO_FEATURE_ENABLED_VOICE_PRINT              := false
AUDIO_FEATURE_ENABLED_3D_AUDIO                 := false
AUDIO_FEATURE_ENABLED_WMA_OFFLOAD              := true
BOARD_USES_SRS_TRUEMEDIA                       := false
DOLBY_ENABLE                                   := false
DTS_CODEC_M_                                   := false
USE_LEGACY_AUDIO_DAEMON                        := false
USE_LEGACY_AUDIO_MEASUREMENT                   := false
endif

USE_XML_AUDIO_POLICY_CONF := 1


# ============================================================
# Загрузчик
# ============================================================

TARGET_NO_BOOTLOADER := true
TARGET_NO_RECOVERY   := false   # A-only с отдельным recovery разделом


# ============================================================
# Камера
# ============================================================

# SDM660/636 camera HAL работает только в 32-bit режиме
BOARD_QTI_CAMERA_32BIT_ONLY := true

# Оптимизация памяти для 32-bit процессов камеры
MALLOC_SVELTE           := true
MALLOC_SVELTE_FOR_LIBC32 := true


# ============================================================
# Дисплей
# ============================================================

TARGET_SCREEN_DENSITY              := 440
TARGET_USES_GRALLOC1               := true
TARGET_USES_GRALLOC4               := true
TARGET_USES_HWC2                   := true
TARGET_USES_ION                    := true
TARGET_USES_QTI_MAPPER_2_0         := true
TARGET_USES_QTI_MAPPER_EXTENSIONS_1_1 := true


# ============================================================
# DRM / Widevine
# ============================================================

TARGET_ENABLE_MEDIADRM_64 := true


# ============================================================
# Файловая система
# ============================================================

TARGET_FS_CONFIG_GEN := $(VENDOR_PATH)/config.fs


# ============================================================
# FM-радио (Qualcomm Cherokee SoC)
# ============================================================

#BOARD_HAS_QCA_FM_SOC := cherokee
#BOARD_HAVE_QCOM_FM   := true

# ============================================================
# GPS
# ============================================================

BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE := default

# ============================================================
# HIDL / VINTF манифесты
# ============================================================

DEVICE_MANIFEST_FILE := $(VENDOR_PATH)/manifest.xml
DEVICE_MATRIX_FILE                        := $(VENDOR_PATH)/compatibility_matrix.xml
DEVICE_FRAMEWORK_COMPATIBILITY_MATRIX_FILE := \
    $(VENDOR_PATH)/framework_compatibility_matrix.xml

# ODM манифест для SKU с NFC
#ODM_MANIFEST_SKUS      += NFC
#ODM_MANIFEST_NFC_FILES := $(VENDOR_PATH)/manifest_nfc.xml


# ============================================================
# HWUI
# ============================================================

HWUI_COMPILE_FOR_PERF := true


# ============================================================
# Init
# ============================================================

$(call soong_config_set,libinit,vendor_init_lib,//$(VENDOR_PATH):libinit_sdm660)


# ============================================================
# Ядро
# ============================================================

BOARD_BOOT_HEADER_VERSION  := 1
BOARD_KERNEL_BASE          := 0x00000000
BOARD_KERNEL_IMAGE_NAME    := Image.gz-dtb
BOARD_KERNEL_PAGESIZE      := 4096

BOARD_KERNEL_CMDLINE := \
    androidboot.hardware=qcom \
    user_debug=31 \
    msm_rtb.filter=0x37 \
    ehci-hcd.park=3 \
    lpm_levels.sleep_disabled=1 \
    sched_enable_hmp=1 \
    sched_enable_power_aware=1 \
    service_locator.enable=1 \
    printk.devkmsg=on \
    usbcore.autosuspend=7
# BOARD_KERNEL_CMDLINE += androidboot.selinux=permissive   # только для отладки!

BOARD_MKBOOTIMG_ARGS += --header_version $(BOARD_BOOT_HEADER_VERSION)

TARGET_KERNEL_SOURCE     := kernel/asus/sdm660
TARGET_KERNEL_NO_GCC     := true

# ============================================================
# Lineage Health
# ============================================================

$(call soong_config_set,lineage_health,charging_control_charging_path,\
    /sys/class/power_supply/battery/charging_enabled)


# ============================================================
# LMKD
# ============================================================

TARGET_LMKD_STATS_LOG := true


# ============================================================
# Metadata раздел
# ============================================================

BOARD_USES_METADATA_PARTITION := true


# ============================================================
# Разделы (partition layout)
# ============================================================

# Размер блока = BOARD_KERNEL_PAGESIZE * 64
BOARD_FLASH_BLOCK_SIZE := 262144

BOARD_BOOTIMAGE_PARTITION_SIZE     := 67108864       #  64 MiB
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 67108864       #  64 MiB
BOARD_CACHEIMAGE_PARTITION_SIZE    := 367001600      # 350 MiB
BOARD_SYSTEMIMAGE_PARTITION_SIZE   := 4294967296     #   4 GiB
BOARD_VENDORIMAGE_PARTITION_SIZE   := 838860800      # 800 MiB
BOARD_USERDATAIMAGE_PARTITION_SIZE := 55490624512    #  ~52 GiB

BOARD_SYSTEMIMAGE_JOURNAL_SIZE     := 0

BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE  := ext4
BOARD_SYSTEMIMAGE_PARTITION_TYPE   := ext4
BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE := ext4

TARGET_USERIMAGES_USE_EXT4 := true
TARGET_USERIMAGES_USE_F2FS := true
TARGET_COPY_OUT_VENDOR     := vendor

# Симлинки в корне rootfs
BOARD_ROOT_EXTRA_SYMLINKS := \
    /mnt/vendor/persist:/persist

# Не используем A/B OTA
AB_OTA_UPDATER := false


# ============================================================
# Платформа
# ============================================================

TARGET_BOARD_PLATFORM  := sdm660
TARGET_ENFORCES_QSSI   := true
BOARD_USES_QCOM_HARDWARE := true


# ============================================================
# Свойства системы
# ============================================================

TARGET_PRODUCT_PROP += $(VENDOR_PATH)/properties/product.prop
TARGET_SYSTEM_PROP  += $(VENDOR_PATH)/properties/system.prop
TARGET_VENDOR_PROP  += $(VENDOR_PATH)/properties/vendor.prop


# ============================================================
# Recovery
# ============================================================

TARGET_RECOVERY_FSTAB         := $(VENDOR_PATH)/rootdir/etc/fstab.qcom
TARGET_RECOVERY_PIXEL_FORMAT  := RGBX_8888
TARGET_RECOVERY_UPDATER_LIBS  := librecovery_updater_asus
TARGET_RELEASETOOLS_EXTENSIONS := $(VENDOR_PATH)


# ============================================================
# RIL
# ============================================================

ENABLE_VENDOR_RIL_SERVICE := true


# ============================================================
# SELinux
# ============================================================
include device/lineage/sepolicy/libion/sepolicy.mk
include device/qcom/sepolicy-legacy-um/SEPolicy.mk

BOARD_VENDOR_SEPOLICY_DIRS += \
    $(VENDOR_PATH)/sepolicy/vendor

SYSTEM_EXT_PUBLIC_SEPOLICY_DIRS += \
    $(VENDOR_PATH)/sepolicy/public

SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS += \
    $(VENDOR_PATH)/sepolicy/private


# ============================================================
# Shipping API level (GRF / Vendor Freeze)
# ============================================================

# Устройство изначально поставлялось с Android 8.1 (API 27)
# Vendor freeze зафиксирован на уровне Android 14 (API 34)
BOARD_SHIPPING_API_LEVEL := 34


# ============================================================
# Soong namespaces
# ============================================================

PRODUCT_SOONG_NAMESPACES += $(VENDOR_PATH)


# ============================================================
# USB
# ============================================================

TARGET_QTI_USB_SUPPORTS_AUDIO_ACCESSORY := true


# ============================================================
# Vendor Security Patch
# ============================================================

VENDOR_SECURITY_PATCH := 2025-01-05


# ============================================================
# VNDK
# ============================================================

BOARD_VNDK_VERSION := current


# ============================================================
# Wi-Fi
# ============================================================

BOARD_HAS_QCOM_WLAN    := true
BOARD_WLAN_DEVICE      := qcwcn
WIFI_DRIVER_DEFAULT    := qca_cld3

# Драйвер управления состоянием
WIFI_DRIVER_STATE_CTRL_PARAM := "/dev/wlan"
WIFI_DRIVER_STATE_ON         := "ON"
WIFI_DRIVER_STATE_OFF        := "OFF"

# WPA Supplicant
BOARD_WPA_SUPPLICANT_DRIVER      := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
WPA_SUPPLICANT_VERSION           := VER_0_8_X

# HostAPD (точка доступа)
BOARD_HOSTAPD_DRIVER      := NL80211
BOARD_HOSTAPD_PRIVATE_LIB := lib_driver_cmd_$(BOARD_WLAN_DEVICE)

# HIDL / двойной интерфейс
WIFI_HIDL_FEATURE_DUAL_INTERFACE           := true
WIFI_HIDL_UNIFIED_SUPPLICANT_SERVICE_RC_ENTRY := true

# Стандарты 802.11
CONFIG_ACS        := true
CONFIG_IEEE80211AC := true


# ============================================================
# Vendor blobs (должен быть последним)
# ============================================================

include vendor/asus/sdm660-common/BoardConfigVendor.mk
