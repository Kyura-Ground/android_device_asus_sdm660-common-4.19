#
# Copyright (C) 2020 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

# ============================================================
# Наследование vendor/common
# ============================================================

# Proprietary vendor blobs
$(call inherit-product, vendor/asus/sdm660-common/sdm660-common-vendor.mk)

# Non-A/B partition scheme
$(call inherit-product, $(SRC_TARGET_DIR)/product/non_ab_device.mk)

# Qualcomm common definitions
$(call inherit-product, hardware/qcom-caf/common/common.mk)

# Enable project quotas and casefolding for emulated storage without sdcardfs
$(call inherit-product, $(SRC_TARGET_DIR)/product/emulated_storage.mk)

# ============================================================
# Глобальные параметры продукта
# ============================================================

# Shipping API level (устройство поставлялось с Android 8.1)
PRODUCT_SHIPPING_API_LEVEL := 27

# Характеристики устройства (нет слота для SD-карты)
PRODUCT_CHARACTERISTICS := nosdcard

# Плотность экрана
PRODUCT_AAPT_CONFIG      := normal
PRODUCT_AAPT_PREF_CONFIG ?= xxhdpi

# Половинное разрешение загрузочной анимации
TARGET_BOOTANIMATION_HALF_RES := true


# ============================================================
# Ядро
# ============================================================

TARGET_KERNEL_VERSION ?= 4.19

# APEX — не сжимаем (совместимость с older recovery)
PRODUCT_COMPRESSED_APEX := false

# UFFD GC включён для ядра 4.19+
PRODUCT_ENABLE_UFFD_GC := true

# Не проверяем требования VINTF к ядру при OTA
PRODUCT_OTA_ENFORCE_VINTF_KERNEL_REQUIREMENTS := false


# ============================================================
# ART / DEX оптимизация
# ============================================================

PRODUCT_ART_TARGET_INCLUDE_DEBUG_BUILD    := false
PRODUCT_DEX_PREOPT_DEFAULT_COMPILER_FILTER := everything
USE_DEX2OAT_DEBUG                          := false

PRODUCT_DEXPREOPT_SPEED_APPS += \
    SystemUI


# ============================================================
# AID / fs конфигурация
# ============================================================

PRODUCT_PACKAGES += \
    fs_config_files


# ============================================================
# Аудио — HAL и библиотеки
# ============================================================

PRODUCT_PACKAGES += \
    android.hardware.audio@7.1-impl:32 \
    android.hardware.audio.effect@7.0-impl:32 \
    android.hardware.audio.service \
    android.hardware.soundtrigger@2.3-impl:32

PRODUCT_PACKAGES += \
    audio.bluetooth.default \
    audio.primary.sdm660:32 \
    audio.r_submix.default \
    audio.usb.default \
    sound_trigger.primary.sdm660:32 \
    libaudio-resampler \
    libqcompostprocbundle \
    libqcomvisualizer \
    libqcomvoiceprocessing \
    libvolumelistener \
    tinymix

PRODUCT_PACKAGES += \
    libhdmiedid \
    libhfp \
    libsndmonitor \
    libspkrprot \
    libssrec

# Аудио — конфигурационные файлы устройства
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/audio/audio_configs.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_configs.xml \
    $(LOCAL_PATH)/configs/audio/audio_effects.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_effects.xml \
    $(LOCAL_PATH)/configs/audio/audio_output_policy.conf:$(TARGET_COPY_OUT_VENDOR)/etc/audio_output_policy.conf \
    $(LOCAL_PATH)/configs/audio/audio_platform_info_intcodec.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_platform_info_intcodec.xml \
    $(LOCAL_PATH)/configs/audio/audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration.xml \
    $(LOCAL_PATH)/configs/audio/audio_tuning_mixer.txt:$(TARGET_COPY_OUT_VENDOR)/etc/audio_tuning_mixer.txt \
    $(LOCAL_PATH)/configs/audio/graphite_ipc_platform_info.xml:$(TARGET_COPY_OUT_VENDOR)/etc/graphite_ipc_platform_info.xml \
    $(LOCAL_PATH)/configs/audio/listen_platform_info.xml:$(TARGET_COPY_OUT_VENDOR)/etc/listen_platform_info.xml \
    $(LOCAL_PATH)/configs/audio/mixer_paths.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths.xml \
    $(LOCAL_PATH)/configs/audio/sound_trigger_mixer_paths.xml:$(TARGET_COPY_OUT_VENDOR)/etc/sound_trigger_mixer_paths.xml \
    $(LOCAL_PATH)/configs/audio/sound_trigger_platform_info.xml:$(TARGET_COPY_OUT_VENDOR)/etc/sound_trigger_platform_info.xml

# Аудио — политики из AOSP frameworks
PRODUCT_COPY_FILES += \
    frameworks/av/services/audiopolicy/config/a2dp_in_audio_policy_configuration_7_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/a2dp_in_audio_policy_configuration_7_0.xml \
    frameworks/av/services/audiopolicy/config/audio_policy_volumes.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_volumes.xml \
    frameworks/av/services/audiopolicy/config/bluetooth_audio_policy_configuration_7_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/bluetooth_audio_policy_configuration_7_0.xml \
    frameworks/av/services/audiopolicy/config/default_volume_tables.xml:$(TARGET_COPY_OUT_VENDOR)/etc/default_volume_tables.xml \
    frameworks/av/services/audiopolicy/config/r_submix_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/r_submix_audio_policy_configuration.xml


# ============================================================
# ANT+
# ============================================================

PRODUCT_PACKAGES += \
    com.dsi.ant@1.0.vendor


# ============================================================
# Bluetooth
# ============================================================

PRODUCT_PACKAGES += \
    android.hardware.bluetooth@1.1 \
    android.hardware.bluetooth@1.1.vendor \
    android.hardware.bluetooth.audio-impl \
    vendor.qti.hardware.bluetooth_audio@2.1.vendor \
    vendor.qti.hardware.btconfigstore@1.0.vendor \
    vendor.qti.hardware.btconfigstore@2.0.vendor


# ============================================================
# Камера
# ============================================================

PRODUCT_PACKAGES += \
    android.hardware.camera.provider-service_32.lineage \
    vendor.qti.hardware.camera.device@1.0.vendor \
    camera.sdm660:32 \
    Aperture \
    libgui_vendor \
    liblz4.vendor \
    libpng.vendor:32 \
    libutilscallstack.vendor \
    libxml2


# ============================================================
# Cgroup и task profiles
# ============================================================

PRODUCT_COPY_FILES += \
    device/asus/sdm660-common/configs/cgroups.json:$(TARGET_COPY_OUT_VENDOR)/etc/cgroups.json \
    device/asus/sdm660-common/configs/task_profiles.json:$(TARGET_COPY_OUT_VENDOR)/etc/task_profiles.json


# ============================================================
# Configstore (отключён — используется AIDL)
# ============================================================

PRODUCT_PACKAGES += \
    disable_configstore


# ============================================================
# DeviceAsWebCam
# ============================================================

#PRODUCT_PACKAGES += \
#    AsusDeviceAsWebcam


# ============================================================
# Дисплей — HAL и библиотеки
# ============================================================

PRODUCT_PACKAGES += \
    android.hardware.graphics.composer@2.1-service \
    android.hardware.graphics.mapper@3.0-impl-qti-display \
    android.hardware.graphics.mapper@4.0-impl-qti-display \
    android.frameworks.displayservice@1.0 \
    android.frameworks.displayservice@1.0_32 \
    android.frameworks.displayservice@1.0.vendor \
    gralloc.sdm660 \
    hwcomposer.qcom \
    libdisplayconfig \
    libtinyxml \
    vendor.display.config@1.0.vendor \
    vendor.display.config@2.0 \
    vendor.qti.hardware.display.allocator-service \
    vendor.qti.hardware.memtrack-service

# Дисплей — конфиг устройства
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/displayconfig/display_id_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/displayconfig/display_id_0.xml

# Дисплей — системные свойства GPU/EGL/Vulkan
PRODUCT_PROPERTY_OVERRIDES += \
    ro.hardware.egl=adreno \
    ro.hardware.vulkan=adreno \
    ro.opengles.version=196610 \
    debug.egl.hw=1 \
    debug.sf.hw=1 \
    debug.sf.latch_unsignaled=1 \
    debug.sf.disable_backpressure=1 \
    debug.hwui.renderer=skiagl \
    debug.renderengine.backend=skiaglthreaded \
    vendor.display.enable_default_color_mode=1 \
    vendor.display.disable_skip_validate=1 \
    vendor.gralloc.enable_fb_ubwc=1 \
    vendor.video.disable.ubwc=1

# Дисплей — свойства SurfaceFlinger
PRODUCT_PROPERTY_OVERRIDES += \
    ro.surface_flinger.force_hwc_copy_for_virtual_displays=true \
    ro.surface_flinger.max_frame_buffer_acquired_buffers=3 \
    ro.surface_flinger.max_virtual_display_dimension=4096 \
    ro.surface_flinger.has_wide_color_display=false \
    ro.surface_flinger.has_HDR_display=false \
    ro.surface_flinger.use_color_management=false \
    ro.surface_flinger.protected_contents=true

# ============================================================
# DRM
# ============================================================

PRODUCT_PACKAGES += \
    android.hardware.drm@1.4.vendor \
    android.hardware.drm-service.clearkey \
    libcrypto \
    libhidlmemory.vendor:64 \
    libunwindstack.vendor

# ============================================================
# Отпечаток пальца
# ============================================================

PRODUCT_PACKAGES += \
    android.hardware.biometrics.fingerprint-service.asus


# ============================================================
# FM-радио
# ============================================================

#PRODUCT_PACKAGES += \
#    android.hardware.broadcastradio@1.0-impl \
#    FM2 \
#    libqcomfm_jni \
#    qcom.fmradio \
#    qcom.fmradio.xml


# ============================================================
# Framework detection (Qualcomm VND)
# ============================================================

PRODUCT_PACKAGES += \
    libqti_vndfwk_detect \
    libqti_vndfwk_detect.vendor \
    libvndfwk_detect_jni.qti \
    libvndfwk_detect_jni.qti.vendor


# ============================================================
# Gatekeeper
# ============================================================

PRODUCT_PACKAGES += \
    android.hardware.gatekeeper@1.0.vendor \
    libion.vendor


# ============================================================
# GPS / Геолокация
# ============================================================

PRODUCT_PACKAGES += \
    android.hardware.gnss@2.1-impl-qti \
    android.hardware.gnss@2.1-service-qti

PRODUCT_PACKAGES += \
    libbatching \
    libgeofencing \
    libgnss \
    libsensorndkbridge

PRODUCT_PACKAGES += \
    apdr.conf \
    flp.conf \
    gnss_antenna_info.conf \
    gps.conf \
    izat.conf \
    lowi.conf \
    sap.conf

PRODUCT_PACKAGES += \
    gnss@2.0-base.policy \
    gnss@2.0-xtra-daemon.policy

# ============================================================
# GMS — разрешения приложений
# ============================================================

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/privapp-permission/privapp-permissions-gms.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/permissions/privapp.permissions-gms.xml


# ============================================================
# Health
# ============================================================

PRODUCT_PACKAGES += \
    android.hardware.health@2.1.vendor \
    android.hardware.health-service.qti \
    android.hardware.health-service.qti_recovery


# ============================================================
# HIDL (базовые транспортные библиотеки)
# ============================================================

PRODUCT_PACKAGES += \
    android.hidl.base@1.0 \
    libhidltransport \
    libhidltransport.vendor \
    libhwbinder \
    libhwbinder.vendor


# ============================================================
# Input — раскладки клавиш
# ============================================================

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/keylayout/gpio-keys.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/gpio-keys.kl


# ============================================================
# IPACM (IP Accelerator Connection Manager)
# ============================================================

PRODUCT_PACKAGES += \
    ipacm \
    IPACM_cfg.xml


# ============================================================
# IPC router
# ============================================================

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/sec_config:$(TARGET_COPY_OUT_VENDOR)/etc/sec_config


# ============================================================
# Keymaster
# ============================================================

PRODUCT_PACKAGES += \
    android.hardware.keymaster@3.0.vendor


# ============================================================
# Подсветка и LED
# ============================================================

PRODUCT_PACKAGES += \
    android.hardware.light-service.asus_sdm660


# ============================================================
# Lineage Health
# ============================================================

PRODUCT_PACKAGES += \
    vendor.lineage.health-service.default

# ============================================================
# LiveDisplay native
# ============================================================
PRODUCT_PACKAGES += \
    vendor.lineage.livedisplay-service.sdm

$(call soong_config_set_bool,livedisplay_sdm,enable_dm,false)


# ============================================================
# Медиа — кодеки и профили
# ============================================================

# Конфигурация устройства
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/media/media_codecs.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml \
    $(LOCAL_PATH)/configs/media/media_codecs_performance.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml \
    $(LOCAL_PATH)/configs/media/media_profiles.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/media_profiles.xml \
    $(LOCAL_PATH)/configs/media/media_profiles.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_profiles_vendor.xml \
    $(LOCAL_PATH)/configs/media/media_profiles_V1_0.xml:$(TARGET_COPY_OUT_ODM)/etc/media_profiles_V1_0.xml

# ============================================================
# Сеть
# ============================================================

PRODUCT_PACKAGES += \
    android.system.net.netd@1.1.vendor \
    libnetutils.vendor


# ============================================================
# NFC
# ============================================================

# Разрешения (SKU с NFC модулем)
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.nfc.hce.xml:$(TARGET_COPY_OUT_ODM)/etc/permissions/sku_NFC/android.hardware.nfc.hce.xml \
    frameworks/native/data/etc/android.hardware.nfc.xml:$(TARGET_COPY_OUT_ODM)/etc/permissions/sku_NFC/android.hardware.nfc.xml \
    frameworks/native/data/etc/com.android.nfc_extras.xml:$(TARGET_COPY_OUT_ODM)/etc/permissions/sku_NFC/com.android.nfc_extras.xml \
    frameworks/native/data/etc/com.nxp.mifare.xml:$(TARGET_COPY_OUT_ODM)/etc/permissions/sku_NFC/com.nxp.mifare.xml

# Конфигурация NFC-чипа NXP
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/nfc/libnfc-nci.conf:$(TARGET_COPY_OUT_VENDOR)/etc/libnfc-nci.conf \
    $(LOCAL_PATH)/configs/nfc/libnfc-nxp.conf:$(TARGET_COPY_OUT_VENDOR)/etc/libnfc-nxp.conf

PRODUCT_PACKAGES += \
    android.hardware.nfc-service.nxp \
    com.android.nfc_extras \
    Tag


# ============================================================
# Neural Networks (NNAPI)
# ============================================================

PRODUCT_PACKAGES += \
    android.hardware.neuralnetworks@1.3.vendor


# ============================================================
# OEM Unlock
# ============================================================

PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    ro.oem_unlock_supported=1


# ============================================================
# OMX / медиа декодеры
# ============================================================

PRODUCT_PACKAGES += \
    libc2dcolorconvert \
    libmm-omxcore \
    libOmxCore \
    libOmxVdec \
    libOmxVenc \
    libstagefright_amrnb_common.vendor \
    libstagefright_enc_common \
    libstagefright_enc_common.vendor \
    libstagefright_softomx.vendor \
    libstagefright_softomx_plugin.vendor \
    libstagefrighthw


# ============================================================
# Overlays (RRO)
# ============================================================

DEVICE_PACKAGE_OVERLAYS += \
    $(LOCAL_PATH)/overlay \
    $(LOCAL_PATH)/overlay-lineage \
    $(LOCAL_PATH)/overlay-translates

PRODUCT_ENFORCE_RRO_TARGETS := *

PRODUCT_PACKAGES += \
    NcmTetheringOverlay

# ============================================================
# Разделы (partition mountpoints)
# ============================================================

PRODUCT_PACKAGES += \
    vendor_bt_firmware_mountpoint \
    vendor_dsp_mountpoint \
    vendor_firmware_mnt_mountpoint


# ============================================================
# Perf (TFLite / TextClassifier)
# ============================================================

PRODUCT_PACKAGES += \
    libtflite \
    libtextclassifier_hash


# ============================================================
# Разрешения (permissions XML)
# ============================================================

# Privapp разрешения QTI
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/privapp-permission/privapp-permissions-qti.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/permissions/privapp-permissions-qti.xml \
    $(LOCAL_PATH)/configs/privapp-permission/privapp-permissions-qti-system-ext.xml:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/permissions/privapp-permissions-qti-system-ext.xml

# Аппаратные возможности
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.audio.low_latency.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.audio.low_latency.xml \
    frameworks/native/data/etc/android.hardware.audio.pro.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.audio.pro.xml \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth_le.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.camera.full.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.full.xml \
    frameworks/native/data/etc/android.hardware.camera.raw.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.raw.xml \
    frameworks/native/data/etc/android.hardware.fingerprint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.fingerprint.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.opengles.aep.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.opengles.aep.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepcounter.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.stepcounter.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepdetector.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.stepdetector.xml \
    frameworks/native/data/etc/android.hardware.telephony.cdma.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.cdma.xml \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.telephony.ims.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.ims.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.vulkan.compute-0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.compute.xml \
    frameworks/native/data/etc/android.hardware.vulkan.level-0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.level.xml \
    frameworks/native/data/etc/android.hardware.vulkan.version-1_1.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.version.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.wifi.passpoint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.passpoint.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.software.app_widgets.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.app_widgets.xml \
    frameworks/native/data/etc/android.software.freeform_window_management.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.freeform_window_management.xml \
    frameworks/native/data/etc/android.software.ipsec_tunnel_migration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.ipsec_tunnel_migration.xml \
    frameworks/native/data/etc/android.software.ipsec_tunnels.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.ipsec_tunnels.xml \
    frameworks/native/data/etc/android.software.midi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.midi.xml \
    frameworks/native/data/etc/android.software.opengles.deqp.level-2021-03-01.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.opengles.deqp.level.xml \
    frameworks/native/data/etc/android.software.picture_in_picture.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.picture_in_picture.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/android.software.vulkan.deqp.level-2021-03-01.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.vulkan.deqp.level.xml \
    frameworks/native/data/etc/handheld_core_hardware.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/handheld_core_hardware.xml


# ============================================================
# Power (AIDL + libperfmgr)
# ============================================================

PRODUCT_PACKAGES += \
    android.hardware.power-service.asus-libperfmgr \
    libqti-perfd-client

# ============================================================
# Public Libraries
# ============================================================

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/public.libraries.txt:$(TARGET_COPY_OUT_VENDOR)/etc/public.libraries.txt

# ============================================================
# Remove packages
# ============================================================

PRODUCT_PACKAGES += \
    RemovePackages

# ============================================================
# Platform
# ============================================================
TARGET_BOARD_PLATFORM := sdm660

# ============================================================
# QCOM — whitelist и privapp разрешения
# ============================================================

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/privapp-permissions-qti.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/permissions/privapp-permissions-qti.xml \
    $(LOCAL_PATH)/configs/qti_whitelist.xml:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/sysconfig/qti_whitelist.xml


# ============================================================
# QMI (Qualcomm MSM Interface)
# ============================================================

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/qmi/qmi_fw.conf:$(TARGET_COPY_OUT_VENDOR)/etc/qmi_fw.conf

PRODUCT_PACKAGES += \
    libcrypto_utils.vendor \
    libjson \
    libjsoncpp.vendor \
    libqti_vndfwk_detect.vendor \
    libvndfwk_detect_jni.qti \
    libvndfwk_detect_jni.qti.vendor


# ============================================================
# QNS (Qualcomm Network Selection)
# ============================================================

PRODUCT_PACKAGES += \
    libstdc++_vendor


# ============================================================
# Ramdisk — скрипты инициализации
# ============================================================

PRODUCT_PACKAGES += \
    init.class_main.sh \
    init.qcom.post_boot.sh \
    init.qcom.sensors.sh \
    init.qcom.sh \
    init.qcom.usb.sh \
    init.zram.sh \
    init.zram_vm.sh

# Ramdisk — RC файлы
PRODUCT_PACKAGES += \
    fstab.qcom \
    init.asus.sdm660-common.rc \
    init.qcom.rc \
    init.qcom.usb.rc \
    init.recovery.qcom.rc \
    init.target.rc \
    init.uclamp.rc \
    init.zram.rc \
    ueventd.qcom.rc


# ============================================================
# RCS (Rich Communication Services)
# ============================================================

PRODUCT_PACKAGES += \
    com.android.ims.rcsmanager \
    PresencePolling \
    RcsService


# ============================================================
# Recovery
# ============================================================

PRODUCT_PACKAGES += \
    librecovery_updater_asus


# ============================================================
# RIL (Radio Interface Layer)
# ============================================================

PRODUCT_PACKAGES += \
    android.hardware.radio@1.5 \
    android.hardware.radio@1.5.vendor \
    android.hardware.radio.config@1.2 \
    android.hardware.radio.config@1.2.vendor \
    android.hardware.radio.deprecated@1.0 \
    android.hardware.radio.deprecated@1.0.vendor \
    android.hardware.secure_element@1.1 \
    android.hardware.secure_element@1.1.vendor \
    android.hardware.secure_element@1.2 \
    android.hardware.secure_element@1.2.vendor \
    libavservices_minijail.vendor \
    librmnetctl \
    libsqlite.vendor:64 \
    libsysutils.vendor


# ============================================================
# Seccomp — политики для медиа процессов
# ============================================================

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/seccomp/mediacodec-seccomp.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediacodec.policy \
    $(LOCAL_PATH)/configs/seccomp/mediaextractor-seccomp.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediaextractor.policy


# ============================================================
# Сенсоры
# ============================================================

PRODUCT_PACKAGES += \
    android.frameworks.sensorservice@1.0.vendor \
    android.hardware.sensors@1.0-impl \
    android.hardware.sensors@1.0-service \
    libpower.vendor

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/sensors/hals.conf:$(TARGET_COPY_OUT_VENDOR)/etc/sensors/hals.conf


# ============================================================
# Soong namespaces
# ============================================================

PRODUCT_SOONG_NAMESPACES += \
    hardware/google/interfaces \
    hardware/google/pixel \
    device/asus/sdm660-common/power-libperfmgr \
    hardware/qcom-caf/common/libqti-perfd-client \
    vendor/qcom/opensource/usb/etc

# ============================================================
# Телефония
# ============================================================

PRODUCT_PACKAGES += \
    extphonelib \
    extphonelib-product \
    extphonelib.xml \
    extphonelib_product.xml \
    ims-ext-common \
    ims_ext_common.xml \
    qti-telephony-hidl-wrapper \
    qti-telephony-hidl-wrapper-prd \
    qti-telephony-utils \
    qti_telephony_hidl_wrapper.xml \
    qti_telephony_hidl_wrapper_prd.xml \
    qti_telephony_utils.xml \
    telephony-ext

PRODUCT_BOOT_JARS += \
    telephony-ext


# ============================================================
# Thermal
# ============================================================

PRODUCT_PACKAGES += \
    android.hardware.thermal-service.asus_sdm660


# ============================================================
# Touch
# ============================================================

PRODUCT_PACKAGES += \
    vendor.lineage.touch-service.asus_sdm660


# ============================================================
# USB
# ============================================================

PRODUCT_PACKAGES += \
    android.hardware.usb@1.3-service.basic \
    android.hardware.usb.gadget-service.qti \
    usb_compositions.conf

PRODUCT_VENDOR_PROPERTIES += \
    vendor.usb.controller=a800000.dwc3


# ============================================================
# Вибромотор
# ============================================================

$(call inherit-product, vendor/qcom/opensource/vibrator/vibrator-vendor-product.mk)


# ============================================================
# Wi-Fi
# ============================================================

PRODUCT_PACKAGES += \
    android.hardware.wifi-service \
    android.hardware.wifi@1.6.vendor \
    android.hardware.wifi.hostapd@1.3.vendor \
    hostapd \
    hostapd_cli \
    libwifi-hal-qcom \
    libwpa_client \
    wificond \
    wpa_supplicant \
    wpa_supplicant.conf \
    WifiOverlay

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/wifi/p2p_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/p2p_supplicant_overlay.conf \
    $(LOCAL_PATH)/configs/wifi/wpa_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/wpa_supplicant_overlay.conf \
    $(LOCAL_PATH)/configs/wifi/WCNSS_qcom_cfg.ini:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/WCNSS_qcom_cfg.ini

# ============================================================
# Симлинк на конфигурацию Wi-Fi прошивки
# ============================================================
PRODUCT_PACKAGES += \
    firmware_WCNSS_qcom_cfg.ini_symlink

# ============================================================
# WiFi Display
# ============================================================
PRODUCT_PACKAGES += \
    android.media.audio.common.types-V2-cpp
