/**
 * @file motor_ctl_openloop.h
 * @author Savent Gate (savent_gate@outlook.com)
 * @brief
 * @date 2023-09-02
 *
 * Copyright 2023 savent_gate
 *
 */
#pragma once

#include <memory>
#include <utility>

#include "coriander/motorctl/duration_estimator.h"
#include "coriander/motorctl/foc_motor_driver.h"
#include "coriander/motorctl/imotorctl.h"
#include "coriander/parameter_requirements.h"
#include "coriander/parameter_requirements_validator.h"
#include "coriander/parameters.h"

namespace coriander::motorctl {

struct MotorCtlOpenLoop : public IMotorCtl, coriander::IParamReq {
  using DurationExpired =
      detail::DurationExpired<detail::DurationEstimatorUnit::MS>;

  explicit MotorCtlOpenLoop(
      std::shared_ptr<FocMotorDriver> motor,
      std::shared_ptr<Parameter> parameters,
      std::unique_ptr<DurationExpired> durationEstimator,
      std::shared_ptr<IParamReqValidator> paramReqValidator)
      : mFocMotorDriver(motor),
        mParameters(parameters),
        mDurationTimeout(std::move(durationEstimator)) {
    paramReqValidator->addParamReq(this);
  }
  virtual ~MotorCtlOpenLoop() = default;

  virtual void start();
  virtual void stop();
  virtual void loop();

  virtual void emergencyStop();
  virtual bool fatalError();

  virtual const ParameterRequireItem* requiredParameters() const {
    using Type = coriander::base::TypeId;
    static const ParameterRequireItem items[] = {
        {"MotorCtl_MotorDriver_SupplyVoltage", Type::Float},
        {"MotorCtl_OpenLoop_OutVoltage", Type::Float},
        PARAMETER_REQ_EOF};
    return items;
  }

 private:
  std::shared_ptr<FocMotorDriver> mFocMotorDriver;
  std::shared_ptr<Parameter> mParameters;
  std::unique_ptr<DurationExpired> mDurationTimeout;

  float mDutyCycleUd = 0;
  float mCurrentAngle = 0;
};
}  // namespace coriander::motorctl