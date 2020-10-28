// Copyright (c) 2020 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/components/ipfs/ipfs_p3a.h"

#include "brave/components/ipfs/ipfs_service.h"
#include "brave/components/ipfs/pref_names.h"
#include "components/prefs/testing_pref_service.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace ipfs {

class IPFSP3ATest : public testing::Test {
 public:
  IPFSP3ATest() {}

  void SetUp() override {
    auto* registry = pref_service_.registry();
    IpfsService::RegisterPrefs(registry);
  }

  PrefService* GetPrefs() { return &pref_service_; }

  TestingPrefServiceSimple pref_service_;
};

TEST_F(IPFSP3ATest, TestVal1) {
  auto* prefs = GetPrefs();
  ASSERT_EQ(GetIPFSDetectionPromptBucket(prefs), 0);
  prefs->SetInteger(kIPFSInfobarCount, 1);
  ASSERT_EQ(GetIPFSDetectionPromptBucket(prefs), 1);
  prefs->SetInteger(kIPFSInfobarCount, 2);
  ASSERT_EQ(GetIPFSDetectionPromptBucket(prefs), 2);
  prefs->SetInteger(kIPFSInfobarCount, 3);
  ASSERT_EQ(GetIPFSDetectionPromptBucket(prefs), 2);
  prefs->SetInteger(kIPFSInfobarCount, 5);
  ASSERT_EQ(GetIPFSDetectionPromptBucket(prefs), 2);
  prefs->SetInteger(kIPFSInfobarCount, 6);
  ASSERT_EQ(GetIPFSDetectionPromptBucket(prefs), 3);
  prefs->SetInteger(kIPFSInfobarCount, 1337);
  ASSERT_EQ(GetIPFSDetectionPromptBucket(prefs), 3);
}

}  // namespace ipfs
