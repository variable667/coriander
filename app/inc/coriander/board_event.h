/**
 * @file board_event.h
 * @author Savent Gate (savent_gate@outlook.com)
 * @brief
 * @date 2023-07-28
 *
 * Copyright 2023 savent_gate
 *
 */
#pragma once

#include "coriander/base/ilogger.h"
#include "coriander/base/loggerstream.h"
#include "coriander/iboard_event.h"

namespace coriander {
struct BoardEvent : public IBoardEvent {
  explicit BoardEvent(std::shared_ptr<base::ILogger> logger) noexcept
      : mLogger(logger), mEventCallbacks{} {}

  virtual void raiseEvent(Event event) noexcept override {
    if (mEventCallbacks[static_cast<int>(event)]) {
      base::LoggerStream os(mLogger);

      os << "Raise event: " << static_cast<int>(event) << std::endl;
      mEventCallbacks[static_cast<int>(event)](event);
    }
  }

  virtual void registerEventCallback(Event event,
                                     EventCallback callback) noexcept override {
    mEventCallbacks[static_cast<int>(event)] = callback;
  }

 private:
  std::shared_ptr<base::ILogger> mLogger;
  EventCallback mEventCallbacks[static_cast<int>(Event::MAX_EVENT)];
};
}  // namespace coriander