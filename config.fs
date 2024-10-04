[AID_VENDOR_QTI_DIAG]
value:2901

[AID_VENDOR_QDSS]
value:2902

[AID_VENDOR_RFS]
value:2903

[AID_VENDOR_RFS_SHARED]
value:2904

[AID_VENDOR_ADPL_ODL]
value:2905

[AID_VENDOR_QRTR]
value:2906

[AID_VENDOR_THERMAL]
value:2907

[AID_VENDOR_FASTRPC]
value:2908

[AID_VENDOR_QTR]
value:2909

[AID_VENDOR_SENSORS]
value:2910

# ============================================
# Bluetooth HAL
# ============================================
[vendor/bin/hw/android.hardware.bluetooth@1.0-service-qti]
mode: 0755
user: AID_BLUETOOTH
group: AID_BLUETOOTH
caps: BLOCK_SUSPEND NET_ADMIN

# ============================================
# Power & Platform Services
# ============================================
[vendor/bin/pm-service]
mode: 0755
user: AID_SYSTEM
group: AID_SYSTEM
caps: NET_BIND_SERVICE SYS_BOOT

[vendor/bin/pd-mapper]
mode: 0755
user: AID_SYSTEM
group: AID_SYSTEM
caps: NET_BIND_SERVICE

# ============================================
# IMS Services
# ============================================
[vendor/bin/imsdatadaemon]
mode: 0755
user: AID_RADIO
group: AID_RADIO
caps: NET_BIND_SERVICE

[vendor/bin/ims_rtp_daemon]
mode: 0755
user: AID_RADIO
group: AID_RADIO
caps: NET_BIND_SERVICE

[vendor/bin/imsrcsd]
mode: 0755
user: AID_RADIO
group: AID_RADIO
caps: NET_BIND_SERVICE BLOCK_SUSPEND WAKE_ALARM

[vendor/bin/imsdaemon]
mode: 0755
user: AID_RADIO
group: AID_RADIO
caps: NET_BIND_SERVICE BLOCK_SUSPEND WAKE_ALARM

# ============================================
# Connectivity Network Daemon
# ============================================
[vendor/bin/cnd]
mode: 0755
user: AID_SYSTEM
group: AID_SYSTEM
caps: NET_BIND_SERVICE BLOCK_SUSPEND NET_ADMIN

[vendor/bin/loc_launcher]
mode: 0755
user:  AID_GPS
group: AID_GPS
caps: SETUID SETGID

# ============================================
# Sensors
# ============================================
[vendor/bin/sensors.qti]
mode: 0755
user: AID_SYSTEM
group: AID_SYSTEM
caps: NET_BIND_SERVICE

# ============================================
# Directories
# ============================================
[dsp/]
mode: 0771
user: AID_MEDIA
group: AID_MEDIA
caps: 0

[firmware/]
mode: 0771
user: AID_SYSTEM
group: AID_SYSTEM
caps: 0

[vendor/firmware_mnt/image/*]
mode: 0771
user: AID_SYSTEM
group: AID_SYSTEM
caps: 0
