#include "network.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
    #define closesocket close
#endif

#include <cstring>
#include <atomic>
#include <mutex>

#include "data.h"

void RunNetworkLoop(std::atomic<bool>& running, SharedState& shared, int port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return;
    }
#endif

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    }
#endif

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));

    sockaddr_in clientAddr{};
    socklen_t addrLen = sizeof(clientAddr);
    Packet packet{};

    while (running) {
        int bytes = recvfrom(sock, reinterpret_cast<char*>(&packet), sizeof(packet), 0,
                             reinterpret_cast<sockaddr*>(&clientAddr), &addrLen);
        if (bytes > 0) {
            if (packet.type == PACKET_BROADCAST) {
                Packet ack = {PACKET_BROADCAST, "PC"};
                sendto(sock, reinterpret_cast<const char*>(&ack), sizeof(ack), 0,
                       reinterpret_cast<sockaddr*>(&clientAddr), addrLen);

                std::lock_guard<std::mutex> lock(shared.mtx);
                shared.connected = true;
                shared.deckIP = inet_ntoa(clientAddr.sin_addr);
                shared.deckID = packet.id;
            }
            else if (packet.type == PACKET_INPUT_STATE) {
                std::lock_guard<std::mutex> lock(shared.mtx);
                shared.state = packet.state;
                shared.lastPacketTime = SDL_GetTicks();
            }
        }
        else {
            SDL_Delay(1);
        }
    }

    closesocket(sock);
#ifdef _WIN32
    WSACleanup();
#endif
}