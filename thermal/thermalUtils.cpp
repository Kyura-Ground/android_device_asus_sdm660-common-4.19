/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <android-base/logging.h>

#include "thermalUtils.h"

namespace aidl::android::hardware::thermal::implementation {

ThermalUtils::ThermalUtils(const ueventCB &inp_cb)
    : cfg(),
      cmnInst(),
      monitor(std::bind(&ThermalUtils::ueventParse, this,
                         std::placeholders::_1,
                         std::placeholders::_2)),
      cb(inp_cb) {
    int ret = 0;
    std::vector<struct therm_sensor> sensorList;
    std::vector<struct target_therm_cfg> therm_cfg = cfg.fetchConfig();

    is_sensor_init = false;
    is_cdev_init = false;

    ret = cmnInst.initThermalZones(therm_cfg);
    if (ret > 0) {
        is_sensor_init = true;
        sensorList = cmnInst.fetch_sensor_list();
        std::lock_guard<std::mutex> _lock(sens_cb_mutex);
        for (const struct therm_sensor& sensor : sensorList) {
            struct therm_sensor& sens = thermalConfig[sensor.sensor_name];
            sens = sensor;
            if (cmnInst.read_temperature(sens) < 0)
                continue;
            cmnInst.estimateSeverity(sens);
            cmnInst.initThreshold(sens);
        }
        monitor.start();
    }
    ret = cmnInst.initCdev();
    if (ret > 0) {
        is_cdev_init = true;
        cdevList = cmnInst.fetch_cdev_list();
    }
}

void ThermalUtils::Notify(struct therm_sensor& sens) {
    int severity = cmnInst.estimateSeverity(sens);
    if (severity != -1) {
        LOG(INFO) << "sensor: " << sens.sensor_name
                  << " temperature: " << sens.t.value
                  << " old: " << (int)sens.lastThrottleStatus
                  << " new: " << (int)sens.t.throttlingStatus;
        cb(sens.t);
        cmnInst.initThreshold(sens);
    }
}

void ThermalUtils::ueventParse(std::string sensor_name, int temp) {
    LOG(INFO) << "uevent triggered for sensor: " << sensor_name;
    if (thermalConfig.find(sensor_name) == thermalConfig.end()) {
        LOG(DEBUG) << "sensor is not monitored:" << sensor_name;
        return;
    }
    std::lock_guard<std::mutex> _lock(sens_cb_mutex);
    struct therm_sensor& sens = thermalConfig[sensor_name];
    sens.t.value = (float)temp / (float)sens.mulFactor;
    return Notify(sens);
}

int ThermalUtils::readTemperatures(bool filterType, TemperatureType type,
                                    std::vector<Temperature>& temp) {
    int ret = 0;

    std::lock_guard<std::mutex> _lock(sens_cb_mutex);
    for (auto it = thermalConfig.begin(); it != thermalConfig.end(); it++) {
        struct therm_sensor& sens = it->second;

        if (filterType && sens.t.type != type)
            continue;
        ret = cmnInst.read_temperature(sens);
        if (ret >= 0)
            Notify(sens);
        // AIDL keeps offline sensors in the list with an unavailable (NaN) value.
        temp.push_back(sens.t);
    }

    return temp.size();
}

int ThermalUtils::readTemperatureThreshold(bool filterType, TemperatureType type,
                                            std::vector<TemperatureThreshold>& thresh) {
    for (auto it = thermalConfig.begin(); it != thermalConfig.end(); it++) {
        struct therm_sensor& sens = it->second;

        if (filterType && sens.t.type != type)
            continue;
        thresh.push_back(sens.thresh);
    }

    return thresh.size();
}

int ThermalUtils::readCdevStates(bool filterType, CoolingType type,
                                  std::vector<CoolingDevice>& cdev_out) {
    int ret = 0;

    for (struct therm_cdev cdev : cdevList) {
        if (filterType && cdev.c.type != type)
            continue;
        ret = cmnInst.read_cdev_state(cdev);
        if (ret < 0)
            return ret;
        cdev_out.push_back(cdev.c);
    }

    return cdev_out.size();
}

}  // namespace aidl::android::hardware::thermal::implementation
