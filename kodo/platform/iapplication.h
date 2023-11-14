#pragma once

#include <string>
#include <vector>

namespace kodo {

class IApplication {
 public:
  virtual ~IApplication() noexcept = default;

  virtual void init(const std::vector<std::string>& cmdArgs) = 0;
  virtual void terminate() = 0;
};

}  // namespace kodo
