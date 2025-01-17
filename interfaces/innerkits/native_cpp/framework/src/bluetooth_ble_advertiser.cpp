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
#include "bluetooth_ble_advertiser.h"

#include "bluetooth_ble_advertise_callback_stub.h"
#include "bluetooth_def.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_map.h"
#include "i_bluetooth_ble_advertiser.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include <memory>

namespace OHOS {
namespace Bluetooth {

using namespace bluetooth;

struct BleAdvertiser::impl {
    impl();
    ~impl();

    class BluetoothBleAdvertiserCallbackImp;
    sptr<BluetoothBleAdvertiserCallbackImp> callbackImp_ = nullptr;

    BluetoothObserverMap<BleAdvertiseCallback> callbacks_ {};
    sptr<IBluetoothBleAdvertiser> proxy_ = nullptr;
};

class BleAdvertiser::impl::BluetoothBleAdvertiserCallbackImp : public BluetoothBleAdvertiseCallbackStub {
public:
    BluetoothBleAdvertiserCallbackImp(BleAdvertiser::impl &bleAdvertiser) : bleAdvertiser_(bleAdvertiser){};
    ~BluetoothBleAdvertiserCallbackImp()
    {}

    void OnStartResultEvent(int32_t result, int32_t advHandle, int32_t opcode) override
    {
        HILOGD("BleAdvertiser::impl::BluetoothBleAdvertiserCallbackImp::OnStartResultEvent");
        bleAdvertiser_.callbacks_.ForEach(
            [result, advHandle](int32_t handle, BleAdvertiseCallback *observer) {
                if (advHandle == handle) {
                    observer->OnStartResultEvent(result);
                }
            },
            advHandle);
        if ((opcode == BLE_ADV_STOP_COMPLETE_OP_CODE && result == 0) || opcode == BLE_ADV_START_FAILED_OP_CODE) {
            BleAdvertiseCallback *observer = bleAdvertiser_.callbacks_.GetAdvertiserObserver(advHandle);
            if (observer != nullptr) {
                bleAdvertiser_.callbacks_.Deregister(observer);
            }
        }
    }

    void OnAutoStopAdvEvent(int32_t advHandle) override
    {
        HILOGD("BleAdvertiser::impl::BluetoothBleAdvertiserCallbackImp::OnAutoStopAdvEvent advHandle");
        BleAdvertiseCallback *observer = bleAdvertiser_.callbacks_.GetAdvertiserObserver(advHandle);
        if (observer != nullptr) {
            bleAdvertiser_.callbacks_.Deregister(observer);
        }
    }

private:
    BleAdvertiser::impl &bleAdvertiser_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothBleAdvertiserCallbackImp);
};

BleAdvertiser::impl::impl()
{
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!samgr) {
        HILOGE("BleAdvertiser::impl::impl() error: no samgr");
        return;
    }

    sptr<IRemoteObject> hostRemote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    if (!hostRemote) {
        HILOGE("BleAdvertiser::impl::impl() error: no remote");
        return;
    }
    sptr<IBluetoothHost> hostProxy = iface_cast<IBluetoothHost>(hostRemote);
    if (!hostProxy) {
        HILOGE("BleAdvertiser::impl::impl() error: host no proxy");
        return;
    }
    sptr<IRemoteObject> remote = hostProxy->GetBleRemote(BLE_ADVERTISER_SERVER);
    if (!remote) {
        HILOGE("BleAdvertiser::impl::impl() error: no remote");
        return;
    }
    proxy_ = iface_cast<IBluetoothBleAdvertiser>(remote);
    if (!proxy_) {
        HILOGE("BleAdvertiser::impl::impl() error: no proxy");
        return;
    }

    callbackImp_ = new BluetoothBleAdvertiserCallbackImp(*this);
    proxy_->RegisterBleAdvertiserCallback(callbackImp_);
}

BleAdvertiser::impl::~impl()
{
    proxy_->DeregisterBleAdvertiserCallback(callbackImp_);
}

BleAdvertiser::BleAdvertiser() : pimpl(nullptr)
{
    if (pimpl == nullptr) {
        pimpl = std::make_unique<impl>();
        if (!pimpl) {
            HILOGE("BleAdvertiser::BleAdvertiser fails: no pimpl");
        }
    }

    HILOGD("BleAdvertiser::BleAdvertiser success");
}

BleAdvertiser::~BleAdvertiser()
{}

void BleAdvertiser::StartAdvertising(const BleAdvertiserSettings &settings, const BleAdvertiserData &advData,
    const BleAdvertiserData &scanResponse, BleAdvertiseCallback &callback)
{
    HILOGD("BleAdvertiser::StartAdvertising");
    if (pimpl->proxy_ != nullptr) {
        BluetoothBleAdvertiserSettings setting;
        setting.SetConnectable(settings.IsConnectable());
        setting.SetInterval(settings.GetInterval());
        setting.SetLegacyMode(settings.IsLegacyMode());
        setting.SetTxPower(settings.GetTxPower());

        BluetoothBleAdvertiserData bleAdvertiserData;
        bleAdvertiserData.SetAdvFlag(advData.GetAdvFlag());
        std::map<uint16_t, std::string> manufacturerData = advData.GetManufacturerData();
        for (auto iter = manufacturerData.begin(); iter != manufacturerData.end(); iter++) {
            bleAdvertiserData.AddManufacturerData(iter->first, iter->second);
        }
        std::map<ParcelUuid, std::string> serviceData = advData.GetServiceData();
        for (auto it = serviceData.begin(); it != serviceData.end(); it++) {
            Uuid uuid = Uuid::ConvertFromString(it->first.ToString());
            bleAdvertiserData.AddServiceData(uuid, it->second);
        }
        std::vector<ParcelUuid> serviceUuids = advData.GetServiceUuids();
        for (auto it = serviceUuids.begin(); it != serviceUuids.end(); it++) {
            Uuid uuid = Uuid::ConvertFromString(it->ToString());
            bleAdvertiserData.AddServiceUuid(uuid);
        }

        BluetoothBleAdvertiserData bleScanResponse;
        manufacturerData = scanResponse.GetManufacturerData();
        for (auto it = manufacturerData.begin(); it != manufacturerData.end(); it++) {
            bleScanResponse.AddManufacturerData(it->first, it->second);
        }
        serviceData = scanResponse.GetServiceData();
        for (auto it = serviceData.begin(); it != serviceData.end(); it++) {
            Uuid uuid = Uuid::ConvertFromString(it->first.ToString());
            bleScanResponse.AddServiceData(uuid, it->second);
        }
        serviceUuids = scanResponse.GetServiceUuids();
        for (auto it = serviceUuids.begin(); it != serviceUuids.end(); it++) {
            Uuid uuid = Uuid::ConvertFromString(it->ToString());
            bleScanResponse.AddServiceUuid(uuid);
        }

        int32_t advHandle = BLE_INVALID_ADVERTISING_HANDLE;
        if (pimpl->callbacks_.IsExistAdvertiserCallback(&callback, advHandle)) {
            pimpl->proxy_->StartAdvertising(setting, bleAdvertiserData, bleScanResponse, advHandle, false);
        } else {
            advHandle = pimpl->proxy_->GetAdvertiserHandle();
            if (advHandle == BLE_INVALID_ADVERTISING_HANDLE) {
                callback.OnStartResultEvent(BLE_INVALID_ADVERTISING_HANDLE);
                return;
            }
            pimpl->callbacks_.Register(advHandle, &callback);
            pimpl->proxy_->StartAdvertising(setting, bleAdvertiserData, bleScanResponse, advHandle, false);
        }
    }
}

void BleAdvertiser::StartAdvertising(const BleAdvertiserSettings &settings, const std::vector<uint8_t> &advData,
    const std::vector<uint8_t> &scanResponse, BleAdvertiseCallback &callback)
{
    HILOGD("BleAdvertiser::StartAdvertising");
    if (pimpl->proxy_ != nullptr) {
        BluetoothBleAdvertiserSettings setting;
        setting.SetConnectable(settings.IsConnectable());
        setting.SetInterval(settings.GetInterval());
        setting.SetLegacyMode(settings.IsLegacyMode());
        setting.SetTxPower(settings.GetTxPower());

        BluetoothBleAdvertiserData bleAdvertiserData;
        std::string dataPayload;
        bleAdvertiserData.SetAdvFlag(0);
        dataPayload.assign(advData.begin(), advData.end());
        bleAdvertiserData.SetPayload(dataPayload);

        std::string respPayload;
        BluetoothBleAdvertiserData bleScanResponse;
        respPayload.assign(scanResponse.begin(), scanResponse.end());
        bleScanResponse.SetPayload(respPayload);

        int32_t advHandle = BLE_INVALID_ADVERTISING_HANDLE;
        if (pimpl->callbacks_.IsExistAdvertiserCallback(&callback, advHandle)) {
            pimpl->proxy_->StartAdvertising(setting, bleAdvertiserData, bleScanResponse, advHandle, true);
        } else {
            advHandle = pimpl->proxy_->GetAdvertiserHandle();
            if (advHandle == BLE_INVALID_ADVERTISING_HANDLE) {
                callback.OnStartResultEvent(BLE_INVALID_ADVERTISING_HANDLE);
                return;
            }
            pimpl->callbacks_.Register(advHandle, &callback);
            pimpl->proxy_->StartAdvertising(setting, bleAdvertiserData, bleScanResponse, advHandle, true);
        }
    }
}

void BleAdvertiser::StopAdvertising(BleAdvertiseCallback &callback)
{
    HILOGD("BleAdvertiser::StartAdvertising");
    if (pimpl->proxy_ != nullptr) {
        uint8_t advHandle = pimpl->callbacks_.GetAdvertiserHandle(&callback);
        if (advHandle == BLE_INVALID_ADVERTISING_HANDLE) {
            callback.OnStartResultEvent(BLE_INVALID_ADVERTISING_HANDLE);
            return;
        }
        pimpl->proxy_->StopAdvertising(advHandle);
    }
}

void BleAdvertiser::Close(BleAdvertiseCallback &callback)
{
    if (pimpl->proxy_ != nullptr) {
        uint8_t advHandle = pimpl->callbacks_.GetAdvertiserHandle(&callback);
        pimpl->proxy_->Close(advHandle);
    }
}

BleAdvertiserData::BleAdvertiserData()
{}

BleAdvertiserData::~BleAdvertiserData()
{}

void BleAdvertiserData::AddServiceData(const ParcelUuid &uuid, const std::string &serviceData)
{
    if (serviceData.empty()) {
        return;
    }

    serviceData_.insert(std::make_pair(uuid, serviceData));
}

void BleAdvertiserData::AddManufacturerData(uint16_t manufacturerId, const std::string &data)
{
    if (data.empty()) {
        return;
    }

    manufacturerSpecificData_.insert(std::make_pair(manufacturerId, data));
}

std::map<uint16_t, std::string> BleAdvertiserData::GetManufacturerData() const
{
    return manufacturerSpecificData_;
}

void BleAdvertiserData::AddServiceUuid(const ParcelUuid &serviceUuid)
{
    serviceUuids_.push_back(serviceUuid);
}

std::vector<ParcelUuid> BleAdvertiserData::GetServiceUuids() const
{
    return serviceUuids_;
}

void BleAdvertiserData::SetAdvFlag(uint8_t flag)
{
    advFlag_ = flag;
}

uint8_t BleAdvertiserData::GetAdvFlag() const
{
    return advFlag_;
}

std::map<ParcelUuid, std::string> BleAdvertiserData::GetServiceData() const
{
    return serviceData_;
}

BleAdvertiserSettings::BleAdvertiserSettings()
{}

BleAdvertiserSettings::~BleAdvertiserSettings()
{}

void BleAdvertiserSettings::SetConnectable(bool connectable)
{
    connectable_ = connectable;
}

bool BleAdvertiserSettings::IsConnectable() const
{
    return connectable_;
}

void BleAdvertiserSettings::SetLegacyMode(bool legacyMode)
{
    legacyMode_ = legacyMode;
}

bool BleAdvertiserSettings::IsLegacyMode() const
{
    return legacyMode_;
}

void BleAdvertiserSettings::SetInterval(uint16_t interval)
{
    interval_ = interval;
}

uint16_t BleAdvertiserSettings::GetInterval() const
{
    return interval_;
}

void BleAdvertiserSettings::SetTxPower(uint8_t txPower)
{
    txPower_ = txPower;
}

uint8_t BleAdvertiserSettings::GetTxPower() const
{
    return txPower_;
}

int BleAdvertiserSettings::GetPrimaryPhy() const
{
    return primaryPhy_;
}

void BleAdvertiserSettings::SetPrimaryPhy(int primaryPhy)
{
    primaryPhy_ = primaryPhy;
}

int BleAdvertiserSettings::GetSecondaryPhy() const
{
    return secondaryPhy_;
}

void BleAdvertiserSettings::SetSecondaryPhy(int secondaryPhy)
{
    secondaryPhy_ = secondaryPhy;
}
}  // namespace Bluetooth
}  // namespace OHOS
