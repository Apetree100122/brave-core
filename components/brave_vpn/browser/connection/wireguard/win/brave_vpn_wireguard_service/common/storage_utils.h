/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_CONNECTION_WIREGUARD_WIN_BRAVE_VPN_WIREGUARD_SERVICE_COMMON_STORAGE_UTILS_H_
#define BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_CONNECTION_WIREGUARD_WIN_BRAVE_VPN_WIREGUARD_SERVICE_COMMON_STORAGE_UTILS_H_

#include "base/files/file_path.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace brave_vpn {

namespace wireguard {

bool IsVPNTrayIconEnabled();
void EnableVPNTrayIcon(bool value);

bool UpdateLastUsedConfigPath(const base::FilePath& config_path);
absl::optional<base::FilePath> GetLastUsedConfigPath();

}  // namespace wireguard
}  // namespace brave_vpn

#endif  // BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_CONNECTION_WIREGUARD_WIN_BRAVE_VPN_WIREGUARD_SERVICE_COMMON_STORAGE_UTILS_H_
