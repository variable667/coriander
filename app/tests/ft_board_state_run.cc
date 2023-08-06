/**
 * @file ft_board_state_run.cc
 * @author Savent Gate (savent_gate@outlook.com)
 * @brief
 * @date 2023-08-06
 *
 * Copyright 2023 savent_gate
 *
 */
#include <gtest/gtest.h>

#include "boost/di.hpp"
#include "coriander/application/iappstatus.h"
#include "coriander/base/const_hash.h"
#include "coriander/base/ilogger.h"
#include "coriander/base/property.h"
#include "coriander/board_state_run_handler.h"
#include "coriander/coriander.h"
#include "coriander/motorctl/foc_motor_driver.h"
#include "coriander/motorctl/iencoder.h"
#include "coriander/motorctl/imotorctl.h"
#include "coriander/motorctl/motor_ctl_velocity.h"
#include "coriander/os/isystick.h"
#include "coriander/parameters.h"
#include "mocks.h"

namespace {
using coriander::base::operator""_hash;

struct MockIMotorCtl : public coriander::motorctl::IMotorCtl {
  MOCK_METHOD0(start, void());
  MOCK_METHOD0(stop, void());
  MOCK_METHOD0(loop, void());
  MOCK_METHOD0(emergencyStop, void());
  MOCK_METHOD0(fatalError, bool());
};
static auto createInjector() {
  return boost::di::make_injector(
      boost::di::bind<coriander::application::IAppStatus>.to<testing::mock::MockAppStatus>(),
      boost::di::bind<coriander::base::ILogger>.to<testing::mock::MockLogger>(),
      boost::di::bind<coriander::os::ISystick>.to<testing::mock::MockSystick>(),
      boost::di::bind<coriander::motorctl::IEncoder>.to<testing::mock::MockEncoder>(),
      boost::di::bind<coriander::motorctl::FocMotorDriver>.to<testing::mock::MockFocMotorDriver>());
}
}  // namespace

TEST(BoardStateRun, basic) {
  using AppStatus = coriander::application::IAppStatus::Status;
  auto c = coriander::coriander_create_injector(createInjector());

  auto param = c.create<std::shared_ptr<coriander::ParameterBase>>();
  param->add(coriander::base::Property{1, "motorctl.mode"});
  param->add(coriander::base::Property{1.0f, "velocity_pid_p"});
  param->add(coriander::base::Property{0.0f, "velocity_pid_i"});
  param->add(coriander::base::Property{0.0f, "velocity_pid_d"});
  param->add(coriander::base::Property{99999.0f, "velocity_pid_output_ramp"});
  param->add(coriander::base::Property{99999.0f, "velocity_pid_limit"});
  param->add(coriander::base::Property{600.0f, "target_velocity"});
  param->add(coriander::base::Property{16.0f, "motor_supply_voltage"});
  param->add(coriander::base::Property(16, "velocity_sample_window_size"));
  param->add(coriander::base::Property{3500, "velocity_sample_window_time"});
  param->add(coriander::base::Property{50, "velocity_sample_minimal_duration"});
  param->add(coriander::base::Property{4, "pole_pair"});

  auto encoder = c.create<std::shared_ptr<testing::mock::MockEncoder>>();

  auto runHandler =
      c.create<std::shared_ptr<coriander::IBoardStateRunHandler>>();

  EXPECT_CALL(*encoder, sync()).Times(1);
  runHandler->onEnter();
  runHandler->onLoop();
}