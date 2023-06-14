// Copyright (c) 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef BRAVE_SERVICES_DEVICE_GEOLOCATION_GEOCLUE_LOCATION_PROVIDER_H_
#define BRAVE_SERVICES_DEVICE_GEOLOCATION_GEOCLUE_LOCATION_PROVIDER_H_

#include <memory>
#include <string>
#include <vector>

#include "base/component_export.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "brave/services/device/geolocation/geoclue_client_object.h"
#include "dbus/bus.h"
#include "services/device/public/cpp/geolocation/location_provider.h"
#include "services/device/public/mojom/geoposition.mojom.h"

namespace device {

class COMPONENT_EXPORT(BRAVE_GEOLOCATION) GeoClueLocationProvider
    : public LocationProvider {
 public:
  GeoClueLocationProvider();

  GeoClueLocationProvider(const GeoClueLocationProvider&) = delete;
  GeoClueLocationProvider& operator=(const GeoClueLocationProvider&) = delete;

  ~GeoClueLocationProvider() override;

  // LocationProvider:
  void SetUpdateCallback(
      const LocationProviderUpdateCallback& callback) override;
  void StartProvider(bool high_accuracy) override;
  void StopProvider() override;
  const mojom::GeopositionResult* GetPosition() override;
  void OnPermissionGranted() override;

 protected:
  enum ClientState {
    kStopped,
    kInitializing,
    kInitialized,
    kStarting,
    kStarted,
  };

  ClientState client_state_ = ClientState::kStopped;

  // Stores whether or not permission has been granted.
  bool permission_granted_ = false;
  bool high_accuracy_requested_ = false;

  void SetPosition(mojom::GeopositionResultPtr position);

 private:
  // There is a bit of a process to setup a GeoClue2.Client and start listening
  // for location changes:
  // 1. Get the current GeoClue2.Manager
  // 2. Call Manager.GetClient(), which returns a client
  // 3. SetClientProperties - we need to set an accuracy, and an identifier for
  // ourselves.
  // 4. Connect to the `LocationUpdated` signal, which will fire when we get a
  // location.
  // 5. Finally, we can call GeoClue2.Client.Start(), which will let us access
  // the current location.
  // In this process, it's safe to do steps 1, 2, 3 & 4 before permission is
  // granted, but step 5 should not be called until permission has been granted.
  //
  // If any step fails, we set the current position to POSITION_UNAVAILABLE.
  //
  // When the provider is stopped this process is completely reset.

  // Step 2
  void GetClient();
  void OnGetClientCompleted(std::unique_ptr<GeoClueClientObject> client);

  // Step 3
  void SetClientProperties();
  void OnSetClientProperties(std::vector<bool> success);

  // Step 4
  void ConnectSignal();
  void OnSignalConnected(const std::string& interface_name,
                         const std::string& signal_name,
                         bool success);

  // Step 5: Start the client. This can be called before permission has been
  // granted or while the client is starting up but it will have no effect.
  void MaybeStartClient();
  void OnClientStarted(dbus::Response* response);

  void OnReadGeoClueLocation(
      std::unique_ptr<GeoClueClientObject::LocationProperties> properties);

  scoped_refptr<dbus::Bus> bus_;
  std::unique_ptr<GeoClueClientObject> client_;

  mojom::GeopositionResultPtr last_position_;
  LocationProviderUpdateCallback position_update_callback_;

  base::WeakPtrFactory<GeoClueLocationProvider> weak_ptr_factory_{this};
};

COMPONENT_EXPORT(BRAVE_GEOLOCATION)
std::unique_ptr<GeoClueLocationProvider> MaybeCreateGeoClueLocationProvider();

}  // namespace device

#endif  // BRAVE_SERVICES_DEVICE_GEOLOCATION_GEOCLUE_LOCATION_PROVIDER_H_
