#ifndef NETCLIENT_H
#define NETCLIENT_H

#include <string>
#include <map>
#include <vector>

#ifdef _WIN32
    #ifdef NETCLIENT_EXPORTS
        #define NETCLIENT_API __declspec(dllexport)
    #else
        #define NETCLIENT_API __declspec(dllimport)
    #endif
#else
    #define NETCLIENT_API __attribute__((visibility("default")))
#endif

namespace NetClient {

    struct Response {
        int status_code;
        std::string text;
        std::string url;
        std::map<std::string, std::string> headers;
        
        bool ok() const { return status_code >= 200 && status_code < 300; }
        
        NETCLIENT_API std::string header(const std::string& name) const;
        NETCLIENT_API bool is_json() const;
    };

    NETCLIENT_API Response get(const std::string& url, 
                               const std::map<std::string, std::string>& params = {},
                               const std::map<std::string, std::string>& headers = {});

    NETCLIENT_API Response post(const std::string& url, 
                                const std::string& data = "",
                                const std::map<std::string, std::string>& headers = {});

    NETCLIENT_API Response put(const std::string& url, 
                               const std::string& data = "",
                               const std::map<std::string, std::string>& headers = {});

    NETCLIENT_API Response del(const std::string& url, 
                               const std::map<std::string, std::string>& headers = {});

    NETCLIENT_API Response options(const std::string& url, 
                                   const std::map<std::string, std::string>& headers = {});
}

#endif // NETCLIENT_H
