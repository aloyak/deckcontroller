#include "network.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>

NetworkClient::NetworkClient(int port) : port(port) {
    sock = socket(AF_INET, SOCK_DGRAM, 0);
}

NetworkClient::~NetworkClient() {
    close(sock);
}

void NetworkClient::EnableBroadcast() {
    int broadcastEnable = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
}

void NetworkClient::SendBroadcast(const Packet& packet) {
    sockaddr_in broadcastAddr{};
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(port);
    broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;
    
    sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
}

bool NetworkClient::Send(const Packet& packet, sockaddr_in dest) {
    return sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr*)&dest, sizeof(dest)) > 0;
}

void NetworkClient::SetNonBlocking(bool enable) {
    int flags = fcntl(sock, F_GETFL, 0);
    if (enable) flags |= O_NONBLOCK;
    else        flags &= ~O_NONBLOCK;
    fcntl(sock, F_SETFL, flags);
}

bool NetworkClient::Receive(Packet& packet, sockaddr_in& sender) {
    socklen_t len = sizeof(sender);
    int bytes = recvfrom(sock, &packet, sizeof(packet), 0,
                         (struct sockaddr*)&sender, &len);
    return bytes > 0;
}