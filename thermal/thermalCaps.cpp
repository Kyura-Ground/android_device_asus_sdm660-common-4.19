#include "thermalCaps.h"

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include <android-base/logging.h>

ThermalCapsController::ThermalCapsController() {}

bool ThermalCapsController::readFile(const std::string& path, std::string* out) {
    std::ifstream f(path);
    if (!f.is_open()) return false;

    std::stringstream ss;
    ss << f.rdbuf();
    *out = ss.str();

    while (!out->empty() &&
           (out->back() == '\n' || out->back() == '\r' ||
            out->back() == ' '  || out->back() == '\t')) {
        out->pop_back();
    }
    return true;
}

bool ThermalCapsController::writeFile(const std::string& path, const std::string& val) {
    std::ofstream f(path);
    if (!f.is_open()) {
        LOG(WARNING) << "ThermalCaps: failed to open " << path << " for writing";
        return false;
    }
    f << val;
    return f.good();
}

bool ThermalCapsController::readInt64(const std::string& path, int64_t* out) {
    std::string s;
    if (!readFile(path, &s)) return false;
    if (s.empty()) return false;

    char* end = nullptr;
    errno = 0;
    long long v = std::strtoll(s.c_str(), &end, 10);
    if (errno != 0 || end == s.c_str()) return false;

    *out = static_cast<int64_t>(v);
    return true;
}

std::vector<int64_t> ThermalCapsController::parseIntList(const std::string& s) {
    std::vector<int64_t> vals;
    std::stringstream ss(s);
    int64_t v;
    while (ss >> v) {
        vals.push_back(v);
    }
    return vals;
}

void ThermalCapsController::sortUnique(std::vector<int64_t>* vals) {
    std::sort(vals->begin(), vals->end());
    vals->erase(std::unique(vals->begin(), vals->end()), vals->end());
}

int64_t ThermalCapsController::clampToAvail(int64_t target,
                                            const std::vector<int64_t>& avail) {
    if (avail.empty()) return target;

    int64_t best = 0;
    for (auto f : avail) {
        if (f <= target && f >= best) best = f;
    }

    if (best == 0) {
        best = *std::min_element(avail.begin(), avail.end());
    }

    return best;
}

int64_t ThermalCapsController::pickStepDown(const std::vector<int64_t>& avail, size_t steps) {
    if (avail.empty()) return 0;
    if (steps >= avail.size()) return avail.front();
    return avail[avail.size() - 1 - steps];
}

std::vector<int64_t> ThermalCapsController::buildCpuFallbackTable(int policy, int64_t hwMax) {
    // Fallback only if scaling_available_frequencies is unavailable.
    if (policy == 0) {
        // little cluster
        if (hwMax >= 1843200) {
            return {633600, 902400, 1113600, 1401600, 1536000, 1612800, 1747200, 1843200};
        }
        return {633600, 902400, 1113600, 1401600, 1536000, 1612800};
    } else {
        // big cluster
        if (hwMax >= 2208000) {
            return {1113600, 1401600, 1747200, 1804800, 1958400, 2150400, 2208000};
        }
        if (hwMax >= 1958400) {
            return {1113600, 1401600, 1747200, 1804800, 1958400};
        }
        return {1113600, 1401600, 1747200, 1804800};
    }
}

void ThermalCapsController::loadOnceLocked() {
    if (mLoaded) return;
    mLoaded = true;

    int64_t v = 0;
    if (readInt64(CPU0_INFO_MAX, &v)) mCpu0HwMax = v;
    if (readInt64(CPU4_INFO_MAX, &v)) mCpu4HwMax = v;

    std::string s;

    // CPU available freqs
    if (readFile(CPU0_AVAIL, &s)) {
        mCpu0Avail = parseIntList(s);
        sortUnique(&mCpu0Avail);
    }
    if (readFile(CPU4_AVAIL, &s)) {
        mCpu4Avail = parseIntList(s);
        sortUnique(&mCpu4Avail);
    }

    // Filter out frequencies above real hw max
    if (mCpu0HwMax > 0 && !mCpu0Avail.empty()) {
        mCpu0Avail.erase(
                std::remove_if(mCpu0Avail.begin(), mCpu0Avail.end(),
                               [&](int64_t f) { return f > mCpu0HwMax; }),
                mCpu0Avail.end());
    }
    if (mCpu4HwMax > 0 && !mCpu4Avail.empty()) {
        mCpu4Avail.erase(
                std::remove_if(mCpu4Avail.begin(), mCpu4Avail.end(),
                               [&](int64_t f) { return f > mCpu4HwMax; }),
                mCpu4Avail.end());
    }

    // Fallback if kernel does not expose scaling_available_frequencies
    if (mCpu0Avail.empty()) mCpu0Avail = buildCpuFallbackTable(0, mCpu0HwMax);
    if (mCpu4Avail.empty()) mCpu4Avail = buildCpuFallbackTable(4, mCpu4HwMax);

    if (mCpu0HwMax <= 0 && !mCpu0Avail.empty()) mCpu0HwMax = mCpu0Avail.back();
    if (mCpu4HwMax <= 0 && !mCpu4Avail.empty()) mCpu4HwMax = mCpu4Avail.back();

    // GPU available frequencies
    if (readFile(GPU_AVAIL, &s)) {
        mGpuAvail = parseIntList(s);
        sortUnique(&mGpuAvail);
    }

    // GPU BW available frequencies
    if (readFile(GPUBW_AVAIL, &s)) {
        mGpuBwAvail = parseIntList(s);
        sortUnique(&mGpuBwAvail);
    }

    // DDR BW available frequencies
    if (readFile(DDR_AVAIL, &s)) {
        mDdrAvail = parseIntList(s);
        sortUnique(&mDdrAvail);
    }

    LOG(INFO) << "ThermalCaps: loaded"
              << " cpu0_hwmax=" << mCpu0HwMax
              << " cpu4_hwmax=" << mCpu4HwMax
              << " cpu0_avail=" << mCpu0Avail.size()
              << " cpu4_avail=" << mCpu4Avail.size()
              << " gpu_avail=" << mGpuAvail.size()
              << " gpubw_avail=" << mGpuBwAvail.size()
              << " ddr_avail=" << mDdrAvail.size();
}

void ThermalCapsController::applyLevelLocked(int level) {
    auto getMax = [](const std::vector<int64_t>& avail) -> int64_t {
        if (avail.empty()) return 0;
        return avail.back();
    };

    auto pickFrac = [](const std::vector<int64_t>& avail, double frac) -> int64_t {
        if (avail.empty()) return 0;
        int64_t maxf = avail.back();
        int64_t target = static_cast<int64_t>(maxf * frac);
        return ThermalCapsController::clampToAvail(target, avail);
    };

    int64_t cpu0max = mCpu0HwMax;
    int64_t cpu4max = mCpu4HwMax;

    size_t littleStepsDown = 0;
    size_t bigStepsDown = 0;

    double gpuFrac   = 1.0;
    double gpuBwFrac = 1.0;
    double ddrFrac   = 1.0;

    switch (level) {
        case 0:
            // full performance
            break;

        case 1:
            // mild
            littleStepsDown = 1;
            bigStepsDown = 1;
            gpuFrac = 0.90;
            gpuBwFrac = 0.90;
            ddrFrac = 0.90;
            break;

        case 2:
            // medium
            littleStepsDown = 2;
            bigStepsDown = 2;
            gpuFrac = 0.75;
            gpuBwFrac = 0.75;
            ddrFrac = 0.75;
            break;

        case 3:
            // strong
            littleStepsDown = 3;
            bigStepsDown = 3;
            gpuFrac = 0.60;
            gpuBwFrac = 0.60;
            ddrFrac = 0.60;
            break;

        default:
            // emergency
            littleStepsDown = 4;
            bigStepsDown = 5;
            gpuFrac = 0.45;
            gpuBwFrac = 0.45;
            ddrFrac = 0.45;
            break;
    }

    if (level == 0) {
        cpu0max = getMax(mCpu0Avail);
        cpu4max = getMax(mCpu4Avail);
    } else {
        cpu0max = pickStepDown(mCpu0Avail, littleStepsDown);
        cpu4max = pickStepDown(mCpu4Avail, bigStepsDown);
    }

    if (cpu0max <= 0) cpu0max = mCpu0HwMax;
    if (cpu4max <= 0) cpu4max = mCpu4HwMax;

    if (mCpu0HwMax > 0) cpu0max = std::min(cpu0max, mCpu0HwMax);
    if (mCpu4HwMax > 0) cpu4max = std::min(cpu4max, mCpu4HwMax);

    // Apply CPU caps
    writeFile(CPU0_MAX, std::to_string(cpu0max));
    writeFile(CPU4_MAX, std::to_string(cpu4max));

    // Apply GPU cap
    if (!mGpuAvail.empty()) {
        int64_t gpuMax = (level == 0) ? getMax(mGpuAvail) : pickFrac(mGpuAvail, gpuFrac);
        if (gpuMax > 0) writeFile(GPU_MAX, std::to_string(gpuMax));
    }

    // Apply GPU BW cap
    if (!mGpuBwAvail.empty()) {
        int64_t gpuBwMax = (level == 0) ? getMax(mGpuBwAvail) : pickFrac(mGpuBwAvail, gpuBwFrac);
        if (gpuBwMax > 0) writeFile(GPUBW_MAX, std::to_string(gpuBwMax));
    }

    // Apply DDR BW cap
    if (!mDdrAvail.empty()) {
        int64_t ddrMax = (level == 0) ? getMax(mDdrAvail) : pickFrac(mDdrAvail, ddrFrac);
        if (ddrMax > 0) writeFile(DDR_MAX, std::to_string(ddrMax));
    }

    LOG(INFO) << "ThermalCaps: applied level=" << level
              << " cpu0=" << cpu0max
              << " cpu4=" << cpu4max
              << " gpuFrac=" << gpuFrac
              << " gpuBwFrac=" << gpuBwFrac
              << " ddrFrac=" << ddrFrac;
}

void ThermalCapsController::update(int64_t t) {
    std::lock_guard<std::mutex> lk(mLock);
    loadOnceLocked();

    int level = 0;
    if (t >= T3)      level = 4;
    else if (t >= T2) level = 3;
    else if (t >= T1) level = 2;
    else if (t >= T0) level = 1;

    // hysteresis on cooldown
    if (mLastLevel >= 0 && level < mLastLevel) {
        int64_t cool = 0;
        switch (mLastLevel) {
            case 4: cool = T3 - HYST; break;
            case 3: cool = T2 - HYST; break;
            case 2: cool = T1 - HYST; break;
            case 1: cool = T0 - HYST; break;
            default: break;
        }
        if (t > cool) {
            level = mLastLevel;
        }
    }

    if (level != mLastLevel) {
        applyLevelLocked(level);
        mLastLevel = level;
    }
}
