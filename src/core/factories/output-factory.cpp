#include <syncwav/context.h>
#include <syncwav/factories/output-factory.h>
#include <syncwav/io/file-input.h>
#include <syncwav/io/local-output.h>
#include <syncwav/io/loopback-input.h>
#include <syncwav/io/output.h>
#include <syncwav/io/tcp-output.h>

namespace swav {

LocalOutputFactory::LocalOutputFactory(Device device) : device(device) {}

Output *LocalOutputFactory::getInstance(Context &context) {
  return new LocalOutput(context, device);
}

TCPOutputFactory::TCPOutputFactory(const char *ip, int port)
    : ip(ip), port(port) {}

Output *TCPOutputFactory::getInstance(Context &context) {
  return new TCPOutput(context, ip, port);
}

} // namespace swav
