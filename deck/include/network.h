#pragma once

#include <netinet/in.h>
#include "data.h"

class NetworkClient {
public:
    NetworkClient(int port);
    ~NetworkClient();

    void EnableBroadcast();
    void SendBroadcast(const Packet& packet);
    bool Send(const Packet& packet, sockaddr_in dest);

private:
    int sock;
    int port;
};