#!/vendor/bin/sh

# ═══════════════════════════════════════════════════════════════════════════
# init.qcom.post_boot.sh for SDM660/SDM636 on Kernel 4.19 (EAS)
# ASUS X00TD (ZenFone Max Pro M1) / X01BD (ZenFone Max Pro M2)
#
# Correct CPU topology:
#   policy0 -> cpu0-3 -> Kryo 260 Silver (LITTLE)
#   policy4 -> cpu4-7 -> Kryo 260 Gold   (BIG)
#
# No hardcoded CPU caps. Real min/max read from kernel sysfs.
# Works automatically on both SDM636 and SDM660.
# ═══════════════════════════════════════════════════════════════════════════

LOGTAG="post_boot_sdm660"

write()     { [ -e "$1" ] && echo "$2" > "$1" 2>/dev/null; }
write_str() { [ -e "$1" ] && printf '%s' "$2" > "$1" 2>/dev/null; }
read_one()  { [ -e "$1" ] && cat "$1" 2>/dev/null; }

first_freq() {
    if [ -e "$1" ]; then
        cat "$1" 2>/dev/null | tr ' ' '\n' | sort -n | head -1
    fi
}

last_freq() {
    if [ -e "$1" ]; then
        cat "$1" 2>/dev/null | tr ' ' '\n' | sort -n | tail -1
    fi
}

# ═══════════════════════════════════════════════════════════════════════════
# 1. CPU GOVERNOR — SCHEDUTIL
# ═══════════════════════════════════════════════════════════════════════════
configure_cpu_governor() {
    # Read real limits from kernel
    cpu0_hwmax="$(read_one /sys/devices/system/cpu/cpufreq/policy0/cpuinfo_max_freq)"
    cpu4_hwmax="$(read_one /sys/devices/system/cpu/cpufreq/policy4/cpuinfo_max_freq)"
    cpu0_hwmin="$(read_one /sys/devices/system/cpu/cpufreq/policy0/cpuinfo_min_freq)"
    cpu4_hwmin="$(read_one /sys/devices/system/cpu/cpufreq/policy4/cpuinfo_min_freq)"

    # Try scaling_available_frequencies as fallback
    [ -z "$cpu0_hwmin" ] && cpu0_hwmin="$(first_freq /sys/devices/system/cpu/cpufreq/policy0/scaling_available_frequencies)"
    [ -z "$cpu4_hwmin" ] && cpu4_hwmin="$(first_freq /sys/devices/system/cpu/cpufreq/policy4/scaling_available_frequencies)"
    [ -z "$cpu0_hwmax" ] && cpu0_hwmax="$(last_freq  /sys/devices/system/cpu/cpufreq/policy0/scaling_available_frequencies)"
    [ -z "$cpu4_hwmax" ] && cpu4_hwmax="$(last_freq  /sys/devices/system/cpu/cpufreq/policy4/scaling_available_frequencies)"

    # Final fallbacks (SDM636 values as safe minimum)
    [ -z "$cpu0_hwmin" ] && cpu0_hwmin=633600
    [ -z "$cpu4_hwmin" ] && cpu4_hwmin=1113600
    [ -z "$cpu0_hwmax" ] && cpu0_hwmax=1612800
    [ -z "$cpu4_hwmax" ] && cpu4_hwmax=1804800

    log -t "$LOGTAG" -p i "policy0 LITTLE: min=${cpu0_hwmin} max=${cpu0_hwmax}"
    log -t "$LOGTAG" -p i "policy4 BIG:    min=${cpu4_hwmin} max=${cpu4_hwmax}"

    # LITTLE cluster: policy0 / cpu0-3
    write /sys/devices/system/cpu/cpufreq/policy0/scaling_governor schedutil
    write /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq "$cpu0_hwmin"
    write /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq "$cpu0_hwmax"
    write /sys/devices/system/cpu/cpufreq/policy0/schedutil/up_rate_limit_us 500
    write /sys/devices/system/cpu/cpufreq/policy0/schedutil/down_rate_limit_us 20000
    write /sys/devices/system/cpu/cpufreq/policy0/schedutil/iowait_boost_enable 1

    # BIG cluster: policy4 / cpu4-7
    write /sys/devices/system/cpu/cpufreq/policy4/scaling_governor schedutil
    write /sys/devices/system/cpu/cpufreq/policy4/scaling_min_freq "$cpu4_hwmin"
    write /sys/devices/system/cpu/cpufreq/policy4/scaling_max_freq "$cpu4_hwmax"
    write /sys/devices/system/cpu/cpufreq/policy4/schedutil/up_rate_limit_us 500
    write /sys/devices/system/cpu/cpufreq/policy4/schedutil/down_rate_limit_us 40000
    write /sys/devices/system/cpu/cpufreq/policy4/schedutil/iowait_boost_enable 1

    # Ensure all cores are online
    for cpu in /sys/devices/system/cpu/cpu[0-7]; do
        if [ -f "$cpu/online" ]; then
            echo 1 > "$cpu/online"
        fi
    done
}

# ═══════════════════════════════════════════════════════════════════════════
# 2. SCHEDTUNE (EAS)
# ═══════════════════════════════════════════════════════════════════════════
configure_eas_schedtune() {
    write /dev/stune/top-app/schedtune.boost 10
    write /dev/stune/top-app/schedtune.prefer_idle 1

    write /dev/stune/foreground/schedtune.boost 0
    write /dev/stune/foreground/schedtune.prefer_idle 0

    write /dev/stune/background/schedtune.boost 0
    write /dev/stune/background/schedtune.prefer_idle 0

    write /dev/stune/rt/schedtune.boost 5
    write /dev/stune/rt/schedtune.prefer_idle 1
}

# ═══════════════════════════════════════════════════════════════════════════
# 3. CPUSETS
# ═══════════════════════════════════════════════════════════════════════════
configure_cpusets() {
    # LITTLE = cpu0-3 → background tasks (power saving)
    # BIG    = cpu4-7
    write /dev/cpuset/background/cpus 0-3
    write /dev/cpuset/system-background/cpus 0-3
    write /dev/cpuset/restricted/cpus 0-3

    # Foreground + Top-App: all cores
    write /dev/cpuset/foreground/cpus 0-7
    write /dev/cpuset/top-app/cpus 0-7
}

# ═══════════════════════════════════════════════════════════════════════════
# 4. WALT TUNING
# ═══════════════════════════════════════════════════════════════════════════
configure_walt() {
    write /proc/sys/kernel/sched_walt_rotate_big_tasks 1
    write /proc/sys/kernel/sched_upmigrate 95
    write /proc/sys/kernel/sched_downmigrate 85
    write /proc/sys/kernel/sched_group_upmigrate 95
    write /proc/sys/kernel/sched_group_downmigrate 85
}

# ═══════════════════════════════════════════════════════════════════════════
# 5. CORE CONTROL
# ═══════════════════════════════════════════════════════════════════════════
configure_core_ctl() {
    # LITTLE cluster core_ctl = cpu0
    if [ -d /sys/devices/system/cpu/cpu0/core_ctl ]; then
        write /sys/devices/system/cpu/cpu0/core_ctl/min_cpus 2
        write /sys/devices/system/cpu/cpu0/core_ctl/max_cpus 4
        write /sys/devices/system/cpu/cpu0/core_ctl/busy_up_thres 60
        write /sys/devices/system/cpu/cpu0/core_ctl/busy_down_thres 30
        write /sys/devices/system/cpu/cpu0/core_ctl/offline_delay_ms 100
        write /sys/devices/system/cpu/cpu0/core_ctl/is_big_cluster 0
    fi

    # BIG cluster core_ctl = cpu4
    if [ -d /sys/devices/system/cpu/cpu4/core_ctl ]; then
        write /sys/devices/system/cpu/cpu4/core_ctl/min_cpus 1
        write /sys/devices/system/cpu/cpu4/core_ctl/max_cpus 4
        write /sys/devices/system/cpu/cpu4/core_ctl/busy_up_thres 70
        write /sys/devices/system/cpu/cpu4/core_ctl/busy_down_thres 40
        write /sys/devices/system/cpu/cpu4/core_ctl/offline_delay_ms 200
        write /sys/devices/system/cpu/cpu4/core_ctl/is_big_cluster 1
    fi
}

# ═══════════════════════════════════════════════════════════════════════════
# 6. BUS DCVS
# ═══════════════════════════════════════════════════════════════════════════
configure_bus_dcvs() {
    for cpubw in /sys/devices/platform/soc/*cpu-cpu-ddr-bw/devfreq/*cpu-cpu-ddr-bw; do
        if [ -d "$cpubw" ]; then
            write_str "$cpubw/governor" "bw_hwmon"
            write "$cpubw/polling_interval" 50
            write "$cpubw/min_freq" 762
            write_str "$cpubw/bw_hwmon/mbps_zones" "762 1571 2086 2929 3879 5163 5931 6881"
            write "$cpubw/bw_hwmon/sample_ms" 4
            write "$cpubw/bw_hwmon/io_percent" 85
            write "$cpubw/bw_hwmon/decay_rate" 100
            write "$cpubw/bw_hwmon/bw_step" 50
            write "$cpubw/bw_hwmon/hist_memory" 20
            write "$cpubw/bw_hwmon/hyst_length" 0
            write "$cpubw/bw_hwmon/down_thres" 80
            write "$cpubw/bw_hwmon/guard_band_mbps" 0
            write "$cpubw/bw_hwmon/up_scale" 250
            write "$cpubw/bw_hwmon/idle_mbps" 1600
        fi
    done

    for gpubw in /sys/devices/platform/soc/*gpu-cpu-ddr-bw/devfreq/*gpu-cpu-ddr-bw; do
        if [ -d "$gpubw" ]; then
            write_str "$gpubw/governor" "bw_hwmon"
            write "$gpubw/polling_interval" 50
            write "$gpubw/min_freq" 762
            write_str "$gpubw/bw_hwmon/mbps_zones" "762 1571 2086 2929 3879 5163 5931 6881"
            write "$gpubw/bw_hwmon/sample_ms" 4
            write "$gpubw/bw_hwmon/io_percent" 80
            write "$gpubw/bw_hwmon/decay_rate" 100
            write "$gpubw/bw_hwmon/bw_step" 50
            write "$gpubw/bw_hwmon/hist_memory" 20
            write "$gpubw/bw_hwmon/hyst_length" 0
            write "$gpubw/bw_hwmon/down_thres" 80
            write "$gpubw/bw_hwmon/guard_band_mbps" 0
            write "$gpubw/bw_hwmon/up_scale" 250
            write "$gpubw/bw_hwmon/idle_mbps" 1600
        fi
    done
}

# ═══════════════════════════════════════════════════════════════════════════
# 7. I/O TUNING — final values after boot
# ═══════════════════════════════════════════════════════════════════════════
configure_storage_io() {
    # Internal eMMC
    if [ -b /dev/block/mmcblk0 ] || [ -d /sys/block/mmcblk0 ]; then
        write /sys/block/mmcblk0/queue/read_ahead_kb 128
        write /sys/block/mmcblk0/queue/iosched/slice_idle 0
        write /sys/block/mmcblk0/queue/iosched/low_latency 1
        write /sys/block/mmcblk0/queue/iosched/back_seek_penalty 1
    fi

    # SD Card
    if [ -b /dev/block/mmcblk1 ] || [ -d /sys/block/mmcblk1 ]; then
        write_str /sys/block/mmcblk1/queue/scheduler bfq
        write /sys/block/mmcblk1/queue/read_ahead_kb 128
        write /sys/block/mmcblk1/queue/iosched/slice_idle 0
        write /sys/block/mmcblk1/queue/iosched/low_latency 1
    fi

    # dm devices (FBE layers)
    for dm in /sys/block/dm-*; do
        [ -d "$dm" ] && write "$dm/queue/read_ahead_kb" 128
    done
}

# ═══════════════════════════════════════════════════════════════════════════
# 8. DISABLE LEGACY KERNEL THERMAL
# ═══════════════════════════════════════════════════════════════════════════
disable_legacy_kernel_thermal() {
    write /sys/module/msm_thermal/parameters/enabled N
    write /sys/module/msm_thermal/core_control/enabled 0
}

# ═══════════════════════════════════════════════════════════════════════════
# MAIN
# ═══════════════════════════════════════════════════════════════════════════
main() {
    if [ ! -d /sys/devices/system/cpu/cpufreq/policy0 ] || \
       [ ! -d /sys/devices/system/cpu/cpufreq/policy4 ]; then
        log -t "$LOGTAG" -p w "cpufreq policies not present, skipping"
        setprop vendor.post_boot.parsed 1
        exit 0
    fi

    target="$(getprop ro.board.platform)"
    [ -z "$target" ] && target="unknown"

    case "$target" in
        "sdm660" | "sdm636")
            log -t "$LOGTAG" -p i "Starting post_boot for platform: $target"

            write /proc/irq/default_smp_affinity ff

            disable_legacy_kernel_thermal
            configure_core_ctl
            configure_cpu_governor
            configure_eas_schedtune
            configure_cpusets
            configure_walt
            configure_bus_dcvs
            configure_storage_io

            write /proc/sys/kernel/sched_boost 0

            # Optional CDSP bring-up only if node exists
            if [ -e /sys/kernel/boot_cdsp/boot ]; then
                echo 1 > /sys/kernel/boot_cdsp/boot
                log -t "$LOGTAG" -p i "boot_cdsp triggered"
            fi

            # Optional NPU bring-up only if node exists
            if [ -e /sys/devices/virtual/npu/msm_npu/boot ]; then
                echo 1 > /sys/devices/virtual/npu/msm_npu/boot
                log -t "$LOGTAG" -p i "NPU boot triggered"
            fi

            # Start CDSP RPC daemon only if CDSP exists
            if [ -e /sys/kernel/boot_cdsp/boot ] || [ -d /sys/class/remoteproc/remoteproc2 ]; then
                start vendor.cdsprpcd
                log -t "$LOGTAG" -p i "CDSP present, starting vendor.cdsprpcd"
            else
                log -t "$LOGTAG" -p i "CDSP not present, skipping vendor.cdsprpcd"
            fi

            log -t "$LOGTAG" -p i "post_boot complete for $target"
            ;;
        *)
            log -t "$LOGTAG" -p w "Unknown platform: $target, skipping post_boot"
            ;;
    esac

    setprop vendor.post_boot.parsed 1
}

main
