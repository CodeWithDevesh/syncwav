#include <syncwav/io/TCPOutput.h>
#include <syncwav/log.h>

namespace swav {

struct PerSocketData {
  /* Fill with user data */
};

TCPOutput::TCPOutput(Context &context, const char *ip, int port, int buffSiz)
    : Output("TCP Output", context, buffSiz), ip(ip), port(port) {}

TCPOutput::~TCPOutput() { stop(); }

void TCPOutput::start() {
  if (!running) {
    running = true;
    Output::start();
    run();
  }
}

void TCPOutput::stop() {
  if (running) {
    Output::stop();
    running = false;
    broadcastThread.join();
    globalApp->close();
    appThread.join();
  }
}

void TCPOutput::run() {
  appThread = std::thread([&] {
    uWS::App app = uWS::App();
    app.ws<PerSocketData>(
        "/*",
        {/* Settings */
         .compression = uWS::DISABLED,
         .maxPayloadLength = 16 * 1024 * 1024,
         .idleTimeout = 16,
         .maxBackpressure = 1 * 1024 * 1024,
         .closeOnBackpressureLimit = false,
         .resetIdleTimeoutOnSend = false,
         .sendPingsAutomatically = true,
         /* Handlers */
         .upgrade = nullptr,
         .open =
             [](auto *ws) {
               /* Open event here, you may access ws->getUserData() which points
                * to a PerSocketData struct */
               ws->subscribe("broadcast");
             },
         .message =
             [](auto * /*ws*/, std::string_view /*message*/,
                uWS::OpCode /*opCode*/) {

             },
         .drain =
             [](auto * /*ws*/) {
               /* Check ws->getBufferedAmount() here */
             },
         .ping =
             [](auto * /*ws*/, std::string_view) {
               /* Not implemented yet */
             },
         .pong =
             [](auto * /*ws*/, std::string_view) {
               /* Not implemented yet */
             },
         .close =
             [](auto * /*ws*/, int /*code*/, std::string_view /*message*/) {
               /* You may access ws->getUserData() here */
             }});

    app.listen(ip, port, [&](auto *listen_socket) {
      if (listen_socket) {
        log::i("Listening on port {}", port);
      } else {
        log::e("Failed to listen on port {}", port);
        running = false;
      }
    });
    globalApp = &app;
    globalLoop = uWS::Loop::get();
    if (running)
      app.run();
  });

  // Wait for the app to start
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  broadcastThread = std::thread(([&] {
    using namespace std::chrono;
    auto now = steady_clock::now();
    steady_clock::time_point nextTick = now + 10ms;
    // TODO: Do something better here
    void *buffer = malloc(context.framesSizeInBytes * 4800);
    while (running) {
      std::this_thread::sleep_until(nextTick);

      ma_uint32 framesAvailable = availableRead();

      if (framesAvailable < 480) {
        nextTick += 5ms;
        continue;
      }
      framesAvailable /= 480;

      ma_uint32 framesToSend = ((framesAvailable * 4) / 4);
      framesToSend *= 480;
      log::t("[TCP Output] Gonna send {} frames of audio data.", framesToSend);
      read(buffer, framesToSend);

      globalLoop->defer([&] {
        globalApp->publish(
            "broadcast",
            std::string_view(reinterpret_cast<const char *>(buffer),
                             framesToSend * context.framesSizeInBytes),
            uWS::OpCode::BINARY, false);
        log::t("[TCP Output] Broadcasting at: {}",
               steady_clock::now().time_since_epoch().count());
      });
      nextTick = steady_clock::now() + 5ms;
    }
    free(buffer);
  }));
}

} // namespace swav
