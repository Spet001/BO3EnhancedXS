#include "server_browser.h"
#include "framework.h"
#include "security.h"
#include <algorithm>
#include <filesystem>
#include <cstdio>

static std::vector<SBServerInfo> g_gameList;
static std::vector<SBServerInfo> g_customList;

static void trim(std::string& s) {
    while (!s.empty() && (s.back()=='\r' || s.back()=='\n' || s.back()==' ')) s.pop_back();
}

namespace server_browser {

void init()
{
    // Read optional file list path from config
    const char* path = security::get_config_value("server_list");
    if (path && std::filesystem::exists(path)) {
        refresh_from_file(path);
        ALOG("ServerBrowser: loaded custom servers from %s", path);
    } else {
        // Try default next to DLL
        char modulePath[MAX_PATH] = {0};
        if (GetModuleFileNameA((HMODULE)REBASE(0), modulePath, sizeof(modulePath))) {
            char* lastSlash = strrchr(modulePath, '\\');
            if (lastSlash) *lastSlash = '\0';
            char defaultPath[MAX_PATH] = {0};
            sprintf_s(defaultPath, "%s\\server_list.json", modulePath);
            if (std::filesystem::exists(defaultPath)) {
                refresh_from_file(defaultPath);
                ALOG("ServerBrowser: loaded custom servers from %s", defaultPath);
            }
        }
    }

    // Game-derived list will be populated via hook later
    g_gameList.clear();
}

void refresh_from_game()
{
    // Stub: this will be filled by a hook into the game enumerator
    // For now, do nothing. Keep last results.
}

// Minimal JSON reader for an array of {name,map,mode,ip,players,max,ping}
static void parse_json_file(const char* path)
{
    FILE* f = fopen(path, "rb");
    if (!f) return;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::string txt; txt.resize(sz);
    fread(txt.data(), 1, sz, f);
    fclose(f);

    // Extremely simple parser: look for objects and pull fields by name.
    g_customList.clear();
    size_t pos = 0;
    while ((pos = txt.find('{', pos)) != std::string::npos) {
        size_t end = txt.find('}', pos);
        if (end == std::string::npos) break;
        std::string obj = txt.substr(pos+1, end-pos-1);
        pos = end+1;

        SBServerInfo s;
        auto getStr = [&](const char* key)->std::string{
            std::string k = std::string("\"") + key + "\"";
            size_t p = obj.find(k);
            if (p==std::string::npos) return {};
            p = obj.find(':', p);
            if (p==std::string::npos) return {};
            size_t q = obj.find('"', p);
            if (q==std::string::npos) return {};
            size_t r = obj.find('"', q+1);
            if (r==std::string::npos) return {};
            std::string v = obj.substr(q+1, r-(q+1));
            trim(v);
            return v;
        };
        auto getInt = [&](const char* key)->int{
            std::string k = std::string("\"") + key + "\"";
            size_t p = obj.find(k);
            if (p==std::string::npos) return -1;
            p = obj.find(':', p);
            if (p==std::string::npos) return -1;
            size_t q = p+1;
            while (q<obj.size() && (obj[q]==' '||obj[q]=='\t')) ++q;
            size_t r = q;
            while (r<obj.size() && isdigit((unsigned char)obj[r])) ++r;
            if (r==q) return -1;
            return atoi(obj.substr(q, r-q).c_str());
        };

        s.name = getStr("name");
        s.map = getStr("map");
        s.mode = getStr("mode");
        s.ip = getStr("ip");
        s.players = getInt("players");
        s.maxPlayers = getInt("max");
        s.ping = getInt("ping");
        if (!s.ip.empty()) g_customList.push_back(s);
    }
}

void refresh_from_file(const char* path)
{
    parse_json_file(path);
}

const std::vector<SBServerInfo>& get_game_list()
{
    return g_gameList;
}

const std::vector<SBServerInfo>& get_custom_list()
{
    return g_customList;
}

void add_custom_server(const SBServerInfo& s)
{
    auto it = std::find_if(g_customList.begin(), g_customList.end(), [&](const SBServerInfo& x){return x.ip==s.ip;});
    if (it==g_customList.end()) g_customList.push_back(s);
}

void clear_custom_servers()
{
    g_customList.clear();
}

bool join(const SBServerInfo& s)
{
    // TODO: translate to game connect flow (Steam lobby/IP) once hook points are validated
    ALOG("ServerBrowser: requested join %s (%s) at %s", s.name.c_str(), s.map.c_str(), s.ip.c_str());
    return true;
}

}
