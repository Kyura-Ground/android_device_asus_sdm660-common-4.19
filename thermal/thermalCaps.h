#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

class ThermalCapsController {
public:
    ThermalCapsController();

    // cpu_temp_mC in millicelsius (e.g. 65000 = 65C)
    void update(int64_t cpu_temp_mC);

private:
    void loadOnceLocked();
    void applyLevelLocked(int level);

    static bool readFile(const std::string& path, std::string* out);
    static bool writeFile(const std::string& path, const std::string& val);
    static bool readInt64(const std::string& path, int64_t* out);
    static std::vector<int64_t> parseIntList(const std::string& s);
    static int64_t clampToAvail(int64_t target, const std::vector<int64_t>& avail);
    static int64_t pickStepDown(const std::vector<int64_t>& avail, size_t steps);
    static void sortUnique(std::vector<int64_t>* vals);
    static std::vector<int64_t> buildCpuFallbackTable(int policy, int64_t hwMax);

    std::mutex mLock;
    bool mLoaded = false;

    // CPU sysfs
    const std::string CPU0_MAX       = "/sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq";
    const std::string CPU4_MAX       = "/sys/devices/system/cpu/cpufreq/policy4/scaling_max_freq";
    const std::string CPU0_INFO_MAX  = "/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_max_freq";
    const std::string CPU4_INFO_MAX  = "/sys/devices/system/cpu/cpufreq/policy4/cpuinfo_max_freq";
    const std::string CPU0_AVAIL     = "/sys/devices/system/cpu/cpufreq/policy0/scaling_available_frequencies";
    const std::string CPU4_AVAIL     = "/sys/devices/system/cpu/cpufreq/policy4/scaling_available_frequencies";

    // GPU core devfreq
    const std::string GPU_DIR   = "/sys/class/devfreq/5000000.qcom,kgsl-3d0";
    const std::string GPU_MAX   = GPU_DIR + "/max_freq";
    const std::string GPU_AVAIL = GPU_DIR + "/available_frequencies";

    // GPU BW devfreq
    const std::string GPUBW_DIR   = "/sys/class/devfreq/soc:qcom,gpubw";
    const std::string GPUBW_MAX   = GPUBW_DIR + "/max_freq";
    const std::string GPUBW_AVAIL = GPUBW_DIR + "/available_frequencies";

    // DDR BW devfreq
    const std::string DDR_DIR   = "/sys/class/devfreq/soc:qcom,cpu-cpu-ddr-bw";
    const std::string DDR_MAX   = DDR_DIR + "/max_freq";
    const std::string DDR_AVAIL = DDR_DIR + "/available_frequencies";

    // Cached caps/freq tables
    int64_t mCpu0HwMax = 0;
    int64_t mCpu4HwMax = 0;

    std::vector<int64_t> mCpu0Avail;
    std::vector<int64_t> mCpu4Avail;
    std::vector<int64_t> mGpuAvail;
    std::vector<int64_t> mGpuBwAvail;
    std::vector<int64_t> mDdrAvail;

    int mLastLevel = -1;

    // Thermal thresholds
    static constexpr int64_t T0   = 65000; // mild
    static constexpr int64_t T1   = 72000; // medium
    static constexpr int64_t T2   = 78000; // strong
    static constexpr int64_t T3   = 83000; // emergency
    static constexpr int64_t HYST = 3000;  // 3C
};
