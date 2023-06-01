/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/fl/predictors/variables/does_match_child_interest_segment_predictor_variable_util.h"

#include <memory>

#include "brave/components/brave_ads/core/internal/fl/predictors/predictors_manager.h"
#include "brave/components/brave_ads/core/internal/fl/predictors/variables/does_match_child_interest_segment_predictor_variable.h"

namespace brave_ads {

void SetDoesMatchChildInterestSegmentPredictorVariable(const float value) {

  PredictorsManager::GetInstance().SetPredictorVariable(
      std::make_unique<DoesMatchChildInterestSegmentPredictorVariable>(value));
}

} // namespace brave_ads
