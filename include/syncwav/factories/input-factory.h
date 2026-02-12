#pragma once
#include "../export.h"
#include <miniaudio.h>
#include <syncwav/platform/device.h>

namespace swav {

class Input;
class Context;

class SWAV_API InputFactory {
public:
  virtual ~InputFactory() = default;
  virtual Input *getInstance(Context &context) const = 0;
};

class SWAV_API CaptureInputFactory : public InputFactory {
public:
  CaptureInputFactory(Device);

  Input *getInstance(Context &) const override;

private:
  Device device;
};

class SWAV_API LoopbackInputFactory : public InputFactory {
public:
  LoopbackInputFactory(Device);

  Input *getInstance(Context &) const override;

private:
  Device device;
};

class SWAV_API FileInputFactory : public InputFactory {
public:
  FileInputFactory(const char *filePath);

  Input *getInstance(Context &) const override;

private:
  const char *filePath;
};

class SWAV_API TcpInputFactory : public InputFactory {
public:
  TcpInputFactory(const char *ip, int port);

  Input *getInstance(Context &) const override;

private:
  int port;
  const char *ip;
};

} // namespace swav
