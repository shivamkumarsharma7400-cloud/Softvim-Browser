// Copyright (c) 2024 Softvim Solutions. All rights reserved.
// storage_manager.h — Local persistence for bookmarks, history, and settings.

#ifndef SOFTVIM_STORAGE_MANAGER_H_
#define SOFTVIM_STORAGE_MANAGER_H_

#include <string>
#include <vector>
#include <mutex>

struct BookmarkItem {
  std::string id;
  std::string title;
  std::string url;
  int64_t created_at;
};

struct HistoryItem {
  std::string id;
  std::string title;
  std::string url;
  int64_t visited_at;
};

struct DownloadItem {
  std::string id;
  std::string file_name;
  std::string full_path;
  int64_t total_bytes;
  int64_t received_bytes;
  std::string state; // "in_progress", "complete", "cancelled"
  int percent;
};

struct BrowserSettings {
  std::string search_engine = "https://www.google.com/search?q=";
  std::string home_page = "https://www.google.com";
  std::string theme = "dark"; // "dark" or "light"
  bool restore_session = true;
};

class StorageManager {
 public:
  static StorageManager* GetInstance();

  // Bookmarks
  void AddBookmark(const std::string& title, const std::string& url);
  void RemoveBookmark(const std::string& id);
  std::vector<BookmarkItem> GetBookmarks();
  std::string GetBookmarksJson();

  // History
  void AddHistory(const std::string& title, const std::string& url);
  void ClearHistory();
  std::vector<HistoryItem> GetHistory();
  std::string GetHistoryJson();

  // Settings
  BrowserSettings GetSettings();
  void UpdateSettings(const BrowserSettings& settings);
  std::string GetSettingsJson();

  // Downloads tracking
  void AddDownload(const DownloadItem& item);
  void UpdateDownload(const std::string& id, int64_t received, int64_t total, const std::string& state);
  std::string GetDownloadsJson();

 private:
  StorageManager();
  ~StorageManager() = default;

  void Load();
  void Save();

  std::string GetStorageFilePath();

  std::mutex mutex_;
  std::vector<BookmarkItem> bookmarks_;
  std::vector<HistoryItem> history_;
  std::vector<DownloadItem> downloads_;
  BrowserSettings settings_;

  static StorageManager* instance_;
};

#endif  // SOFTVIM_STORAGE_MANAGER_H_
