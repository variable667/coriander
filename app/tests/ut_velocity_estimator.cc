/**
 * @file ut_velocity_estimator.cc
 * @author Savent Gate (savent_gate@outlook.com)
 * @brief
 * @date 2023-08-03
 *
 * Copyright 2023 savent_gate
 *
 */
#include <gtest/gtest.h>

#include "coriander/motorctl/velocity_estimator.h"
#include "coriander/parameters.h"

namespace {
struct DummyMechAngleEstimator
    : public coriander::motorctl::IMechAngleEstimator {
  virtual void enable() {}
  virtual void disable() {}
  virtual bool enabled() { return true; }
  virtual void sync() {}
  virtual bool needSync(uint32_t syncId) { return false; }
  virtual bool needCalibrate() { return false; }
  virtual void calibrate() {}
  virtual float getMechanicalAngle() noexcept { return mMechAngle; }

  float mMechAngle;
};

struct DummySystick : public coriander::os::ISystick {
  virtual uint32_t systick_ms() { return ms; }
  virtual uint32_t systick_us() { return ms * 1000; }
  uint32_t ms = 0;
};

using Property = coriander::base::Property;
using namespace coriander::base;

TEST(ISensor, BasicVelocityEstimator) {
  ::DummyMechAngleEstimator mechAngleEstimator;
  coriander::ParameterBase param;
  ::DummySystick systick;

  param.add(Property{16, "velocity_sample_window_size"});
  param.add(Property{500, "velocity_sample_window_time"});
  param.add(Property{30, "velocity_sample_minimal_duration"});

  coriander::motorctl::VelocityEstimator velocityEstimator(&mechAngleEstimator,
                                                           &param, &systick);

  velocityEstimator.enable();

  // input 30 samples but not reach minimal duration
  for (int i = 0; i < 30; i++) {
    systick.ms = 30 + i;
    mechAngleEstimator.mMechAngle = i * 10.0f;
    velocityEstimator.sync();
  }
  ASSERT_NEAR(velocityEstimator.getVelocity(), 0.0f, 1e-6);

  // input 1 samples and reach minimal duration
  systick.ms = 30 + 30;
  mechAngleEstimator.mMechAngle = 300.0f;
  velocityEstimator.sync();
  ASSERT_NEAR(velocityEstimator.getVelocity(), (300.0f / 360) / (60.0f / 60000),
              1e-6);
  // input 13 samples and velocity should be slower
  for (int i = 0; i < 13; i++) {
    systick.ms += 30;
    velocityEstimator.sync();
    ASSERT_NEAR(velocityEstimator.getVelocity(),
                (300.0f / 360.0f) / (systick.ms / 60000.0f), 1e-6);
  }

  // input 2 samples and velocity should down to 0
  for (int i = 0; i < 2; i++) {
    systick.ms += 30;
    velocityEstimator.sync();
  }
  ASSERT_NEAR(velocityEstimator.getVelocity(), 0, 1e-6);

  // input normal samples and velocity should be faster
  float v = 0;
  for (int i = 0; i < 30; i++) {
    systick.ms += 30;
    mechAngleEstimator.mMechAngle += 10.0f;
    velocityEstimator.sync();
    ASSERT_GE(velocityEstimator.getVelocity(), v);
    v = velocityEstimator.getVelocity();
  }
  ASSERT_NEAR(velocityEstimator.getVelocity(), (10.0f / 360) / (30.0f / 60000),
              1e-5);

  // re-enable it , sample should be cleared
  velocityEstimator.disable();
  velocityEstimator.enable();
  ASSERT_NEAR(velocityEstimator.getVelocity(), 0, 1e-6);
  velocityEstimator.sync();
  ASSERT_NEAR(velocityEstimator.getVelocity(), 0, 1e-6);
}

}  // namespace