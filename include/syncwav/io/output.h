#pragma once
#include "../export.h"
#include "../tick/tick-producer.h"

namespace swav {

class Middleware;
struct Context;

class SWAV_API Output : public TickProducer {
public:
  // This method will be called by input to write to the buffer
  Output(const char *name, Context &);

  virtual ~Output();

  virtual void start();
  virtual void stop();

public:
  const char *name;

protected:
};

} // namespace swav
