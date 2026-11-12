// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// resource_util.h — Resource loading utilities.

#ifndef SOFTVIM_RESOURCE_UTIL_H_
#define SOFTVIM_RESOURCE_UTIL_H_

#include <string>

namespace ResourceUtil {

// Creates a data: URI from raw content. Useful for inline HTML pages
// (like our error page or new-tab page).
std::string GetDataURI(const std::string& data, const std::string& mime_type);

// Returns the path to the application's executable directory.
std::string GetExecutableDir();

// Returns the full path to a UI resource file (e.g., "ui/index.html").
std::string GetUIResourcePath(const std::string& relative_path);

// Returns the full file:/// URL to the new tab start page.
std::string GetNewTabUrl();

}  // namespace ResourceUtil

#endif  // SOFTVIM_RESOURCE_UTIL_H_

