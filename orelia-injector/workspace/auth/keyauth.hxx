#pragma once
#ifndef skCrypt
#define skCrypt encrypt
#endif
#include <string>
#include <vector>
#include <winhttp.h>
#include <sddl.h>
#include <bcrypt.h>
#include <cctype>
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "bcrypt.lib")

namespace keyauth {
    struct response_t {
        bool success = false;
        std::string message;
    };

    inline std::string url_encode(const std::string& value) {
        static const char hex[] = "0123456789ABCDEF";
        std::string out;
        out.reserve(value.size() * 3);
        for (unsigned char c : value) {
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
                out.push_back(static_cast<char>(c));
            }
            else {
                out.push_back('%');
                out.push_back(hex[(c >> 4) & 0xF]);
                out.push_back(hex[c & 0xF]);
            }
        }
        return out;
    }

    inline std::string get_hwid() {
        std::string binding;

        HANDLE token = nullptr;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
            DWORD size = 0;
            GetTokenInformation(token, TokenUser, nullptr, 0, &size);
            if (size != 0) {
                std::vector<BYTE> buffer(size);
                if (GetTokenInformation(token, TokenUser, buffer.data(), size, &size)) {
                    TOKEN_USER* user = reinterpret_cast<TOKEN_USER*>(buffer.data());
                    LPSTR sid_str = nullptr;
                    if (ConvertSidToStringSidA(user->User.Sid, &sid_str) && sid_str) {
                        binding += sid_str;
                        LocalFree(sid_str);
                    }
                }
            }
            CloseHandle(token);
        }

        HKEY key = nullptr;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ | KEY_WOW64_64KEY, &key) == ERROR_SUCCESS) {
            char machine_guid[128] = {};
            DWORD size = sizeof(machine_guid);
            if (RegQueryValueExA(key, "MachineGuid", nullptr, nullptr,
                reinterpret_cast<LPBYTE>(machine_guid), &size) == ERROR_SUCCESS) {
                binding += '|';
                binding += machine_guid;
            }
            RegCloseKey(key);
        }

        if (binding.empty())
            return "UNKNOWN_HWID";

        BCRYPT_ALG_HANDLE hAlg = nullptr;
        BCRYPT_HASH_HANDLE hHash = nullptr;
        uint8_t digest[32] = {};
        if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0) != 0)
            return "UNKNOWN_HWID";
        if (BCryptCreateHash(hAlg, &hHash, nullptr, 0, nullptr, 0, 0) != 0) {
            BCryptCloseAlgorithmProvider(hAlg, 0);
            return "UNKNOWN_HWID";
        }

        if (BCryptHashData(hHash, reinterpret_cast<PUCHAR>(binding.data()),
            static_cast<ULONG>(binding.size()), 0) != 0 ||
            BCryptFinishHash(hHash, digest, sizeof(digest), 0) != 0) {
            BCryptDestroyHash(hHash);
            BCryptCloseAlgorithmProvider(hAlg, 0);
            SecureZeroMemory(digest, sizeof(digest));
            SecureZeroMemory(binding.data(), binding.size());
            return "UNKNOWN_HWID";
        }

        static const char hex[] = "0123456789abcdef";
        std::string hwid;
        hwid.reserve(32);
        for (size_t i = 0; i < 16; ++i) {
            hwid.push_back(hex[(digest[i] >> 4) & 0xF]);
            hwid.push_back(hex[digest[i] & 0xF]);
        }

        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        SecureZeroMemory(digest, sizeof(digest));
        SecureZeroMemory(binding.data(), binding.size());
        return hwid;
    }

    inline bool verify_hmac_sha256(const std::string& key, const std::string& message, const std::string& expected_hex) {
        if (expected_hex.size() != 64 || key.empty())
            return false;

        BCRYPT_ALG_HANDLE hAlg = nullptr;
        BCRYPT_HASH_HANDLE hHash = nullptr;
        uint8_t digest[32] = {};
        bool ok = false;

        if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, BCRYPT_ALG_HANDLE_HMAC_FLAG) == 0) {
            if (BCryptCreateHash(hAlg, &hHash, nullptr, 0,
                reinterpret_cast<PUCHAR>(const_cast<char*>(key.data())),
                static_cast<ULONG>(key.size()), 0) == 0) {
                if (BCryptHashData(hHash, reinterpret_cast<PUCHAR>(const_cast<char*>(message.data())),
                    static_cast<ULONG>(message.size()), 0) == 0 &&
                    BCryptFinishHash(hHash, digest, sizeof(digest), 0) == 0) {
                    static const char hex[] = "0123456789abcdef";
                    uint8_t diff = 0;
                    for (size_t i = 0; i < 32; ++i) {
                        char hi = static_cast<char>(std::tolower(static_cast<unsigned char>(expected_hex[i * 2])));
                        char lo = static_cast<char>(std::tolower(static_cast<unsigned char>(expected_hex[i * 2 + 1])));
                        diff |= static_cast<uint8_t>(hi ^ hex[(digest[i] >> 4) & 0xF]);
                        diff |= static_cast<uint8_t>(lo ^ hex[digest[i] & 0xF]);
                    }
                    ok = (diff == 0);
                }
                BCryptDestroyHash(hHash);
            }
            BCryptCloseAlgorithmProvider(hAlg, 0);
        }
        SecureZeroMemory(digest, sizeof(digest));
        return ok;
    }

    inline bool find_top_level_json_value(const std::string& json, const std::string& target_key, size_t& val_start) {
        size_t i = 0;
        const size_t n = json.size();
        auto skip_ws = [&]() {
            while (i < n && (json[i] == ' ' || json[i] == '\t' || json[i] == '\r' || json[i] == '\n'))
                ++i;
        };
        auto parse_string = [&](std::string& out) -> bool {
            if (i >= n || json[i] != '"')
                return false;
            ++i;
            out.clear();
            while (i < n) {
                char c = json[i++];
                if (c == '\\' && i < n) {
                    out.push_back(json[i++]);
                }
                else if (c == '"') {
                    return true;
                }
                else {
                    out.push_back(c);
                }
            }
            return false;
        };

        skip_ws();
        if (i >= n || json[i] != '{')
            return false;
        ++i;

        int depth = 1;
        bool in_str = false;
        while (i < n && depth > 0) {
            skip_ws();
            if (depth == 1 && i < n && json[i] == '"') {
                std::string k;
                if (!parse_string(k))
                    return false;
                skip_ws();
                if (i >= n || json[i] != ':')
                    return false;
                ++i;
                skip_ws();
                if (k == target_key) {
                    val_start = i;
                    return true;
                }
                if (i < n && json[i] == '"') {
                    std::string dummy;
                    if (!parse_string(dummy))
                        return false;
                }
                else if (i < n && (json[i] == '{' || json[i] == '[')) {
                    int nested = 1;
                    ++i;
                    while (i < n && nested > 0) {
                        if (in_str) {
                            if (json[i] == '\\' && i + 1 < n) i += 2;
                            else if (json[i] == '"') { in_str = false; ++i; }
                            else ++i;
                        }
                        else {
                            if (json[i] == '"') in_str = true;
                            else if (json[i] == '{' || json[i] == '[') ++nested;
                            else if (json[i] == '}' || json[i] == ']') --nested;
                            ++i;
                        }
                    }
                }
                else {
                    while (i < n && json[i] != ',' && json[i] != '}')
                        ++i;
                }
                skip_ws();
                if (i < n && json[i] == ',')
                    ++i;
                else if (i < n && json[i] == '}')
                    --depth;
            }
            else {
                if (json[i] == '}') --depth;
                ++i;
            }
        }
        return false;
    }

    inline std::string extract_json_string(const std::string& json, const std::string& key) {
        size_t pos = 0;
        if (!find_top_level_json_value(json, key, pos))
            return "";
        if (pos >= json.size() || json[pos] != '"')
            return "";
        ++pos;
        std::string result;
        for (size_t i = pos; i < json.size(); ++i) {
            if (json[i] == '\\' && i + 1 < json.size()) {
                result.push_back(json[i + 1]);
                ++i;
            }
            else if (json[i] == '"') {
                break;
            }
            else {
                result.push_back(json[i]);
            }
        }
        return result;
    }

    inline bool extract_json_bool(const std::string& json, const std::string& key) {
        size_t pos = 0;
        if (!find_top_level_json_value(json, key, pos))
            return false;
        return json.compare(pos, 4, "true") == 0;
    }

    class c_api {
    public:
        std::string name;
        std::string ownerid;
        std::string version;
        std::string url;
        std::string path;
        response_t response;

        c_api(std::string name, std::string ownerid, std::string version, std::string url, std::string path)
            : name(std::move(name)), ownerid(std::move(ownerid)), version(std::move(version)),
              url(std::move(url)), path(std::move(path)), m_session_mask(static_cast<uint8_t>(GetCurrentProcessId() ^ __rdtsc() ^ 0xA7)) {
            if (!m_session_mask) m_session_mask = 0x5D;
        }

        ~c_api() {
            clear_session_id();
        }

        bool init() {
            std::string body = "type=init&ver=" + url_encode(version) +
                               "&name=" + url_encode(name) +
                               "&ownerid=" + url_encode(ownerid);

            std::string raw = post(body);
            if (raw.empty()) {
                response.success = false;
                response.message = "Failed to connect to auth server.";
                return false;
            }

            response.success = extract_json_bool(raw, "success");
            response.message = extract_json_string(raw, "message");
            std::string sid = extract_json_string(raw, "sessionid");
            set_session_id(sid);
            SecureZeroMemory(sid.data(), sid.size());
            return response.success;
        }

        bool license(const std::string& key) {
            std::string hwid = get_hwid();
            if (hwid == "UNKNOWN_HWID") {
                response.success = false;
                response.message = "Blocked: device identity unavailable.";
                return false;
            }
            std::string sid = get_session_id();
            std::string body = "type=license&key=" + url_encode(key) +
                               "&hwid=" + url_encode(hwid) +
                               "&sessionid=" + url_encode(sid) +
                               "&name=" + url_encode(name) +
                               "&ownerid=" + url_encode(ownerid);
            SecureZeroMemory(sid.data(), sid.size());

            std::string raw = post(body);
            if (raw.empty()) {
                response.success = false;
                response.message = "Failed to connect to auth server.";
                return false;
            }

            response.success = extract_json_bool(raw, "success");
            response.message = extract_json_string(raw, "message");
            return response.success;
        }

    private:
        std::vector<uint8_t> m_enc_sessionid;
        uint8_t m_session_mask = 0x5D;

        void clear_session_id() {
            if (!m_enc_sessionid.empty()) {
                SecureZeroMemory(m_enc_sessionid.data(), m_enc_sessionid.size());
                m_enc_sessionid.clear();
            }
        }

        void set_session_id(const std::string& sid) {
            clear_session_id();
            m_enc_sessionid.resize(sid.size());
            for (size_t i = 0; i < sid.size(); ++i)
                m_enc_sessionid[i] = static_cast<uint8_t>(sid[i]) ^ static_cast<uint8_t>(m_session_mask + (i * 13));
        }

        std::string get_session_id() const {
            std::string sid(m_enc_sessionid.size(), '\0');
            for (size_t i = 0; i < m_enc_sessionid.size(); ++i)
                sid[i] = static_cast<char>(m_enc_sessionid[i] ^ static_cast<uint8_t>(m_session_mask + (i * 13)));
            return sid;
        }

        std::string post(const std::string& data) {
            std::wstring wurl(url.begin(), url.end());
            URL_COMPONENTS url_comp{};
            url_comp.dwStructSize = sizeof(url_comp);

            wchar_t host[256]{};
            wchar_t url_path[512]{};
            url_comp.lpszHostName = host;
            url_comp.dwHostNameLength = _countof(host);
            url_comp.lpszUrlPath = url_path;
            url_comp.dwUrlPathLength = _countof(url_path);

            if (!WinHttpCrackUrl(wurl.c_str(), static_cast<DWORD>(wurl.length()), 0, &url_comp))
                return "";

            auto ua = encrypt(L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");
            HINTERNET h_session = WinHttpOpen(ua.decrypt(), WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                                              WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
            if (!h_session) {
                h_session = WinHttpOpen(ua.decrypt(), WINHTTP_ACCESS_TYPE_NO_PROXY,
                                        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
            }
            if (!h_session)
                return "";

            HINTERNET h_connect = WinHttpConnect(h_session, host, url_comp.nPort, 0);
            if (!h_connect) {
                WinHttpCloseHandle(h_session);
                return "";
            }

            DWORD flags = (url_comp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
            HINTERNET h_request = WinHttpOpenRequest(h_connect, L"POST", url_path, nullptr,
                                                     WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
            if (!h_request) {
                WinHttpCloseHandle(h_connect);
                WinHttpCloseHandle(h_session);
                return "";
            }
            WinHttpSetTimeouts(h_request, 5000, 5000, 10000, 10000);

            const wchar_t* headers = L"Content-Type: application/x-www-form-urlencoded\r\n";
            BOOL sent = WinHttpSendRequest(
                h_request,
                headers,
                static_cast<DWORD>(wcslen(headers)),
                const_cast<char*>(data.data()),
                static_cast<DWORD>(data.size()),
                static_cast<DWORD>(data.size()),
                0
            );

            std::string result;
            if (sent && WinHttpReceiveResponse(h_request, nullptr)) {
                DWORD avail = 0;
                while (WinHttpQueryDataAvailable(h_request, &avail) && avail > 0) {
                    std::vector<char> chunk(avail);
                    DWORD read_bytes = 0;
                    if (WinHttpReadData(h_request, chunk.data(), avail, &read_bytes) && read_bytes > 0) {
                        result.append(chunk.data(), read_bytes);
                    }
                }
            }

            WinHttpCloseHandle(h_request);
            WinHttpCloseHandle(h_connect);
            WinHttpCloseHandle(h_session);
            return result;
        }
    };
}
