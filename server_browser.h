#pragma once

#include <vector>
#include <string>

struct SBServerInfo {
    std::string name;
    std::string map;
    std::string mode;
    std::string ip;   // host:port
    int players = 0;
    int maxPlayers = 0;
    int ping = -1;
};

namespace server_browser {
    // Init reads config and prepares hooks/resolvers (no heavy work).
    void init();

    // Periodic refresh from in-game enumerator (if available) or HTTP/file source.
    void refresh_from_game();
    void refresh_from_file(const char* path);

    // Access current lists (two tabs): game-extracted and custom IPs.
    const std::vector<SBServerInfo>& get_game_list();
    const std::vector<SBServerInfo>& get_custom_list();

    // Add/remove custom entries.
    void add_custom_server(const SBServerInfo& s);
    void clear_custom_servers();

    // Request join.
    bool join(const SBServerInfo& s);
}
