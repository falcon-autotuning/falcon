#include "falcon-mock-hub/mock_hub.hpp"
#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>
#include <atomic>

static std::atomic<bool> g_stop{false};

void signal_handler(int) {
    g_stop = true;
}

int main(int argc, char **argv) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    int port = 4222;
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }

    std::cout << "[MockHubDaemon] Starting MockHubServer on port " << port << "..." << std::endl;
    falcon::mock_hub::MockHubServer server;
    if (!server.start(port)) {
        std::cerr << "[MockHubDaemon] Failed to start MockHubServer on port " << port << std::endl;
        return 1;
    }

    // Set sample responses
    auto meas_resp = falcon::mock_hub::MockHubServer::sample_measurement_response();
    server.set_measurement_response(meas_resp);

    std::cout << "MOCK_HUB_READY" << std::endl;
    std::cout.flush();

    while (!g_stop) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::cout << "[MockHubDaemon] Stopping server..." << std::endl;
    server.stop();
    return 0;
}
