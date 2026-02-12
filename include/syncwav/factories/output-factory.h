#pragma once
#include "../export.h"
#include <miniaudio.h>
#include <syncwav/platform/device.h>

namespace swav {

class Output;
class Context;

class SWAV_API OutputFactory {
public:
  virtual ~OutputFactory() = default;
  virtual Output *getInstance(Context &context) const = 0;
};

class SWAV_API LocalOutputFactory : public OutputFactory {
public:
  LocalOutputFactory(Device);

  Output *getInstance(Context &) const override;

private:
  Device device;
};

class SWAV_API TCPOutputFactory : public OutputFactory {
public:
  TCPOutputFactory(const char *ip, int port);

  Output *getInstance(Context &) const override;

private:
  const char *ip;
  int port;
};

} // namespace swav
