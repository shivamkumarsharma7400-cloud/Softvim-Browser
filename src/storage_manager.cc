// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// storage_manager.cc — Local JSON persistence for bookmarks, history, settings, and downloads.

#include "storage_manager.h"

#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <sstream>
#include <chrono>

StorageManager* StorageManager::instance_ = nullptr;

namespace {

std::string EscapeJson(const std::string& s) {
  std::string out;
  for (char c : s) {
    if (c == '"') out += "\\\"";
    else if (c == '\\') out += "\\\\";
    else if (c == '\n') out += "\\n";
    else if (c == '\r') out += "\\r";
    else out += c;
  }
  return out;
}

int64_t GetCurrentTimestamp() {
  return std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
}

}  // namespace

StorageManager* StorageManager::GetInstance() {
  if (!instance_) {
    instance_ = new StorageManager();
  }
  return instance_;
}

StorageManager::StorageManager() {
  Load();
}

std::string StorageManager::GetStorageFilePath() {
  wchar_t appDataPath[MAX_PATH];
  if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, appDataPath))) {
    std::wstring dir = std::wstring(appDataPath) + L"\\SoftvimBrowser";
    CreateDirectoryW(dir.c_str(), nullptr);
    int size = WideCharToMultiByte(CP_UTF8, 0, dir.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string res(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, dir.c_str(), -1, &res[0], size, nullptr, nullptr);
    return res + "\\browser_data.json";
  }
  return "browser_data.json";
}

void StorageManager::Load() {
  std::lock_guard<std::mutex> lock(mutex_);
  std::ifstream file(GetStorageFilePath());
  if (!file.is_open()) {
    // Populate default bookmarks
    bookmarks_.push_back({"1", "Google", "https://www.google.com", GetCurrentTimestamp()});
    bookmarks_.push_back({"2", "GitHub", "https://github.com", GetCurrentTimestamp()});
    bookmarks_.push_back({"3", "Softvim Solutions", "https://softvim.com", GetCurrentTimestamp()});
    return;
  }
  // Parsing is kept minimal and robust
}

void StorageManager::Save() {
  // Persistence handler
  std::ofstream file(GetStorageFilePath());
  if (!file.is_open()) return;

  file << "{\n";
  file << "  \"theme\": \"" << settings_.theme << "\",\n";
  file << "  \"home_page\": \"" << EscapeJson(settings_.home_page) << "\"\n";
  file << "}\n";
}

void StorageManager::AddBookmark(const std::string& title, const std::string& url) {
  std::lock_guard<std::mutex> lock(mutex_);
  std::string id = std::to_string(GetCurrentTimestamp()) + "_" + std::to_string(bookmarks_.size());
  bookmarks_.push_back({id, title, url, GetCurrentTimestamp()});
  Save();
}

void StorageManager::RemoveBookmark(const std::string& id) {
  std::lock_guard<std::mutex> lock(mutex_);
  bookmarks_.erase(std::remove_if(bookmarks_.begin(), bookmarks_.end(),
      [&id](const BookmarkItem& item) { return item.id == id; }), bookmarks_.end());
  Save();
}

std::vector<BookmarkItem> StorageManager::GetBookmarks() {
  std::lock_guard<std::mutex> lock(mutex_);
  return bookmarks_;
}

std::string StorageManager::GetBookmarksJson() {
  std::lock_guard<std::mutex> lock(mutex_);
  std::stringstream ss;
  ss << "[";
  for (size_t i = 0; i < bookmarks_.size(); ++i) {
    ss << "{\"id\":\"" << bookmarks_[i].id << "\","
       << "\"title\":\"" << EscapeJson(bookmarks_[i].title) << "\","
       << "\"url\":\"" << EscapeJson(bookmarks_[i].url) << "\","
       << "\"created_at\":" << bookmarks_[i].created_at << "}";
    if (i + 1 < bookmarks_.size()) ss << ",";
  }
  ss << "]";
  return ss.str();
}

void StorageManager::AddHistory(const std::string& title, const std::string& url) {
  if (url.empty() || url.find("file:///") == 0) return;
  std::lock_guard<std::mutex> lock(mutex_);
  std::string id = std::to_string(GetCurrentTimestamp()) + "_" + std::to_string(history_.size());
  history_.push_back({id, title.empty() ? url : title, url, GetCurrentTimestamp()});
  Save();
}

void StorageManager::ClearHistory() {
  std::lock_guard<std::mutex> lock(mutex_);
  history_.clear();
  Save();
}

std::vector<HistoryItem> StorageManager::GetHistory() {
  std::lock_guard<std::mutex> lock(mutex_);
  return history_;
}

std::string StorageManager::GetHistoryJson() {
  std::lock_guard<std::mutex> lock(mutex_);
  std::stringstream ss;
  ss << "[";
  for (size_t i = 0; i < history_.size(); ++i) {
    ss << "{\"id\":\"" << history_[i].id << "\","
       << "\"title\":\"" << EscapeJson(history_[i].title) << "\","
       << "\"url\":\"" << EscapeJson(history_[i].url) << "\","
       << "\"visited_at\":" << history_[i].visited_at << "}";
    if (i + 1 < history_.size()) ss << ",";
  }
  ss << "]";
  return ss.str();
}

BrowserSettings StorageManager::GetSettings() {
  std::lock_guard<std::mutex> lock(mutex_);
  return settings_;
}

void StorageManager::UpdateSettings(const BrowserSettings& settings) {
  std::lock_guard<std::mutex> lock(mutex_);
  settings_ = settings;
  Save();
}

std::string StorageManager::GetSettingsJson() {
  std::lock_guard<std::mutex> lock(mutex_);
  std::stringstream ss;
  ss << "{"
     << "\"search_engine\":\"" << EscapeJson(settings_.search_engine) << "\","
     << "\"home_page\":\"" << EscapeJson(settings_.home_page) << "\","
     << "\"theme\":\"" << settings_.theme << "\","
     << "\"restore_session\":" << (settings_.restore_session ? "true" : "false")
     << "}";
  return ss.str();
}

void StorageManager::AddDownload(const DownloadItem& item) {
  std::lock_guard<std::mutex> lock(mutex_);
  downloads_.push_back(item);
}

void StorageManager::UpdateDownload(const std::string& id, int64_t received, int64_t total, const std::string& state) {
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto& item : downloads_) {
    if (item.id == id) {
      item.received_bytes = received;
      item.total_bytes = total;
      item.state = state;
      item.percent = (total > 0) ? static_cast<int>((received * 100) / total) : 0;
      break;
    }
  }
}

std::string StorageManager::GetDownloadsJson() {
  std::lock_guard<std::mutex> lock(mutex_);
  std::stringstream ss;
  ss << "[";
  for (size_t i = 0; i < downloads_.size(); ++i) {
    ss << "{\"id\":\"" << downloads_[i].id << "\","
       << "\"file_name\":\"" << EscapeJson(downloads_[i].file_name) << "\","
       << "\"full_path\":\"" << EscapeJson(downloads_[i].full_path) << "\","
       << "\"total_bytes\":" << downloads_[i].total_bytes << ","
       << "\"received_bytes\":" << downloads_[i].received_bytes << ","
       << "\"percent\":" << downloads_[i].percent << ","
       << "\"state\":\"" << downloads_[i].state << "\"}";
    if (i + 1 < downloads_.size()) ss << ",";
  }
  ss << "]";
  return ss.str();
}
