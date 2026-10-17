/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cerrno>
#include <charconv>
#include <cstdio>
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <string>
#include <unordered_map>

#include <android-base/logging.h>

#include "thermalCommon.h"
#include "thermalCaps.h"

#define MAX_LENGTH          50
#define MAX_PATH            256
#define DEFAULT_HYSTERESIS  5000
#define THERMAL_SYSFS       "/sys/class/thermal/"
#define TZ_DIR_NAME         "thermal_zone"
#define TZ_DIR_FMT          "thermal_zone%d"
#define TEMPERATURE_FILE_FORMAT  "/sys/class/thermal/thermal_zone%d/temp"
#define POLICY_FILE_FORMAT       "/sys/class/thermal/thermal_zone%d/policy"
#define TRIP_FILE_FORMAT         "/sys/class/thermal/thermal_zone%d/trip_point_1_temp"
#define HYST_FILE_FORMAT         "/sys/class/thermal/thermal_zone%d/trip_point_1_hyst"
#define USER_SPACE_POLICY   "user_space"
#define TZ_TYPE             "type"
#define CDEV_DIR_NAME       "cooling_device"
#define CDEV_DIR_FMT        "cooling_device%d"
#define CDEV_CUR_STATE_PATH "/sys/class/thermal/cooling_device%d/cur_state"

namespace aidl::android::hardware::thermal::implementation {

static ThermalCapsController gCaps;

static std::unordered_map<std::string, CoolingType> cdev_map = {
    {"thermal-cpufreq-0", CoolingType::CPU},
    {"thermal-cpufreq-1", CoolingType::CPU},
    {"thermal-cpufreq-2", CoolingType::CPU},
    {"thermal-cpufreq-3", CoolingType::CPU},
    {"thermal-cpufreq-4", CoolingType::CPU},
    {"thermal-cpufreq-5", CoolingType::CPU},
    {"thermal-cpufreq-6", CoolingType::CPU},
    {"thermal-cpufreq-7", CoolingType::CPU},
    {"thermal-cluster-4-7", CoolingType::CPU},
    {"thermal-cluster-0-3", CoolingType::CPU},
    {"cpu-isolate0", CoolingType::CPU},
    {"cpu-isolate1", CoolingType::CPU},
    {"cpu-isolate2", CoolingType::CPU},
    {"cpu-isolate3", CoolingType::CPU},
    {"cpu-isolate4", CoolingType::CPU},
    {"cpu-isolate5", CoolingType::CPU},
    {"cpu-isolate6", CoolingType::CPU},
    {"cpu-isolate7", CoolingType::CPU},
    {"thermal-devfreq-0", CoolingType::GPU},
    {"gpu", CoolingType::GPU},
    {"modem_tj", CoolingType::MODEM},
    {"modem_skin", CoolingType::MODEM},
    {"cdsp", CoolingType::NPU},
    {"cdsp_hw", CoolingType::NPU},
    {"battery", CoolingType::BATTERY},
    {"charge_cdev", CoolingType::BATTERY},
};

ThermalCommon::ThermalCommon() {
    LOG(DEBUG) << "Entering " << __func__;
    ncpus = (int)sysconf(_SC_NPROCESSORS_CONF);
    if (ncpus < 1)
        LOG(ERROR) << "Error retrieving number of cores";
}

static int writeToFile(std::string_view path, std::string data) {
    std::fstream outFile;
    outFile.open(std::string(path).c_str(),
                 std::fstream::binary | std::fstream::out);
    if (outFile.is_open()) {
        LOG(DEBUG) << "writing: " << data << " in path:" << path;
        outFile << data;
        outFile.close();
        return data.length();
    }
    LOG(ERROR) << "Error opening file: " << path;
    return -1;
}

static int readLineFromFile(std::string_view path, std::string& out) {
    char *fgets_ret;
    FILE *fd;
    int rv;
    char buf[MAX_LENGTH];

    out.clear();
    fd = fopen(std::string(path).c_str(), "r");
    if (fd == NULL) {
        LOG(ERROR) << "Path:" << std::string(path)
                   << " file open error.err:" << strerror(errno);
        return -errno;
    }

    errno = 0;
    fgets_ret = fgets(buf, MAX_LENGTH, fd);
    if (NULL != fgets_ret) {
        rv = (int)strlen(buf);
        out.append(buf, rv);
    } else {
        rv = ferror(fd) ? -(errno ? errno : EIO) : 0;
    }

    fclose(fd);
    out.erase(std::remove(out.begin(), out.end(), '\n'), out.end());
    LOG(DEBUG) << "Path:" << std::string(path) << " Val:" << out;
    return rv;
}

int ThermalCommon::readFromFile(std::string_view path, std::string& out) {
    return readLineFromFile(path, out);
}

static int get_tzn(std::string sensor_name) {
    DIR *tdir = NULL;
    struct dirent *tdirent = NULL;
    int found = -1;
    int tzn = 0;
    char name[MAX_PATH] = {0};
    char cwd[MAX_PATH] = {0};
    int ret = 0;

    if (!getcwd(cwd, sizeof(cwd)))
        return found;

    ret = chdir(THERMAL_SYSFS);
    if (ret) {
        LOG(ERROR) << "Unable to change to " << THERMAL_SYSFS;
        return found;
    }
    tdir = opendir(THERMAL_SYSFS);
    if (!tdir) {
        LOG(ERROR) << "Unable to open " << THERMAL_SYSFS;
        return found;
    }

    while ((tdirent = readdir(tdir))) {
        std::string buf;

        if (strncmp(tdirent->d_name, TZ_DIR_NAME,
                    strlen(TZ_DIR_NAME)) != 0)
            continue;

        snprintf(name, MAX_PATH, "%s%s/%s", THERMAL_SYSFS,
                 tdirent->d_name, TZ_TYPE);
        ret = readLineFromFile(std::string_view(name), buf);
        if (ret <= 0) {
            LOG(ERROR) << "get_tzn: sensor name read error for tz:"
                       << tdirent->d_name;
            continue;
        }
        if (!strncmp(buf.c_str(), sensor_name.c_str(),
                     sensor_name.length())) {
            found = 1;
            break;
        }
    }

    if (found == 1) {
        sscanf(tdirent->d_name, TZ_DIR_FMT, &tzn);
        LOG(DEBUG) << "Sensor: " << sensor_name
                   << " found at tz: " << tzn;
        found = tzn;
    }

    closedir(tdir);
    chdir(cwd);
    return found;
}

int ThermalCommon::initialize_sensor(struct target_therm_cfg& cfg,
                                      int sens_idx) {
    struct therm_sensor sensor;
    int idx = 0;

    sensor.tzn = get_tzn(cfg.sensor_list[sens_idx]);
    if (sensor.tzn < 0) {
        LOG(ERROR) << "No thermal zone for sensor: "
                   << cfg.sensor_list[sens_idx]
                   << ", ret:" << sensor.tzn;
        return -1;
    }

    if (cfg.type == TemperatureType::CPU)
        sensor.thresh.name = sensor.t.name =
            std::string("CPU") + std::to_string(sens_idx);
    else
        sensor.thresh.name = sensor.t.name = cfg.label;

    if (cfg.type == TemperatureType::BCL_PERCENTAGE ||
        cfg.type == TemperatureType::BCL_CURRENT ||
        cfg.type == TemperatureType::BCL_VOLTAGE) {
        sensor.mulFactor = 1;
    } else {
        sensor.mulFactor = 1000;
    }

    sensor.sensor_name = cfg.sensor_list[sens_idx];
    sensor.positiveThresh = cfg.positive_thresh_ramp;
    sensor.lastThrottleStatus = sensor.t.throttlingStatus =
        ThrottlingSeverity::NONE;
    sensor.thresh.type = sensor.t.type = cfg.type;

    sensor.thresh.hotThrottlingThresholds.resize(
        kThrottlingSeverityCount, UNKNOWN_TEMPERATURE);
    sensor.thresh.coldThrottlingThresholds.resize(
        kThrottlingSeverityCount, UNKNOWN_TEMPERATURE);

    for (idx = 0; idx < (int)kThrottlingSeverityCount; idx++) {
        sensor.thresh.hotThrottlingThresholds[idx] = UNKNOWN_TEMPERATURE;
        sensor.thresh.coldThrottlingThresholds[idx] = UNKNOWN_TEMPERATURE;
    }

    if (cfg.throt_thresh != 0 && cfg.positive_thresh_ramp)
        sensor.thresh.hotThrottlingThresholds[
            (size_t)ThrottlingSeverity::SEVERE] =
            cfg.throt_thresh / (float)sensor.mulFactor;
    else if (cfg.throt_thresh != 0 && !cfg.positive_thresh_ramp)
        sensor.thresh.coldThrottlingThresholds[
            (size_t)ThrottlingSeverity::SEVERE] =
            cfg.throt_thresh / (float)sensor.mulFactor;

    if (cfg.shutdwn_thresh != 0 && cfg.positive_thresh_ramp)
        sensor.thresh.hotThrottlingThresholds[
            (size_t)ThrottlingSeverity::SHUTDOWN] =
            cfg.shutdwn_thresh / (float)sensor.mulFactor;
    else if (cfg.shutdwn_thresh != 0 && !cfg.positive_thresh_ramp)
        sensor.thresh.coldThrottlingThresholds[
            (size_t)ThrottlingSeverity::SHUTDOWN] =
            cfg.shutdwn_thresh / (float)sensor.mulFactor;

    sens.push_back(sensor);

    LOG(INFO) << "ThermalHAL: Initialized sensor '" << sensor.t.name
              << "' (" << sensor.sensor_name << ") at tz" << sensor.tzn
              << " mulFactor=" << sensor.mulFactor;

    return 0;
}

int ThermalCommon::initializeCpuSensor(struct target_therm_cfg& cpu_cfg) {
    int cpu = 0;

    for (; cpu < ncpus; cpu++) {
        if (cpu >= (int)cpu_cfg.sensor_list.size()) {
            LOG(WARNING) << "ThermalHAL: Not enough CPU sensors for CPU"
                         << cpu;
            break;
        }
        if (initialize_sensor(cpu_cfg, cpu) < 0)
            return -1;
    }
    return 0;
}

int ThermalCommon::initThermalZones(
        std::vector<struct target_therm_cfg>& cfg) {
    if (cfg.empty()) {
        LOG(ERROR) << __func__ << ": Invalid input";
        return -1;
    }

    for (auto it = cfg.begin(); it != cfg.end(); it++) {
        if (it->type == TemperatureType::CPU) {
            if (initializeCpuSensor(*it) < 0)
                return -1;
            continue;
        }
        if (initialize_sensor(*it, 0) < 0) {
            LOG(WARNING) << "ThermalHAL: Failed to init sensor: "
                         << it->label << ", skipping";
            continue;
        }
    }

    LOG(INFO) << "ThermalHAL: Initialized " << sens.size()
              << " thermal sensors";
    return sens.size();
}

int ThermalCommon::initCdev() {
    DIR *tdir = NULL;
    struct dirent *tdirent = NULL;
    int cdevn = 0;
    char name[MAX_PATH] = {0};
    char cwd[MAX_PATH] = {0};
    int ret = 0;

    if (!getcwd(cwd, sizeof(cwd)))
        return 0;

    ret = chdir(THERMAL_SYSFS);
    if (ret) {
        LOG(ERROR) << "Unable to change to " << THERMAL_SYSFS;
        return 0;
    }
    tdir = opendir(THERMAL_SYSFS);
    if (!tdir) {
        LOG(ERROR) << "Unable to open " << THERMAL_SYSFS;
        return 0;
    }

    while ((tdirent = readdir(tdir))) {
        std::string buf;
        struct therm_cdev cdevInst;

        if (strncmp(tdirent->d_name, CDEV_DIR_NAME,
                    strlen(CDEV_DIR_NAME)) != 0)
            continue;

        snprintf(name, MAX_PATH, "%s%s/%s", THERMAL_SYSFS,
                 tdirent->d_name, TZ_TYPE);
        ret = readLineFromFile(std::string_view(name), buf);
        if (ret <= 0) {
            LOG(DEBUG) << "init_cdev: cdev type read error for cdev:"
                       << tdirent->d_name;
            continue;
        }
        auto it = cdev_map.find(buf);
        if (it == cdev_map.end()) {
            LOG(DEBUG) << "ThermalHAL: Unknown cooling device type: "
                       << buf;
            continue;
        }
        sscanf(tdirent->d_name, CDEV_DIR_FMT, &cdevn);
        LOG(DEBUG) << "cdev: " << it->first
                   << " found at cdev number: " << cdevn;

        cdevInst.c.name = it->first;
        cdevInst.c.type = it->second;
        cdevInst.cdevn = cdevn;
        read_cdev_state(cdevInst);
        cdev.push_back(cdevInst);
    }

    closedir(tdir);
    chdir(cwd);

    LOG(INFO) << "ThermalHAL: Initialized " << cdev.size()
              << " cooling devices";
    return cdev.size();
}

int ThermalCommon::read_cdev_state(struct therm_cdev& cdev) {
    char file_name[MAX_PATH];
    std::string buf;
    int ret = 0;

    LOG(DEBUG) << "Entering " << __func__;
    snprintf(file_name, sizeof(file_name), CDEV_CUR_STATE_PATH,
             cdev.cdevn);
    ret = readLineFromFile(std::string(file_name), buf);
    if (ret <= 0) {
        LOG(ERROR) << "Cdev state read error:" << ret
                   << " for cdev: " << cdev.c.name;
        return -1;
    }
    cdev.c.value = std::atoi(buf.c_str());
    LOG(DEBUG) << "cdev Name:" << cdev.c.name
               << " state:" << cdev.c.value;
    return cdev.c.value;
}

static int getHysteresis(TemperatureType type) {
    // BCL widths use the native units from the SDM660 thermal zones.
    switch (type) {
        case TemperatureType::BCL_CURRENT:
            return 200;  // ibat-high: mA
        case TemperatureType::BCL_VOLTAGE:
            return 100;  // vbat_adc: mV
        case TemperatureType::BCL_PERCENTAGE:
            return 0;    // soc: percentage points
        default:
            return DEFAULT_HYSTERESIS;  // milliCelsius
    }
}

int ThermalCommon::estimateSeverity(struct therm_sensor& sensor) {
    int idx = 0;
    ThrottlingSeverity severity = ThrottlingSeverity::NONE;
    float temp = sensor.t.value;
    const float hysteresis = getHysteresis(sensor.t.type) / (float)sensor.mulFactor;

    if (std::isnan(temp))
        return -1;

    for (idx = (int)ThrottlingSeverity::SHUTDOWN; idx >= 0; idx--) {
        if (idx == (int)sensor.t.throttlingStatus) {
            if ((sensor.positiveThresh &&
                 !isnan(sensor.thresh.hotThrottlingThresholds[idx]) &&
                 temp >= (sensor.thresh.hotThrottlingThresholds[idx] -
                          hysteresis)) ||
                (!sensor.positiveThresh &&
                 !isnan(sensor.thresh.coldThrottlingThresholds[idx]) &&
                 temp <= (sensor.thresh.coldThrottlingThresholds[idx] +
                          hysteresis)))
                break;
            continue;
        }
        if ((sensor.positiveThresh &&
             !isnan(sensor.thresh.hotThrottlingThresholds[idx]) &&
             temp >= sensor.thresh.hotThrottlingThresholds[idx]) ||
            (!sensor.positiveThresh &&
             !isnan(sensor.thresh.coldThrottlingThresholds[idx]) &&
             temp <= sensor.thresh.coldThrottlingThresholds[idx]))
            break;
    }

    if (idx >= 0)
        severity = (ThrottlingSeverity)(idx);

    LOG(DEBUG) << "Sensor Name:" << sensor.t.name
               << " prev severity:" << (int)sensor.lastThrottleStatus
               << " cur severity:" << (int)sensor.t.throttlingStatus
               << " New severity:" << (int)severity;

    if (severity == sensor.t.throttlingStatus)
        return -1;

    sensor.lastThrottleStatus = sensor.t.throttlingStatus;
    sensor.t.throttlingStatus = severity;
    return (int)severity;
}

int ThermalCommon::read_temperature(struct therm_sensor& sensor) {
    char file_name[MAX_PATH];
    std::string buf;
    int ret = 0;

    LOG(DEBUG) << "Entering " << __func__;
    snprintf(file_name, sizeof(file_name), TEMPERATURE_FILE_FORMAT,
             sensor.tzn);
    ret = readLineFromFile(std::string(file_name), buf);
    if (ret <= 0) {
        sensor.t.value = UNKNOWN_TEMPERATURE;
        LOG(ERROR) << "Temperature read error:" << ret
                   << " for sensor " << sensor.t.name;
        return -1;
    }
    int raw_temp = 0;
    const auto [end, error] = std::from_chars(buf.data(), buf.data() + buf.size(), raw_temp);
    if (error != std::errc() || end != buf.data() + buf.size()) {
        sensor.t.value = UNKNOWN_TEMPERATURE;
        LOG(ERROR) << "Invalid temperature reading for sensor " << sensor.t.name;
        return -EINVAL;
    }
    sensor.t.value = (float)raw_temp / (float)sensor.mulFactor;

    if (sensor.thresh.type == TemperatureType::CPU &&
        sensor.sensor_name == "cpuss-0-usr" &&
        sensor.t.name == "CPU0") {
        const int64_t temp_mC = std::atoll(buf.c_str());
        gCaps.update(temp_mC);
    }

    LOG(DEBUG) << "Sensor Name:" << sensor.t.name
               << " Temperature:" << sensor.t.value;
    return ret;
}

void ThermalCommon::initThreshold(struct therm_sensor& sensor) {
    char file_name[MAX_PATH] = "";
    std::string buf;
    int ret = 0;
    int idx;
    float next_trip;
    int curr_trip;
    int hyst_temp = 0;

    LOG(DEBUG) << "Entering " << __func__;

    if (!sensor.positiveThresh) {
        LOG(DEBUG) << "Negative temperature ramp for sensor:"
                   << sensor.t.name << ", skipping threshold init";
        return;
    }

    snprintf(file_name, sizeof(file_name), POLICY_FILE_FORMAT,
             sensor.tzn);
    ret = readLineFromFile(std::string(file_name), buf);
    if (ret <= 0) {
        LOG(DEBUG) << "Policy read error:" << ret
                   << " for sensor " << sensor.t.name;
        return;
    }
    if (buf != std::string(USER_SPACE_POLICY)) {
        LOG(DEBUG) << "Policy:" << buf << " for sensor:"
                   << sensor.t.name << " (not user_space, skipping)";
        return;
    }

    next_trip = UNKNOWN_TEMPERATURE;
    for (idx = 0; idx <= (int)ThrottlingSeverity::SHUTDOWN; idx++) {
        if (isnan(sensor.thresh.hotThrottlingThresholds[idx]) ||
            idx <= (int)sensor.t.throttlingStatus)
            continue;
        next_trip = sensor.thresh.hotThrottlingThresholds[idx] *
                    sensor.mulFactor;
        break;
    }

    if (!isnan(next_trip)) {
        LOG(DEBUG) << "Sensor: " << sensor.t.name
                   << " high trip:" << next_trip;
        snprintf(file_name, sizeof(file_name), TRIP_FILE_FORMAT,
                 sensor.tzn);
        writeToFile(std::string_view(file_name),
                    std::to_string(static_cast<int>(next_trip)));
    }

    hyst_temp = getHysteresis(sensor.t.type);
    if (sensor.t.throttlingStatus != ThrottlingSeverity::NONE &&
        !isnan(next_trip)) {
        if (isnan(sensor.thresh.hotThrottlingThresholds[
                      (int)sensor.t.throttlingStatus]))
            return;
        curr_trip = (int)(sensor.thresh.hotThrottlingThresholds[
                        (int)sensor.t.throttlingStatus] *
                    sensor.mulFactor);
        hyst_temp += next_trip - curr_trip;
    }
    LOG(DEBUG) << "Sensor: " << sensor.t.name
               << " hysteresis:" << hyst_temp;
    snprintf(file_name, sizeof(file_name), HYST_FILE_FORMAT,
             sensor.tzn);
    writeToFile(std::string_view(file_name),
                std::to_string(hyst_temp));
}

}  // namespace aidl::android::hardware::thermal::implementation
