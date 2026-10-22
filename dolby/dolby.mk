# SPDX-FileCopyrightText: 2026 ikwfahmi
# SPDX-License-Identifier: Apache-2.0

$(call inherit-product, vendor/asus/dolby/dolby-vendor.mk)

PRODUCT_PACKAGES += LunarisDolby

PRODUCT_VENDOR_PROPERTIES += \
    ro.vendor.audio.dolby.dax.support=true \
    ro.vendor.audio.dolby.surround.enable=true \
    vendor.audio.dolby.ds2.enabled=false \
    vendor.audio.dolby.ds2.hardbypass=false
