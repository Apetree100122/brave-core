/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_FL_PREDICTORS_VARIABLES_DOES_MATCH_CHILD_INTENT_SEGMENT_PREDICTOR_VARIABLE_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_FL_PREDICTORS_VARIABLES_DOES_MATCH_CHILD_INTENT_SEGMENT_PREDICTOR_VARIABLE_H_

#include <string>

#include "brave/components/brave_ads/core/internal/fl/predictors/variables/predictor_variable_interface.h"

namespace brave_ads {

class DoesMatchChildIntentSegmentPredictorVariable final
    : public PredictorVariableInterface {
public:
  explicit DoesMatchChildIntentSegmentPredictorVariable(float value);

  // PredictorVariableInterface:
  brave_federated::mojom::DataType GetDataType() const override;
  brave_federated::mojom::CovariateType GetType() const override;
  std::string GetValue() const override;

private:
  const float value_;
};

} // namespace brave_ads

#endif // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_FL_PREDICTORS_VARIABLES_DOES_MATCH_CHILD_INTENT_SEGMENT_PREDICTOR_VARIABLE_H_
