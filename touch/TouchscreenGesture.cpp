/*
 * Copyright (C) 2024 The LineageOS Project
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

#define LOG_TAG "TouchscreenGestureService"

#include "TouchscreenGesture.h"

#include <android-base/file.h>
#include <android-base/logging.h>

#include <fstream>
#include <unordered_map>

namespace {

typedef struct {
    int32_t keycode;
    const char* name;
} GestureData;

// id -> info
const std::unordered_map<int32_t, GestureData> kGestureInfoMap = {
    {0, {748, "Gesture C"}},
    {1, {749, "Gesture e"}},
    {2, {750, "Gesture M"}},
    {3, {751, "Gesture O"}},
    {4, {752, "Gesture S"}},
    {5, {753, "Gesture V"}},
    {6, {754, "Gesture W"}},
    {7, {755, "Gesture Z"}},
    {8, {756, "Gesture Swipe Up"}},
    {9, {757, "Gesture Swipe Down"}},
    {10, {758, "Gesture Swipe Left"}},
    {11, {759, "Gesture Swipe Right"}},
};

static constexpr const char* kGestureNodePath = "/proc/tpd_gesture";

}  // anonymous namespace

namespace aidl {
namespace vendor {
namespace lineage {
namespace touch {

::ndk::ScopedAStatus TouchscreenGesture::getSupportedGestures(
        std::vector<Gesture>* _aidl_return) {
    for (const auto& entry : kGestureInfoMap) {
        Gesture gesture;
        gesture.id = entry.first;
        gesture.name = entry.second.name;
        gesture.keycode = entry.second.keycode;
        _aidl_return->push_back(gesture);
    }
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus TouchscreenGesture::setGestureEnabled(
        const Gesture& in_gesture, bool in_enabled) {
    const auto entry = kGestureInfoMap.find(in_gesture.id);
    if (entry == kGestureInfoMap.end()) {
        return ::ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    std::ofstream file(kGestureNodePath);
    file << (in_enabled ? "1" : "0");
    LOG(DEBUG) << "Wrote file " << kGestureNodePath << " fail " << file.fail();
    if (file.fail()) {
        return ::ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }
    return ::ndk::ScopedAStatus::ok();
}

}  // namespace touch
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
