#pragma once
#include "../export.h"
#include "../tick/tick-consumer.h"
#include <miniaudio.h>

namespace swav {

struct Context;

class SWAV_API Input : public TickConsumer {
public:
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual ~Input() = default;
  
  Input(const char *name, Context &context);

public:
  const char *name;

protected:
};

} // namespace swav
