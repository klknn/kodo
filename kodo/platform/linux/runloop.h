#pragma once

#include <X11/Xlib.h>

#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

namespace kodo {

using TimerID = uint64_t;
using TimerInterval = uint64_t;
using TimerCallback = std::function<void(TimerID)>;

class TimerProcessor {
 public:
  TimerID registerTimer(TimerInterval interval, const TimerCallback& callback);
  void unregisterTimer(TimerID id);

  static constexpr uint64_t noTimers = std::numeric_limits<uint64_t>::max();
  uint64_t handleTimersAndReturnNextFireTimeInMs();

 private:
  using Clock = std::chrono::steady_clock;
  using Millisecond = std::chrono::milliseconds;
  using TimePoint = std::chrono::time_point<Clock, Millisecond>;

  struct Timer {
    TimerID id;
    TimerInterval interval;
    TimerCallback callback;
    TimePoint nextFireTime;
  };
  using Timers = std::vector<Timer>;
  Timers timers;
  TimerID timerIdCounter{0};

  void updateTimerNextFireTime(Timer& timer, TimePoint current);
  void sortTimers();
  TimePoint now();
};

class RunLoop {
 public:
  using EventCallback = std::function<bool(const XEvent& event)>;
  using FileDescriptorCallback = std::function<void(int fd)>;

  static RunLoop& instance();

  void setDisplay(Display* display);

  void registerWindow(XID window, const EventCallback& callback);
  void unregisterWindow(XID window);

  void registerFileDescriptor(int fd, const FileDescriptorCallback& callback);
  void unregisterFileDescriptor(int fd);

  TimerID registerTimer(TimerInterval interval, const TimerCallback& callback);
  void unregisterTimer(TimerID id);

  void start();
  void stop();

 private:
  void select(timeval* timeout = nullptr);
  bool handleEvents();

  using WindowMap = std::unordered_map<XID, EventCallback>;
  using FileDescriptorCallbacks =
      std::unordered_map<int, FileDescriptorCallback>;

  WindowMap map;
  FileDescriptorCallbacks fileDescriptors;
  TimerProcessor timerProcessor;

  Display* display{nullptr};
  bool running{false};
};

}  // namespace kodo
