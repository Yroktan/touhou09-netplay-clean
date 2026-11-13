#pragma once

namespace thnet {
struct RelayEndpoint {
    char host[256];
    unsigned short port;
};
bool LoadRelayEndpoint(const char *host,unsigned short port,RelayEndpoint &endpoint,char *error,unsigned errorSize);
}
