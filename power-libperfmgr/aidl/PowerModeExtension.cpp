/*
 * Copyright (C) 2020 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <aidl/android/hardware/power/BnPower.h>
#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>

namespace aidl {
namespace google {
namespace hardware {
namespace power {
namespace impl {
namespace pixel {

using ::aidl::android::hardware::power::Mode;

constexpr char kTapToWakeNode[] = "/proc/tpd_gesture";
constexpr char kTapToWakeProp[] = "persist.vendor.dt2w.enabled";

bool isDeviceSpecificModeSupported(Mode type, bool* _aidl_return) {
    if (type == Mode::DOUBLE_TAP_TO_WAKE) {
        *_aidl_return = true;
        return true;
    }
    return false;
}

bool setDeviceSpecificMode(Mode type, bool enabled) {
    if (type == Mode::DOUBLE_TAP_TO_WAKE) {
        bool success =
                ::android::base::WriteStringToFile(enabled ? "1" : "0", kTapToWakeNode);
        if (!success) {
            PLOG(ERROR) << "Failed to write tap-to-wake node: " << kTapToWakeNode;
        }

        if (!::android::base::SetProperty(kTapToWakeProp, enabled ? "1" : "0")) {
            LOG(ERROR) << "Failed to set property: " << kTapToWakeProp;
        }

        return true;
    }
    return false;
}

void restoreDeviceSpecificState() {
    std::string val = ::android::base::GetProperty(kTapToWakeProp, "0");
    bool enabled = (val == "1");

    LOG(INFO) << "Restoring DT2W state after reboot: "
              << (enabled ? "enabled" : "disabled");

    bool success =
            ::android::base::WriteStringToFile(enabled ? "1" : "0", kTapToWakeNode);
    if (!success) {
        PLOG(ERROR) << "Failed to restore tap-to-wake node: " << kTapToWakeNode;
    }
}

}  // namespace pixel
}  // namespace impl
}  // namespace power
}  // namespace hardware
}  // namespace google
}  // namespace aidl
