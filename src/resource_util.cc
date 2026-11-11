// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// resource_util.cc — Resource loading utilities.

#include "resource_util.h"

#include <windows.h>
#include <shlwapi.h>

#include <algorithm>
#include <sstream>
#include <iomanip>

namespace ResourceUtil {

std::string GetDataURI(const std::string& data,
                       const std::string& mime_type) {
  // Encode the data as a data: URI.
  // This lets us load HTML content directly without needing a file or server.
  return "data:" + mime_type + ";base64," +
         // Simple base64 encoding
         [](const std::string& input) -> std::string {
           static const char kBase64Chars[] =
               "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
               "0123456789+/";
           std::string output;
           int i = 0;
           unsigned char bytes[3];
           unsigned char encoded[4];

           for (size_t pos = 0; pos < input.size(); pos++) {
             bytes[i++] = static_cast<unsigned char>(input[pos]);
             if (i == 3) {
               encoded[0] = (bytes[0] & 0xfc) >> 2;
               encoded[1] = ((bytes[0] & 0x03) << 4) |
                            ((bytes[1] & 0xf0) >> 4);
               encoded[2] = ((bytes[1] & 0x0f) << 2) |
                            ((bytes[2] & 0xc0) >> 6);
               encoded[3] = bytes[2] & 0x3f;
               for (int j = 0; j < 4; j++) {
                 output += kBase64Chars[encoded[j]];
               }
               i = 0;
             }
           }

           if (i) {
             for (int j = i; j < 3; j++) {
               bytes[j] = 0;
             }
             encoded[0] = (bytes[0] & 0xfc) >> 2;
             encoded[1] = ((bytes[0] & 0x03) << 4) |
                          ((bytes[1] & 0xf0) >> 4);
             encoded[2] = ((bytes[1] & 0x0f) << 2) |
                          ((bytes[2] & 0xc0) >> 6);
             encoded[3] = bytes[2] & 0x3f;
             for (int j = 0; j < i + 1; j++) {
               output += kBase64Chars[encoded[j]];
             }
             while (i++ < 3) {
               output += '=';
             }
           }

           return output;
         }(data);
}

std::string GetExecutableDir() {
  wchar_t path[MAX_PATH];
  GetModuleFileNameW(nullptr, path, MAX_PATH);
  PathRemoveFileSpecW(path);

  // Convert wide string to UTF-8
  int size = WideCharToMultiByte(CP_UTF8, 0, path, -1, nullptr, 0,
                                  nullptr, nullptr);
  std::string result(size - 1, 0);
  WideCharToMultiByte(CP_UTF8, 0, path, -1, &result[0], size,
                       nullptr, nullptr);
  return result;
}

std::string GetUIResourcePath(const std::string& relative_path) {
  return GetExecutableDir() + "\\" + relative_path;
}

}  // namespace ResourceUtil

