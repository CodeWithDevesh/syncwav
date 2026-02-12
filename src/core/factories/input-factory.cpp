#include <syncwav/context.h>
#include <syncwav/factories/input-factory.h>
#include <syncwav/io/capture-input.h>
#include <syncwav/io/file-input.h>
#include <syncwav/io/input.h>
#include <syncwav/io/loopback-input.h>
#include <syncwav/io/tcp-input.h>

namespace swav {

CaptureInputFactory::CaptureInputFactory(Device d) : device(d) {}

Input *CaptureInputFactory::getInstance(Context &context) const {
  return new CaptureInput(context, device);
}

LoopbackInputFactory::LoopbackInputFactory(Device d) : device(d) {}

Input *LoopbackInputFactory::getInstance(Context &context) const {
  return new LoopbackInput(context, device);
}

FileInputFactory::FileInputFactory(const char *filePath) : filePath(filePath) {}

Input *FileInputFactory::getInstance(Context &context) const {
  return new FileAudioInput(context, filePath);
}

TcpInputFactory::TcpInputFactory(const char *ip, int port)
    : ip(ip), port(port) {}

Input *TcpInputFactory::getInstance(Context &context) const {
  return new TCPInput(context, ip, port);
}

} // namespace swav
