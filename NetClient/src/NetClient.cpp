#include "NetClient.h"

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#include <algorithm>
#pragma comment(lib, "winhttp.lib")
#endif

#include <iostream>
#include <sstream>

namespace NetClient {

    std::string Response::header(const std::string& name) const {
        std::string lower_name = name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
        
        for (const auto& h : headers) {
            std::string h_name = h.first;
            std::transform(h_name.begin(), h_name.end(), h_name.begin(), ::tolower);
            if (h_name == lower_name) return h.second;
        }
        return "";
    }

    bool Response::is_json() const {
        std::string ct = header("Content-Type");
        if (ct.find("application/json") != std::string::npos) return true;
        
        // Fallback: check if it looks like JSON
        std::string trimmed = text;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r\f\v"));
        trimmed.erase(trimmed.find_last_not_of(" \t\n\r\f\v") + 1);
        
        if (trimmed.empty()) return false;
        return (trimmed.front() == '{' && trimmed.back() == '}') || 
               (trimmed.front() == '[' && trimmed.back() == ']');
    }

#ifdef _WIN32
    static Response internal_request(const std::string& method,
                                    const std::string& url, 
                                    const std::string& data,
                                    const std::map<std::string, std::string>& headers) {
        
        Response resp;
        resp.url = url;
        resp.status_code = 0;

        HINTERNET hSession = WinHttpOpen(L"NetClient/1.0", 
                                        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                        WINHTTP_NO_PROXY_NAME, 
                                        WINHTTP_NO_PROXY_BYPASS, 0);

        if (hSession) {
            URL_COMPONENTS urlComp = {0};
            urlComp.dwStructSize = sizeof(urlComp);
            urlComp.dwHostNameLength = (DWORD)-1;
            urlComp.dwUrlPathLength = (DWORD)-1;
            urlComp.dwExtraInfoLength = (DWORD)-1;

            wchar_t wUrl[2048];
            MultiByteToWideChar(CP_UTF8, 0, url.c_str(), -1, wUrl, 2048);

            if (WinHttpCrackUrl(wUrl, (DWORD)wcslen(wUrl), 0, &urlComp)) {
                wchar_t szHost[256];
                wcsncpy_s(szHost, urlComp.lpszHostName, urlComp.dwHostNameLength);
                szHost[urlComp.dwHostNameLength] = L'\0';

                HINTERNET hConnect = WinHttpConnect(hSession, szHost, urlComp.nPort, 0);
                if (hConnect) {
                    std::wstring wMethod(method.begin(), method.end());
                    DWORD dwFlags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
                    
                    HINTERNET hRequest = WinHttpOpenRequest(hConnect, wMethod.c_str(), urlComp.lpszUrlPath,
                                                           NULL, WINHTTP_NO_REFERER, 
                                                           WINHTTP_DEFAULT_ACCEPT_TYPES, dwFlags);

                    if (hRequest) {
                        // Add headers
                        for (const auto& head : headers) {
                            std::wstring wHeader = std::wstring(head.first.begin(), head.first.end()) + L": " + 
                                                   std::wstring(head.second.begin(), head.second.end());
                            WinHttpAddRequestHeaders(hRequest, wHeader.c_str(), (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD);
                        }

                        DWORD dwDataSize = (DWORD)data.size();
                        LPVOID lpOptional = dwDataSize > 0 ? (LPVOID)data.c_str() : WINHTTP_NO_REQUEST_DATA;

                        if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, 
                                             lpOptional, dwDataSize, dwDataSize, 0)) {
                            
                            if (WinHttpReceiveResponse(hRequest, NULL)) {
                                // Status Code
                                DWORD dwStatusCode = 0;
                                DWORD dwSize = sizeof(dwStatusCode);
                                WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                                   WINHTTP_HEADER_NAME_BY_INDEX, &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX);
                                resp.status_code = (int)dwStatusCode;

                                // Headers
                                dwSize = 0;
                                WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                                                   WINHTTP_HEADER_NAME_BY_INDEX, NULL, &dwSize, WINHTTP_NO_HEADER_INDEX);
                                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                                    wchar_t* lpHeaders = new wchar_t[dwSize / sizeof(wchar_t)];
                                    if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                                                           WINHTTP_HEADER_NAME_BY_INDEX, lpHeaders, &dwSize, WINHTTP_NO_HEADER_INDEX)) {
                                        std::wstring wAllHeaders(lpHeaders);
                                        std::wstringstream ss(wAllHeaders);
                                        std::wstring line;
                                        while (std::getline(ss, line) && line != L"\r") {
                                            size_t pos = line.find(L":");
                                            if (pos != std::string::npos) {
                                                std::wstring key = line.substr(0, pos);
                                                std::wstring val = line.substr(pos + 1);
                                                // Basic trim and convert to std::string
                                                std::string sKey(key.begin(), key.end());
                                                std::string sVal(val.begin(), val.end());
                                                resp.headers[sKey] = sVal;
                                            }
                                        }
                                    }
                                    delete[] lpHeaders;
                                }

                                // Body
                                DWORD dwDownloaded = 0;
                                do {
                                    dwSize = 0;
                                    if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
                                    if (dwSize == 0) break;

                                    char* pszOutBuffer = new char[dwSize + 1];
                                    if (WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
                                        pszOutBuffer[dwDownloaded] = '\0';
                                        resp.text.append(pszOutBuffer, dwDownloaded);
                                    }
                                    delete[] pszOutBuffer;
                                } while (dwSize > 0);
                            }
                        }
                        WinHttpCloseHandle(hRequest);
                    }
                    WinHttpCloseHandle(hConnect);
                }
            }
            WinHttpCloseHandle(hSession);
        }

        return resp;
    }

    Response get(const std::string& url, 
                const std::map<std::string, std::string>& params,
                const std::map<std::string, std::string>& headers) {
        std::string full_url = url;
        if (!params.empty()) {
            full_url += (url.find('?') == std::string::npos) ? "?" : "&";
            for (auto it = params.begin(); it != params.end(); ++it) {
                if (it != params.begin()) full_url += "&";
                full_url += it->first + "=" + it->second; // Note: Simple encoding
            }
        }
        return internal_request("GET", full_url, "", headers);
    }

    Response post(const std::string& url, const std::string& data, const std::map<std::string, std::string>& headers) {
        return internal_request("POST", url, data, headers);
    }

    Response put(const std::string& url, const std::string& data, const std::map<std::string, std::string>& headers) {
        return internal_request("PUT", url, data, headers);
    }

    Response del(const std::string& url, const std::map<std::string, std::string>& headers) {
        return internal_request("DELETE", url, "", headers);
    }

    Response options(const std::string& url, const std::map<std::string, std::string>& headers) {
        return internal_request("OPTIONS", url, "", headers);
    }

#else
    // Linux/UNIX placeholders
    Response get(const std::string& url, const std::map<std::string, std::string>& params, const std::map<std::string, std::string>& headers) { return {0}; }
    Response post(const std::string& url, const std::string& data, const std::map<std::string, std::string>& headers) { return {0}; }
    Response put(const std::string& url, const std::string& data, const std::map<std::string, std::string>& headers) { return {0}; }
    Response del(const std::string& url, const std::map<std::string, std::string>& headers) { return {0}; }
    Response options(const std::string& url, const std::map<std::string, std::string>& headers) { return {0}; }
#endif

}
