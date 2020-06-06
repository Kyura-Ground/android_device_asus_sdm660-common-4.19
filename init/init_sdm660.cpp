/*
 * ASUS X00TD/X01BD — UNIVERSAL ANDROID 14/15/16 — 2025
 * RAM variants: 3GB / 4GB / 6GB
 */

#include <cstdlib>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/sysinfo.h>

#include <android-base/logging.h>
#include <android-base/properties.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "property_service.h"
#include "vendor_init.h"

using std::string;

/* Dalvik VM */
static string heapstartsize, heapgrowthlimit, heapsize;
static string heaptargetutilization, heapminfree, heapmaxfree;

/* ZRAM */
static string zram_size, zram_streams, zram_swappiness, zram_algo;

/* LMKD */
static string lmk_psi_partial, lmk_psi_complete, lmk_swap_util;
static string lmk_thrashing_limit, lmk_thrashing_decay;
static string lmk_kill_heaviest, lmk_kill_timeout;
static string lmk_upgrade_pressure, lmk_downgrade_pressure;

/* RAM variant identifier */
static string ram_variant;

static void property_override(const char* prop, const char* value, bool add = true)
{
    prop_info* pi = (prop_info*)__system_property_find(prop);
    if (pi) {
        __system_property_update(pi, value, strlen(value));
    } else if (add) {
        __system_property_add(prop, strlen(prop), value, strlen(value));
    }
}

/* ═══════════════════════════════════════════════════════════════════
 * PSI AVAILABILITY CHECK — Критично для Android 14+
 * ═══════════════════════════════════════════════════════════════════ */
static bool is_psi_available()
{
    std::ifstream psi_file("/proc/pressure/memory");
    return psi_file.good();
}

static void configure_by_ram()
{
    struct sysinfo sys {};
    sysinfo(&sys);
    const long ram_mb = (sys.totalram * sys.mem_unit) / (1024L * 1024L);

    LOG(INFO) << "SDM660-X00TD/X01BD: Detected RAM: " << ram_mb << " MB";

    if (ram_mb >= 5000) {
        /* ═══════════════════════════════════════════════════════════
         *  6 GB PROFILE
         * ═══════════════════════════════════════════════════════════ */
        ram_variant = "6GB";
        LOG(INFO) << "Applying 6GB RAM profile";

        heapstartsize = "16m";
        heapgrowthlimit = "256m";
        heapsize = "512m";
        heaptargetutilization = "0.5";
        heapminfree = "8m";
        heapmaxfree = "48m";

        zram_size = "2684354560";       // 2.5 GB
        zram_streams = "8";
        zram_swappiness = "100";
        zram_algo = "lz4";

        lmk_psi_partial = "100";        // ★ Исправлено: было 300 — слишком высоко
        lmk_psi_complete = "700";
        lmk_swap_util = "100";
        lmk_thrashing_limit = "60";
        lmk_thrashing_decay = "20";
        lmk_kill_heaviest = "false";
        lmk_kill_timeout = "100";
        lmk_upgrade_pressure = "100";   // ★ Исправлено: не апгрейдить
        lmk_downgrade_pressure = "60";

    } else if (ram_mb >= 3400) {
        /* ═══════════════════════════════════════════════════════════
         *  4 GB PROFILE
         * ═══════════════════════════════════════════════════════════ */
        ram_variant = "4GB";
        LOG(INFO) << "Applying 4GB RAM profile";

        heapstartsize = "8m";
        heapgrowthlimit = "256m";
        heapsize = "512m";
        heaptargetutilization = "0.6";
        heapminfree = "4m";
        heapmaxfree = "32m";

        zram_size = "2147483648";       // 2 GB (50% RAM)
        zram_streams = "8";
        zram_swappiness = "100";
        zram_algo = "lz4";

        lmk_psi_partial = "80";         // ★ Исправлено
        lmk_psi_complete = "600";
        lmk_swap_util = "100";
        lmk_thrashing_limit = "50";
        lmk_thrashing_decay = "15";
        lmk_kill_heaviest = "true";
        lmk_kill_timeout = "80";
        lmk_upgrade_pressure = "100";
        lmk_downgrade_pressure = "50";

    } else {
        /* ═══════════════════════════════════════════════════════════
         *  3 GB PROFILE
         * ═══════════════════════════════════════════════════════════ */
        ram_variant = "3GB";
        LOG(INFO) << "Applying 3GB RAM profile";

        heapstartsize = "8m";
        heapgrowthlimit = "192m";
        heapsize = "384m";
        heaptargetutilization = "0.75";
        heapminfree = "2m";
        heapmaxfree = "8m";

        zram_size = "1610612736";       // 1.5 GB (50% RAM)
        zram_streams = "4";             // ★ Меньше потоков для 3GB
        zram_swappiness = "100";
        zram_algo = "lz4";

        lmk_psi_partial = "70";         // ★ Агрессивнее для 3GB
        lmk_psi_complete = "500";
        lmk_swap_util = "100";
        lmk_thrashing_limit = "30";     // ★ Агрессивнее
        lmk_thrashing_decay = "10";
        lmk_kill_heaviest = "true";
        lmk_kill_timeout = "50";        // ★ Быстрее убивать
        lmk_upgrade_pressure = "100";
        lmk_downgrade_pressure = "30";
    }
    
    property_override("ro.boot.hardware.ram", ram_variant.c_str());
}

static void NFC_check()
{
    const char* nfc_path = "/proc/NFC_CHECK";
    std::ifstream infile;
    std::string line;
    int retries = 30;

    while (retries-- > 0) {
        infile.open(nfc_path);
        if (infile.is_open())
            break;
        usleep(100000);
    }

    if (infile.is_open() && getline(infile, line)) {
        infile.close();
        
        if (line == "SUPPORTED") {
            property_override("ro.hq.support.nfc", "1");
            property_override("ro.boot.product.hardware.sku", "NFC");
            property_override("persist.sys.nfc.supported", "true");
            LOG(INFO) << "NFC: Hardware detected - ENABLED";
        } else {
            property_override("ro.hq.support.nfc", "0");
            property_override("ro.boot.product.hardware.sku", "");
            property_override("persist.sys.nfc.supported", "false");
            LOG(INFO) << "NFC: Not present - DISABLED";
        }
    } else {
        property_override("ro.hq.support.nfc", "0");
        property_override("ro.boot.product.hardware.sku", "");
        property_override("persist.sys.nfc.supported", "false");
        LOG(INFO) << "NFC: Detection failed - DISABLED";
    }
}

static void set_dalvik_properties()
{
    property_override("dalvik.vm.heapstartsize", heapstartsize.c_str());
    property_override("dalvik.vm.heapgrowthlimit", heapgrowthlimit.c_str());
    property_override("dalvik.vm.heapsize", heapsize.c_str());
    property_override("dalvik.vm.heaptargetutilization", heaptargetutilization.c_str());
    property_override("dalvik.vm.heapminfree", heapminfree.c_str());
    property_override("dalvik.vm.heapmaxfree", heapmaxfree.c_str());
    
    /* Оптимизации DEX2OAT */
    property_override("dalvik.vm.dex2oat-threads", "4");
    property_override("dalvik.vm.image-dex2oat-threads", "4");
    property_override("dalvik.vm.dex2oat64.enabled", "true");
    property_override("dalvik.vm.usejit", "true");
    property_override("dalvik.vm.usejitprofiles", "true");
    
    /* ★ ВАЖНО для Android 14+ */
    property_override("dalvik.vm.dex2oat-minidebuginfo", "false");
    property_override("dalvik.vm.minidebuginfo", "false");
}

static void set_zram_properties()
{
    property_override("vendor.zram.size", zram_size.c_str());
    property_override("vendor.zram.streams", zram_streams.c_str());
    property_override("vendor.zram.swappiness", zram_swappiness.c_str());
    property_override("vendor.zram.comp_algorithm", zram_algo.c_str());
    property_override("vendor.zram.enabled", "true");
    
    /* ★ Дополнительные параметры ZRAM для Android 16 */
    property_override("ro.zram.mark_idle_delay_mins", "60");
    property_override("ro.zram.first_wb_delay_mins", "180");
    property_override("ro.zram.periodic_wb_delay_hours", "24");
}

static void set_lmkd_properties()
{
    /* ★ КРИТИЧНО: Проверяем доступность PSI */
    bool psi_enabled = is_psi_available();
    
    if (psi_enabled) {
        property_override("ro.lmk.use_psi", "true");
        LOG(INFO) << "LMKD: PSI enabled";
    } else {
        property_override("ro.lmk.use_psi", "false");
        property_override("ro.lmk.use_minfree_levels", "true");
        LOG(WARNING) << "LMKD: PSI not available, falling back to minfree";
    }
    
    /* ★ Исправлено: use_minfree_levels должен быть false если PSI работает */
    property_override("ro.lmk.use_minfree_levels", psi_enabled ? "false" : "true");
    
    property_override("ro.lmk.psi_partial_stall_ms", lmk_psi_partial.c_str());
    property_override("ro.lmk.psi_complete_stall_ms", lmk_psi_complete.c_str());
    property_override("ro.lmk.swap_util_max", lmk_swap_util.c_str());
    property_override("ro.lmk.thrashing_limit", lmk_thrashing_limit.c_str());
    property_override("ro.lmk.thrashing_limit_decay", lmk_thrashing_decay.c_str());
    property_override("ro.lmk.kill_heaviest_task", lmk_kill_heaviest.c_str());
    property_override("ro.lmk.kill_timeout_ms", lmk_kill_timeout.c_str());
    property_override("ro.lmk.upgrade_pressure", lmk_upgrade_pressure.c_str());
    property_override("ro.lmk.downgrade_pressure", lmk_downgrade_pressure.c_str());

    /* Фиксированные параметры */
    property_override("ro.lmk.swap_free_low_percentage", "10");
    property_override("ro.lmk.psi_scrit_complete_stall_ms", "400");
    property_override("ro.lmk.critical_upgrade", "false");  // ★ Исправлено: было true
    property_override("ro.lmk.filecache_min_kb", "153600");
    property_override("ro.lmk.stall_limit_critical", "50");

    /* ★ Низкий порог когда начинать убивать */
    property_override("ro.lmk.low", "1001");
    property_override("ro.lmk.medium", "800");
    property_override("ro.lmk.critical", "0");

    /* Не low_ram устройства */
    property_override("ro.lmk.low_ram", "false");
    property_override("ro.config.low_ram", "false");

    /* Debug off для production */
    property_override("ro.lmk.debug", "false");
    property_override("ro.lmk.log_stats", "false");
}

static void set_performance_properties()
{
    /* Surfaceflinger */
    property_override("debug.sf.latch_unsignaled", "0");
    property_override("debug.sf.disable_backpressure", "1");
    property_override("debug.sf.enable_gl_backpressure", "0");
    
    /* HWUI Renderer */
    property_override("debug.hwui.renderer", "skiagl");
    property_override("renderthread.skia.reduceopstasksplitting", "true");
    
    /* ★ Важно для Android 14+ */
    property_override("ro.surface_flinger.max_frame_buffer_acquired_buffers", "3");
    property_override("ro.surface_flinger.running_without_sync_framework", "false");
    
    /* Disable logging for performance */
    property_override("persist.traced.enable", "0");
    property_override("persist.sys.raf.override", "false");
}

/* ═══════════════════════════════════════════════════════════════════
 * MAIN ENTRY POINT
 * ═══════════════════════════════════════════════════════════════════ */
void vendor_load_properties()
{
    configure_by_ram();
    NFC_check();

    set_dalvik_properties();
    set_zram_properties();
    set_lmkd_properties();
    set_performance_properties();

    LOG(INFO) << "SDM660-X00TD/X01BD: vendor properties loaded for " 
              << ram_variant << " variant";
}
