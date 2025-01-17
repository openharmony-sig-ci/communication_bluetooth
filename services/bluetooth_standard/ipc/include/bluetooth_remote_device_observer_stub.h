/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_BLUETOOTH_STANDARD_REMOTE_DEVICE_OBSERVER_STUB_H
#define OHOS_BLUETOOTH_STANDARD_REMOTE_DEVICE_OBSERVER_STUB_H

#include "i_bluetooth_host.h"
#include "i_bluetooth_remote_device_observer.h"
#include "iremote_stub.h"
#include "map"

namespace OHOS {
namespace Bluetooth {

class BluetoothRemoteDeviceObserverstub : public IRemoteStub<IBluetoothRemoteDeviceOberver> {
public:
    explicit BluetoothRemoteDeviceObserverstub();
    ~BluetoothRemoteDeviceObserverstub() override;

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    ErrCode OnPairStatusChangedInner(MessageParcel &data, MessageParcel &reply);
    ErrCode OnRemoteNameUuidChangedInner(MessageParcel &data, MessageParcel &reply);
    ErrCode OnRemoteNameChangedInner(MessageParcel &data, MessageParcel &reply);
    ErrCode OnRemoteAliasChangedInner(MessageParcel &data, MessageParcel &reply);
    ErrCode OnRemoteCodChangedInner(MessageParcel &data, MessageParcel &reply);
    ErrCode OnRemoteBatteryLevelChangedInner(MessageParcel &data, MessageParcel &reply);
    static std::map<uint32_t, ErrCode (BluetoothRemoteDeviceObserverstub::*)(MessageParcel &data, MessageParcel &reply)>
        memberFuncMap_;
    DISALLOW_COPY_AND_MOVE(BluetoothRemoteDeviceObserverstub);
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_REMOTE_DEVICE_OBSERVER_STUB_H