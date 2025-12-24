#pragma once
#include "../export.h"
#include <miniaudio.h>

namespace swav {

struct Context;

class SWAV_API Input {
public:
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual ~Input() = default;
  Input(const char *name, Context &context);

public:
  const char *name;

protected:
  Context &context;
};

} // namespace swav
