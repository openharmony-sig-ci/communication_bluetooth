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

#ifndef OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_INTERFACE_H
#define OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_INTERFACE_H

#include "../parcel/bluetooth_ble_scan_settings.h"
#include "i_bluetooth_ble_central_manager_callback.h"
#include "iremote_broker.h"

namespace OHOS {
namespace Bluetooth {
class IBluetoothBleCentralManager : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetootBleCentralManager");

    enum Code {
        BLE_REGISTER_BLE_CENTRAL_MANAGER_CALLBACK = 0,
        BLE_DE_REGISTER_BLE_CENTRAL_MANAGER_CALLBACK,
        BLE_START_SCAN,
        BLE_START_SCAN_WITH_SETTINGS,
        BLE_STOP_SCAN,
    };

    virtual void RegisterBleCentralManagerCallback(const sptr<IBluetoothBleCentralManageCallback> &callback) = 0;
    virtual void DeregisterBleCentralManagerCallback(const sptr<IBluetoothBleCentralManageCallback> &callback) = 0;
    virtual void StartScan() = 0;
    virtual void StartScan(const BluetoothBleScanSettings &settings) = 0;
    virtual void StopScan() = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_INTERFACE_H