#define LOG_TAG "fingerprint-asus"

#include "BiometricsFingerprint.h"

#include <android-base/logging.h>
#include <cutils/properties.h>
#include <hardware/hw_auth_token.h>
#include <log/log.h>
#include <endian.h>
#include <unistd.h>

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {

static const uint16_t kVersion = HARDWARE_MODULE_API_VERSION(2, 1);

// ─────────────────────────────────────────────
// FingerprintSession – static member definition
// ─────────────────────────────────────────────
FingerprintSession* FingerprintSession::sCurrentSession = nullptr;

// ─────────────────────────────────────────────
// FingerprintSession – constructor / destructor
// ─────────────────────────────────────────────
FingerprintSession::FingerprintSession(
        fingerprint_device_t* device,
        int32_t /*sensorId*/,
        int32_t userId,
        const std::shared_ptr<ISessionCallback>& cb)
    : mDevice(device), mUserId(userId), mCb(cb) {

    sCurrentSession = this;

    if (mDevice) {
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "/data/vendor_de/%d/fpdata", mUserId);
        // make sure the directory exists
        if (access(path, W_OK) != 0) {
            snprintf(path, sizeof(path), "/data/vendor/fps");
        }
        mDevice->set_active_group(mDevice, mUserId, path);
    }
}

FingerprintSession::~FingerprintSession() {
    if (sCurrentSession == this) {
        sCurrentSession = nullptr;
    }
}

// ─────────────────────────────────────────────
// Vendor filter helpers
// ─────────────────────────────────────────────
Error FingerprintSession::vendorErrorFilter(int32_t error, int32_t* vendorCode) {
    *vendorCode = 0;
    switch (error) {
        case FINGERPRINT_ERROR_HW_UNAVAILABLE:
            return Error::HW_UNAVAILABLE;
        case FINGERPRINT_ERROR_UNABLE_TO_PROCESS:
            return Error::UNABLE_TO_PROCESS;
        case FINGERPRINT_ERROR_TIMEOUT:
            return Error::TIMEOUT;
        case FINGERPRINT_ERROR_NO_SPACE:
            return Error::NO_SPACE;
        case FINGERPRINT_ERROR_CANCELED:
            return Error::CANCELED;
        case FINGERPRINT_ERROR_UNABLE_TO_REMOVE:
            return Error::UNABLE_TO_REMOVE;
        case FINGERPRINT_ERROR_LOCKOUT:
            return Error::VENDOR;   // handled separately via onLockoutTimed
        default:
            if (error >= FINGERPRINT_ERROR_VENDOR_BASE) {
                *vendorCode = error - FINGERPRINT_ERROR_VENDOR_BASE;
                return Error::VENDOR;
            }
            return Error::UNABLE_TO_PROCESS;
    }
}

AcquiredInfo FingerprintSession::vendorAcquiredFilter(int32_t info, int32_t* vendorCode) {
    *vendorCode = 0;
    switch (info) {
        case FINGERPRINT_ACQUIRED_GOOD:
            return AcquiredInfo::GOOD;
        case FINGERPRINT_ACQUIRED_PARTIAL:
            return AcquiredInfo::PARTIAL;
        case FINGERPRINT_ACQUIRED_INSUFFICIENT:
            return AcquiredInfo::INSUFFICIENT;
        case FINGERPRINT_ACQUIRED_IMAGER_DIRTY:
            return AcquiredInfo::SENSOR_DIRTY;
        case FINGERPRINT_ACQUIRED_TOO_SLOW:
            return AcquiredInfo::TOO_SLOW;
        case FINGERPRINT_ACQUIRED_TOO_FAST:
            return AcquiredInfo::TOO_FAST;
        default:
            if (info >= FINGERPRINT_ACQUIRED_VENDOR_BASE) {
                *vendorCode = info - FINGERPRINT_ACQUIRED_VENDOR_BASE;
                return AcquiredInfo::VENDOR;
            }
            return AcquiredInfo::INSUFFICIENT;
    }
}

// ─────────────────────────────────────────────
// ISession implementation
// ─────────────────────────────────────────────
ndk::ScopedAStatus FingerprintSession::generateChallenge() {
    ALOGD("generateChallenge");
    if (mDevice) {
        uint64_t challenge = mDevice->pre_enroll(mDevice);
        mCb->onChallengeGenerated(static_cast<int64_t>(challenge));
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus FingerprintSession::revokeChallenge(int64_t challenge) {
    ALOGD("revokeChallenge");
    if (mDevice) {
        mDevice->post_enroll(mDevice);
    }
    mCb->onChallengeRevoked(challenge);
    return ndk::ScopedAStatus::ok();
}

static void convertHAT(const HardwareAuthToken& in, hw_auth_token_t* out) {
    memset(out, 0, sizeof(*out));
    out->version          = 0;
    out->challenge        = static_cast<uint64_t>(in.challenge);
    out->user_id          = static_cast<uint64_t>(in.userId);
    out->authenticator_id = static_cast<uint64_t>(in.authenticatorId);
    out->authenticator_type =
        htobe32(static_cast<uint32_t>(in.authenticatorType));
    out->timestamp = htobe64(static_cast<uint64_t>(in.timestamp.milliSeconds));
    if (in.mac.size() <= sizeof(out->hmac)) {
        memcpy(out->hmac, in.mac.data(), in.mac.size());
    }
}

ndk::ScopedAStatus FingerprintSession::enroll(
        const HardwareAuthToken& hat,
        std::shared_ptr<ICancellationSignal>* out) {
    ALOGD("enroll");
    if (mDevice) {
        hw_auth_token_t token;
        convertHAT(hat, &token);
        mDevice->enroll(mDevice, &token, mUserId, 60);
    }
    *out = ndk::SharedRefBase::make<CancellationSignal>(mDevice);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus FingerprintSession::authenticate(
        int64_t operationId,
        std::shared_ptr<ICancellationSignal>* out) {
    ALOGD("authenticate(operationId=%lld)", (long long)operationId);  // исправлено
    if (mDevice) {
        mDevice->authenticate(mDevice, static_cast<uint64_t>(operationId), mUserId);
    }
    *out = ndk::SharedRefBase::make<CancellationSignal>(mDevice);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus FingerprintSession::detectInteraction(
        std::shared_ptr<ICancellationSignal>* out) {
    ALOGD("detectInteraction");
    if (mDevice) {
        mDevice->authenticate(mDevice, 0, mUserId);
    }
    *out = ndk::SharedRefBase::make<CancellationSignal>(mDevice);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus FingerprintSession::enumerateEnrollments() {
    ALOGD("enumerateEnrollments");
    if (mDevice) {
        mDevice->enumerate(mDevice);
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus FingerprintSession::removeEnrollments(
        const std::vector<int32_t>& enrollmentIds) {
    ALOGD("removeEnrollments");
    if (mDevice) {
        if (enrollmentIds.empty()) {
            mDevice->remove(mDevice, mUserId, 0);
        } else {
            for (int32_t id : enrollmentIds) {
                mDevice->remove(mDevice, mUserId, static_cast<uint32_t>(id));
            }
        }
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus FingerprintSession::getAuthenticatorId() {
    ALOGD("getAuthenticatorId");
    uint64_t id = mDevice ? mDevice->get_authenticator_id(mDevice) : 0;
    mCb->onAuthenticatorIdRetrieved(static_cast<int64_t>(id));
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus FingerprintSession::invalidateAuthenticatorId() {
    ALOGD("invalidateAuthenticatorId");
    uint64_t id = mDevice ? mDevice->get_authenticator_id(mDevice) : 0;
    mCb->onAuthenticatorIdInvalidated(static_cast<int64_t>(id));
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus FingerprintSession::resetLockout(
        const HardwareAuthToken& /*hat*/) {
    ALOGD("resetLockout");
    mCb->onLockoutCleared();
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus FingerprintSession::close() {
    ALOGD("close");
    mCb->onSessionClosed();
    return ndk::ScopedAStatus::ok();
}

// Pointer / UI – stubs (no-op for rear sensor)
ndk::ScopedAStatus FingerprintSession::onPointerDown(
        int32_t, int32_t, int32_t, float, float) {
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus FingerprintSession::onPointerUp(int32_t) {
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus FingerprintSession::onUiReady() {
    return ndk::ScopedAStatus::ok();
}

// WithContext – delegate to base methods
ndk::ScopedAStatus FingerprintSession::authenticateWithContext(
        int64_t operationId,
        const OperationContext& /*context*/,
        std::shared_ptr<ICancellationSignal>* out) {
    return authenticate(operationId, out);
}

ndk::ScopedAStatus FingerprintSession::enrollWithContext(
        const HardwareAuthToken& hat,
        const OperationContext& /*context*/,
        std::shared_ptr<ICancellationSignal>* out) {
    return enroll(hat, out);
}

ndk::ScopedAStatus FingerprintSession::detectInteractionWithContext(
        const OperationContext& /*context*/,
        std::shared_ptr<ICancellationSignal>* out) {
    return detectInteraction(out);
}

ndk::ScopedAStatus FingerprintSession::onPointerDownWithContext(
        const PointerContext& /*context*/) {
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus FingerprintSession::onPointerUpWithContext(
        const PointerContext& /*context*/) {
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus FingerprintSession::onPointerCancelWithContext(
        const PointerContext& /*context*/) {
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus FingerprintSession::onContextChanged(
        const OperationContext& /*context*/) {
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus FingerprintSession::setIgnoreDisplayTouches(bool) {
    return ndk::ScopedAStatus::ok();
}

// ─────────────────────────────────────────────
// Legacy HAL callback
// ─────────────────────────────────────────────
void FingerprintSession::legacyNotify(const fingerprint_msg_t* msg) {
    if (!sCurrentSession) {
        ALOGE("legacyNotify: no active session");
        return;
    }

    FingerprintSession* self = sCurrentSession;
    std::lock_guard<std::mutex> lock(self->mMutex);

    if (!self->mCb) {
        ALOGE("legacyNotify: callback is null");
        return;
    }

    switch (msg->type) {

        case FINGERPRINT_ERROR: {
            int32_t vendorCode = 0;
            int32_t error = msg->data.error;

            // Lockout – убран PERMANENT, его нет в legacy HAL sdm660
            if (error == FINGERPRINT_ERROR_LOCKOUT) {
                self->mCb->onLockoutTimed(5000);
                break;
            }

            Error result = vendorErrorFilter(error, &vendorCode);
            ALOGD("onError(%d, vendorCode=%d)", (int)result, vendorCode);
            self->mCb->onError(result, vendorCode);
            break;
        }

        case FINGERPRINT_ACQUIRED: {
            int32_t vendorCode = 0;
            AcquiredInfo result =
                vendorAcquiredFilter(msg->data.acquired.acquired_info, &vendorCode);
            // These private CDFinger status codes have no framework help text.
            if (result == AcquiredInfo::VENDOR &&
                (vendorCode == 102 || vendorCode == 1001)) {
                ALOGD("Ignoring CDFinger acquired status %d", vendorCode);
                break;
            }
            ALOGD("onAcquired(%d, vendorCode=%d)", result, vendorCode);
            self->mCb->onAcquired(result, vendorCode);
            break;
        }

        case FINGERPRINT_TEMPLATE_ENROLLING: {
            int32_t fid = msg->data.enroll.finger.fid;
            int32_t rem = static_cast<int32_t>(msg->data.enroll.samples_remaining);
            ALOGD("onEnrollmentProgress(fid=%d, remaining=%d)", fid, rem);
            self->mCb->onEnrollmentProgress(fid, rem);
            break;
        }

        case FINGERPRINT_TEMPLATE_REMOVED: {
            int32_t fid       = msg->data.removed.finger.fid;
            int32_t remaining = static_cast<int32_t>(msg->data.removed.remaining_templates);
            ALOGD("onRemoved(fid=%d, remaining=%d)", fid, remaining);
            std::vector<int32_t> ids;
            if (fid != 0) ids.push_back(fid);
            self->mCb->onEnrollmentsRemoved(ids);
            break;
        }

        case FINGERPRINT_AUTHENTICATED: {
            int32_t fid = msg->data.authenticated.finger.fid;
            ALOGD("onAuthenticated(fid=%d)", fid);

            if (fid != 0) {
                const hw_auth_token_t* hwToken = &msg->data.authenticated.hat;
                HardwareAuthToken token;
                token.challenge       = static_cast<int64_t>(hwToken->challenge);
                token.userId          = static_cast<int64_t>(hwToken->user_id);
                token.authenticatorId = static_cast<int64_t>(hwToken->authenticator_id);
                token.authenticatorType =
                    static_cast<::aidl::android::hardware::keymaster::HardwareAuthenticatorType>(
                        be32toh(hwToken->authenticator_type));
                token.timestamp.milliSeconds =
                    static_cast<int64_t>(be64toh(hwToken->timestamp));
                token.mac.assign(hwToken->hmac,
                                 hwToken->hmac + sizeof(hwToken->hmac));

                self->mCb->onAuthenticationSucceeded(fid, token);
            } else {
                self->mCb->onAuthenticationFailed();
            }
            break;
        }

        case FINGERPRINT_TEMPLATE_ENUMERATING: {
            int32_t fid       = msg->data.enumerated.finger.fid;
            int32_t remaining = static_cast<int32_t>(
                msg->data.enumerated.remaining_templates);
            ALOGD("onEnumerate(fid=%d, remaining=%d)", fid, remaining);

            // Accumulate and fire when done
            static std::vector<int32_t> enumIds;
            if (fid != 0) enumIds.push_back(fid);
            if (remaining == 0) {
                self->mCb->onEnrollmentsEnumerated(enumIds);
                enumIds.clear();
            }
            break;
        }

        default:
            ALOGW("legacyNotify: unknown msg type %d", msg->type);
            break;
    }
}

// ─────────────────────────────────────────────
// BiometricsFingerprint
// ─────────────────────────────────────────────
BiometricsFingerprint::BiometricsFingerprint()
    : mDevice(nullptr) {
    mDevice = openHal();
    if (!mDevice) {
        ALOGE("Can't open HAL module");
    }
}

BiometricsFingerprint::~BiometricsFingerprint() {
    if (mDevice) {
        mDevice->common.close(
            reinterpret_cast<hw_device_t*>(mDevice));
        mDevice = nullptr;
    }
}

std::shared_ptr<BiometricsFingerprint> BiometricsFingerprint::create() {
    return ndk::SharedRefBase::make<BiometricsFingerprint>();
}

ndk::ScopedAStatus BiometricsFingerprint::getSensorProps(
        std::vector<SensorProps>* props) {
    SensorProps p;
    p.commonProps.sensorId              = 0;
    p.commonProps.sensorStrength        = common::SensorStrength::STRONG;
    p.commonProps.maxEnrollmentsPerUser = 5;
    p.commonProps.componentInfo         = {};
    p.sensorType                        = FingerprintSensorType::REAR;
    p.halControlsIllumination           = false;
    // displaySupportsAod убран - нет в V3 SensorProps
    props->push_back(std::move(p));
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus BiometricsFingerprint::createSession(
        int32_t sensorId, int32_t userId,
        const std::shared_ptr<ISessionCallback>& cb,
        std::shared_ptr<ISession>* out) {
    ALOGD("createSession(sensorId=%d, userId=%d)", sensorId, userId);

    std::lock_guard<std::mutex> lock(mSessionMutex);

    auto session = ndk::SharedRefBase::make<FingerprintSession>(
        mDevice, sensorId, userId, cb);

    *out = session;
    return ndk::ScopedAStatus::ok();
}

fingerprint_device_t* BiometricsFingerprint::openHal() {
    struct Candidate { const char* id; const char* vendor; };
    static const Candidate candidates[] = {
        {"fingerprint",           "goodix"},
        {"cdfinger.fingerprint",  "cdfinger"},
        {"fingerprint.focaltech", "focaltech"},
    };

    for (const auto& c : candidates) {
        ALOGD("Trying HAL: %s (%s)", c.id, c.vendor);

        const hw_module_t* hw_mdl = nullptr;
        int err = hw_get_module(c.id, &hw_mdl);
        if (err || !hw_mdl) {
            ALOGW("hw_get_module(%s) failed: %d", c.id, err);
            continue;
        }

        const auto* mod =
            reinterpret_cast<const fingerprint_module_t*>(hw_mdl);
        if (!mod->common.methods || !mod->common.methods->open) {
            ALOGW("%s: no open method", c.id);
            continue;
        }

        hw_device_t* device = nullptr;
        err = mod->common.methods->open(hw_mdl, nullptr, &device);
        if (err || !device) {
            ALOGW("%s: open() failed: %d", c.id, err);
            continue;
        }

        if (kVersion != device->version) {
            ALOGE("%s: version mismatch: expected %d, got %d",
                  c.id, kVersion, device->version);
            device->close(device);
            continue;
        }

        auto* fp = reinterpret_cast<fingerprint_device_t*>(device);

        err = fp->set_notify(fp, FingerprintSession::legacyNotify);
        if (err) {
            ALOGE("%s: set_notify failed: %d", c.id, err);
            device->close(device);
            continue;
        }

        ALOGI("Opened fingerprint HAL: %s", c.vendor);
        property_set("persist.vendor.runin.fp", c.vendor);
        return fp;
    }

    ALOGE("No fingerprint HAL found");
    property_set("persist.vendor.runin.fp", "unknown");
    return nullptr;
}

}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
