/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef VTS_KEYMINT_AIDL_TEST_UTILS_H
#define VTS_KEYMINT_AIDL_TEST_UTILS_H

#pragma once

#include <aidl/Gtest.h>
#include <aidl/Vintf.h>
#include <android/hardware/keymint/ErrorCode.h>
#include <android/hardware/keymint/IKeyMintDevice.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <gtest/gtest.h>

#include <keymintSupport/authorization_set.h>

namespace android {
namespace hardware {
namespace keymint {
namespace test {

using ::android::sp;
using binder::Status;
using ::std::shared_ptr;
using ::std::string;
using ::std::vector;

constexpr uint64_t kOpHandleSentinel = 0xFFFFFFFFFFFFFFFF;

::std::ostream& operator<<(::std::ostream& os, const AuthorizationSet& set);

class KeyMintAidlTestBase : public ::testing::TestWithParam<string> {
  public:
    void SetUp() override;
    void TearDown() override {
        if (key_blob_.size()) {
            CheckedDeleteKey();
        }
        AbortIfNeeded();
    }

    void InitializeKeyMint(sp<IKeyMintDevice> keyMint);
    IKeyMintDevice& keyMint() { return *keymint_; }
    uint32_t os_version() { return os_version_; }
    uint32_t os_patch_level() { return os_patch_level_; }

    ErrorCode GetReturnErrorCode(Status result);
    ErrorCode GenerateKey(const AuthorizationSet& key_desc, vector<uint8_t>* key_blob,
                          KeyCharacteristics* key_characteristics);

    ErrorCode GenerateKey(const AuthorizationSet& key_desc);

    ErrorCode ImportKey(const AuthorizationSet& key_desc, KeyFormat format,
                        const string& key_material, vector<uint8_t>* key_blob,
                        KeyCharacteristics* key_characteristics);
    ErrorCode ImportKey(const AuthorizationSet& key_desc, KeyFormat format,
                        const string& key_material);

    ErrorCode ImportWrappedKey(string wrapped_key, string wrapping_key,
                               const AuthorizationSet& wrapping_key_desc, string masking_key,
                               const AuthorizationSet& unwrapping_params);

    ErrorCode DeleteKey(vector<uint8_t>* key_blob, bool keep_key_blob = false);
    ErrorCode DeleteKey(bool keep_key_blob = false);

    ErrorCode DeleteAllKeys();

    void CheckedDeleteKey(vector<uint8_t>* key_blob, bool keep_key_blob = false);
    void CheckedDeleteKey();

    ErrorCode Begin(KeyPurpose purpose, const vector<uint8_t>& key_blob,
                    const AuthorizationSet& in_params, AuthorizationSet* out_params,
                    sp<IKeyMintOperation>& op);
    ErrorCode Begin(KeyPurpose purpose, const vector<uint8_t>& key_blob,
                    const AuthorizationSet& in_params, AuthorizationSet* out_params);
    ErrorCode Begin(KeyPurpose purpose, const AuthorizationSet& in_params,
                    AuthorizationSet* out_params);
    ErrorCode Begin(KeyPurpose purpose, const AuthorizationSet& in_params);

    ErrorCode Update(const AuthorizationSet& in_params, const string& input,
                     AuthorizationSet* out_params, string* output, int32_t* input_consumed);
    ErrorCode Update(const string& input, string* out, int32_t* input_consumed);

    ErrorCode Finish(const AuthorizationSet& in_params, const string& input,
                     const string& signature, AuthorizationSet* out_params, string* output);
    ErrorCode Finish(const string& message, string* output);
    ErrorCode Finish(const string& message, const string& signature, string* output);
    ErrorCode Finish(string* output) { return Finish(string(), output); }

    ErrorCode Abort();
    ErrorCode Abort(const sp<IKeyMintOperation>& op);
    void AbortIfNeeded();

    string ProcessMessage(const vector<uint8_t>& key_blob, KeyPurpose operation,
                          const string& message, const AuthorizationSet& in_params,
                          AuthorizationSet* out_params);

    string SignMessage(const vector<uint8_t>& key_blob, const string& message,
                       const AuthorizationSet& params);
    string SignMessage(const string& message, const AuthorizationSet& params);

    string MacMessage(const string& message, Digest digest, size_t mac_length);

    void CheckHmacTestVector(const string& key, const string& message, Digest digest,
                             const string& expected_mac);

    void CheckAesCtrTestVector(const string& key, const string& nonce, const string& message,
                               const string& expected_ciphertext);

    void CheckTripleDesTestVector(KeyPurpose purpose, BlockMode block_mode,
                                  PaddingMode padding_mode, const string& key, const string& iv,
                                  const string& input, const string& expected_output);

    void VerifyMessage(const vector<uint8_t>& key_blob, const string& message,
                       const string& signature, const AuthorizationSet& params);
    void VerifyMessage(const string& message, const string& signature,
                       const AuthorizationSet& params);

    string EncryptMessage(const vector<uint8_t>& key_blob, const string& message,
                          const AuthorizationSet& in_params, AuthorizationSet* out_params);
    string EncryptMessage(const string& message, const AuthorizationSet& params,
                          AuthorizationSet* out_params);
    string EncryptMessage(const string& message, const AuthorizationSet& params);
    string EncryptMessage(const string& message, BlockMode block_mode, PaddingMode padding);
    string EncryptMessage(const string& message, BlockMode block_mode, PaddingMode padding,
                          vector<uint8_t>* iv_out);
    string EncryptMessage(const string& message, BlockMode block_mode, PaddingMode padding,
                          const vector<uint8_t>& iv_in);
    string EncryptMessage(const string& message, BlockMode block_mode, PaddingMode padding,
                          uint8_t mac_length_bits, const vector<uint8_t>& iv_in);

    string DecryptMessage(const vector<uint8_t>& key_blob, const string& ciphertext,
                          const AuthorizationSet& params);
    string DecryptMessage(const string& ciphertext, const AuthorizationSet& params);
    string DecryptMessage(const string& ciphertext, BlockMode block_mode, PaddingMode padding_mode,
                          const vector<uint8_t>& iv);

    std::pair<ErrorCode, vector<uint8_t>> UpgradeKey(const vector<uint8_t>& key_blob);

    bool IsSecure() { return securityLevel_ != SecurityLevel::SOFTWARE; }
    SecurityLevel SecLevel() { return securityLevel_; }

    vector<uint32_t> ValidKeySizes(Algorithm algorithm);
    vector<uint32_t> InvalidKeySizes(Algorithm algorithm);

    vector<EcCurve> ValidCurves();
    vector<EcCurve> InvalidCurves();

    vector<Digest> ValidDigests(bool withNone, bool withMD5);

    static vector<string> build_params() {
        auto params = android::getAidlHalInstanceNames(IKeyMintDevice::descriptor);
        return params;
    }

    sp<IKeyMintOperation> op_;
    vector<Certificate> certChain_;
    vector<uint8_t> key_blob_;
    KeyCharacteristics key_characteristics_;

  private:
    sp<IKeyMintDevice> keymint_;
    uint32_t os_version_;
    uint32_t os_patch_level_;

    SecurityLevel securityLevel_;
    string name_;
    string author_;
    long challenge_;
};

#define INSTANTIATE_KEYMINT_AIDL_TEST(name)                                          \
    INSTANTIATE_TEST_SUITE_P(PerInstance, name,                                      \
                             testing::ValuesIn(KeyMintAidlTestBase::build_params()), \
                             android::PrintInstanceNameToString)

}  // namespace test
}  // namespace keymint
}  // namespace hardware
}  // namespace android

#endif  // VTS_KEYMINT_AIDL_TEST_UTILS_H