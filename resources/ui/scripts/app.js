/**
 * Softvim Browser — Master UI Controller
 * Google Search Integration, Smart URL Processing, Tab Management,
 * Settings Persistence, Bookmarks, History & Shortcuts.
 */

// Search Engine Registry
const SearchEngines = {
    google: {
        name: 'Google',
        url: 'https://www.google.com/search?q=',
        placeholder: 'Search Google or enter URL'
    },
    bing: {
        name: 'Bing',
        url: 'https://www.bing.com/search?q=',
        placeholder: 'Search Bing or enter URL'
    },
    duckduckgo: {
        name: 'DuckDuckGo',
        url: 'https://duckduckgo.com/?q=',
        placeholder: 'Search DuckDuckGo or enter URL'
    }
};

let currentSearchEngine = 'google';
let currentSidebarMode = null;

// ──────────────────────────────────────────────
// Host Communication Wrapper (CEF Query Router)
// ──────────────────────────────────────────────
const NativeHost = {
    navigate: function(url) {
        if (!url) return;
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'navigate', url: url }),
                onSuccess: function() {},
                onFailure: function() {}
            });
        }
    },
    goBack: function() {
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'back' }),
                onSuccess: function() {},
                onFailure: function() {}
            });
        }
    },
    goForward: function() {
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'forward' }),
                onSuccess: function() {},
                onFailure: function() {}
            });
        }
    },
    reload: function() {
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'reload' }),
                onSuccess: function() {},
                onFailure: function() {}
            });
        }
    },
    newTab: function(isPrivate = false) {
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'new_tab', is_private: isPrivate }),
                onSuccess: function() {},
                onFailure: function() {}
            });
        }
    },
    switchTab: function(tabId) {
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'switch_tab', id: tabId }),
                onSuccess: function() {},
                onFailure: function() {}
            });
        }
    },
    closeTab: function(tabId) {
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'close_tab', id: tabId }),
                onSuccess: function() {},
                onFailure: function() {}
            });
        }
    },
    getBookmarks: function(callback) {
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'get_bookmarks' }),
                onSuccess: function(res) {
                    try { callback(JSON.parse(res)); } catch (e) { callback([]); }
                },
                onFailure: function() { callback([]); }
            });
        }
    },
    addBookmark: function(title, url) {
        if (!url) return;
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'add_bookmark', title: title, url: url }),
                onSuccess: function() {
                    if (currentSidebarMode === 'bookmarks') openSidebar('bookmarks');
                },
                onFailure: function() {}
            });
        }
    },
    deleteBookmark: function(id) {
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'delete_bookmark', id: id }),
                onSuccess: function() {
                    if (currentSidebarMode === 'bookmarks') openSidebar('bookmarks');
                },
                onFailure: function() {}
            });
        }
    },
    getHistory: function(callback) {
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'get_history' }),
                onSuccess: function(res) {
                    try { callback(JSON.parse(res)); } catch (e) { callback([]); }
                },
                onFailure: function() { callback([]); }
            });
        }
    },
    clearHistory: function() {
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'clear_history' }),
                onSuccess: function() {
                    if (currentSidebarMode === 'history') openSidebar('history');
                },
                onFailure: function() {}
            });
        }
    },
    getSettings: function(callback) {
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'get_settings' }),
                onSuccess: function(res) {
                    try { callback(JSON.parse(res)); } catch (e) { callback(null); }
                },
                onFailure: function() { callback(null); }
            });
        }
    },
    updateSettings: function(settingsObj) {
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify(Object.assign({ action: 'update_settings' }, settingsObj)),
                onSuccess: function() {},
                onFailure: function() {}
            });
        }
    },
    toggleTheme: function() {
        const html = document.documentElement;
        const currentTheme = html.getAttribute('data-theme') || 'dark';
        const nextTheme = currentTheme === 'dark' ? 'light' : 'dark';
        html.setAttribute('data-theme', nextTheme);
        NativeHost.updateSettings({ theme: nextTheme });
    }
};

// ──────────────────────────────────────────────
// URL Detection & Search Query Formulation
// ──────────────────────────────────────────────
function isLikelyUrl(text) {
    if (!text) return false;
    // Protocol already present
    if (/^[a-zA-Z][a-zA-Z0-9+.-]*:\/\//i.test(text)) return true;
    // Localhost or loopback
    if (/^localhost(:\d+)?(\/.*)?$/i.test(text)) return true;
    if (/^127\.0\.0\.1(:\d+)?(\/.*)?$/.test(text)) return true;
    if (text.startsWith('file:///')) return true;

    // Domain name pattern without spaces: name.tld
    if (!text.includes(' ') && text.includes('.')) {
        // Must match standard domain structure
        const domainPattern = /^[a-zA-Z0-9-]+(\.[a-zA-Z0-9-]+)+(:[0-9]+)?(\/.*|\?.*)?$/;
        if (domainPattern.test(text)) return true;
    }
    return false;
}

function resolveNavigationTarget(rawText) {
    const query = rawText.trim();
    if (!query) return null;

    // 1. Direct valid URL
    if (isLikelyUrl(query)) {
        if (!/^[a-zA-Z][a-zA-Z0-9+.-]*:\/\//i.test(query)) {
            return 'https://' + query;
        }
        return query;
    }

    // 2. Search Query (supports English, Hindi, and all Unicode)
    const engine = SearchEngines[currentSearchEngine] || SearchEngines.google;
    const encoded = encodeURIComponent(query);
    return engine.url + encoded;
}

// ──────────────────────────────────────────────
// UI Initialization
// ──────────────────────────────────────────────
document.addEventListener('DOMContentLoaded', () => {
    const addressInput = document.getElementById('address-input');
    const btnBack = document.getElementById('btn-back');
    const btnForward = document.getElementById('btn-forward');
    const btnReload = document.getElementById('btn-reload');
    const btnTheme = document.getElementById('btn-theme');
    const btnNewTab = document.querySelector('.new-tab-btn');
    const btnPrivate = document.getElementById('btn-private');
    const btnBookmarks = document.getElementById('btn-bookmarks');
    const btnHistory = document.getElementById('btn-history');
    const btnSettings = document.getElementById('btn-settings');
    const btnAddBookmark = document.getElementById('btn-add-bookmark');
    const sidebarClose = document.getElementById('sidebar-close');

    // Load persisted settings
    NativeHost.getSettings((settings) => {
        if (settings) {
            if (settings.search_engine_name && SearchEngines[settings.search_engine_name]) {
                setSearchEngine(settings.search_engine_name, false);
            }
            if (settings.theme) {
                document.documentElement.setAttribute('data-theme', settings.theme);
            }
        }
    });

    if (btnBack) btnBack.addEventListener('click', () => NativeHost.goBack());
    if (btnForward) btnForward.addEventListener('click', () => NativeHost.goForward());
    if (btnReload) btnReload.addEventListener('click', () => NativeHost.reload());
    if (btnTheme) btnTheme.addEventListener('click', () => NativeHost.toggleTheme());
    if (btnNewTab) btnNewTab.addEventListener('click', () => NativeHost.newTab(false));
    if (btnPrivate) btnPrivate.addEventListener('click', () => NativeHost.newTab(true));

    if (btnBookmarks) btnBookmarks.addEventListener('click', () => toggleSidebar('bookmarks'));
    if (btnHistory) btnHistory.addEventListener('click', () => toggleSidebar('history'));
    if (btnSettings) btnSettings.addEventListener('click', () => toggleSidebar('settings'));
    if (sidebarClose) sidebarClose.addEventListener('click', () => closeSidebar());

    if (btnAddBookmark) {
        btnAddBookmark.addEventListener('click', () => {
            const url = addressInput ? addressInput.value : '';
            const activeTab = document.querySelector('.tab.active .tab-title');
            const title = activeTab ? activeTab.textContent : url;
            if (url) NativeHost.addBookmark(title, url);
        });
    }

    if (addressInput) {
        addressInput.addEventListener('keydown', (e) => {
            if (e.key === 'Enter') {
                const target = resolveNavigationTarget(addressInput.value);
                if (target) {
                    NativeHost.navigate(target);
                    addressInput.blur();
                }
            }
        });

        addressInput.addEventListener('focus', () => addressInput.select());
    }

    // Keyboard Shortcuts
    window.addEventListener('keydown', (e) => {
        if (e.ctrlKey && !e.shiftKey && e.key.toLowerCase() === 't') {
            e.preventDefault();
            NativeHost.newTab(false);
        } else if (e.ctrlKey && e.shiftKey && e.key.toLowerCase() === 'p') {
            e.preventDefault();
            NativeHost.newTab(true);
        } else if (e.ctrlKey && e.key.toLowerCase() === 'w') {
            e.preventDefault();
            const activeTab = document.querySelector('.tab.active');
            if (activeTab) {
                NativeHost.closeTab(parseInt(activeTab.getAttribute('data-tab-id')));
            }
        } else if (e.ctrlKey && e.key.toLowerCase() === 'l') {
            e.preventDefault();
            if (addressInput) {
                addressInput.focus();
                addressInput.select();
            }
        } else if ((e.ctrlKey && e.key.toLowerCase() === 'r') || e.key === 'F5') {
            e.preventDefault();
            NativeHost.reload();
        } else if (e.altKey && e.key === 'ArrowLeft') {
            e.preventDefault();
            NativeHost.goBack();
        } else if (e.altKey && e.key === 'ArrowRight') {
            e.preventDefault();
            NativeHost.goForward();
        } else if (e.ctrlKey && e.key.toLowerCase() === 'b') {
            e.preventDefault();
            toggleSidebar('bookmarks');
        } else if (e.ctrlKey && e.key.toLowerCase() === 'h') {
            e.preventDefault();
            toggleSidebar('history');
        } else if (e.ctrlKey && e.key.toLowerCase() === 'd') {
            e.preventDefault();
            const url = addressInput ? addressInput.value : '';
            const activeTab = document.querySelector('.tab.active .tab-title');
            const title = activeTab ? activeTab.textContent : url;
            if (url) NativeHost.addBookmark(title, url);
        } else if (e.key === 'Escape') {
            closeSidebar();
        }
    });

    bindTabEvents();
});

function setSearchEngine(name, persist = true) {
    if (!SearchEngines[name]) return;
    currentSearchEngine = name;
    const addressInput = document.getElementById('address-input');
    if (addressInput) {
        addressInput.placeholder = SearchEngines[name].placeholder;
    }
    if (persist) {
        NativeHost.updateSettings({
            search_engine_name: name,
            search_engine: SearchEngines[name].url
        });
    }
}

function bindTabEvents() {
    document.querySelectorAll('.tab').forEach(tab => {
        const tabId = parseInt(tab.getAttribute('data-tab-id'));
        tab.onclick = (e) => {
            if (e.target.closest('.tab-close')) {
                e.stopPropagation();
                NativeHost.closeTab(tabId);
            } else {
                NativeHost.switchTab(tabId);
            }
        };
    });
}

// ──────────────────────────────────────────────
// Sidebar Panels
// ──────────────────────────────────────────────
function toggleSidebar(mode) {
    if (currentSidebarMode === mode) {
        closeSidebar();
    } else {
        openSidebar(mode);
    }
}

function openSidebar(mode) {
    currentSidebarMode = mode;
    const sidebar = document.getElementById('sidebar-panel');
    const title = document.getElementById('sidebar-title');
    const content = document.getElementById('sidebar-content');
    if (!sidebar || !title || !content) return;

    sidebar.classList.add('open');

    if (mode === 'bookmarks') {
        title.textContent = 'Bookmarks';
        content.innerHTML = '<div style="color:var(--text-secondary); padding:var(--space-2);">Loading...</div>';
        NativeHost.getBookmarks((items) => {
            renderPanelItems(content, items, (item) => {
                NativeHost.navigate(item.url);
                closeSidebar();
            }, (id) => NativeHost.deleteBookmark(id));
        });
    } else if (mode === 'history') {
        title.textContent = 'History';
        content.innerHTML = '<div style="color:var(--text-secondary); padding:var(--space-2);">Loading...</div>';
        NativeHost.getHistory((items) => {
            renderPanelItems(content, items, (item) => {
                NativeHost.navigate(item.url);
                closeSidebar();
            }, null);
        });
    } else if (mode === 'settings') {
        title.textContent = 'Settings';
        renderSettings(content);
    }
}

function closeSidebar() {
    currentSidebarMode = null;
    const sidebar = document.getElementById('sidebar-panel');
    if (sidebar) sidebar.classList.remove('open');
}

function renderSettings(container) {
    container.innerHTML = `
        <div class="settings-section">
            <span class="settings-section-title">Default Search Engine</span>
            <div class="settings-card">
                <label class="settings-option">
                    <div class="settings-option-label">
                        <span class="settings-option-name">Google (Recommended)</span>
                        <span class="settings-option-desc">Fast, accurate global results</span>
                    </div>
                    <input type="radio" class="settings-radio" name="search-engine" value="google" ${currentSearchEngine === 'google' ? 'checked' : ''}>
                </label>
                <label class="settings-option">
                    <div class="settings-option-label">
                        <span class="settings-option-name">Bing</span>
                        <span class="settings-option-desc">Microsoft search engine</span>
                    </div>
                    <input type="radio" class="settings-radio" name="search-engine" value="bing" ${currentSearchEngine === 'bing' ? 'checked' : ''}>
                </label>
                <label class="settings-option">
                    <div class="settings-option-label">
                        <span class="settings-option-name">DuckDuckGo</span>
                        <span class="settings-option-desc">Privacy-first tracker protection</span>
                    </div>
                    <input type="radio" class="settings-radio" name="search-engine" value="duckduckgo" ${currentSearchEngine === 'duckduckgo' ? 'checked' : ''}>
                </label>
            </div>
        </div>

        <div class="settings-section">
            <span class="settings-section-title">Appearance</span>
            <div class="settings-card">
                <div class="settings-option" id="setting-toggle-theme">
                    <div class="settings-option-label">
                        <span class="settings-option-name">Toggle Dark / Light Theme</span>
                        <span class="settings-option-desc">Apple Safari aesthetic styling</span>
                    </div>
                </div>
            </div>
        </div>

        <div class="settings-section" style="margin-top:auto; padding:0 8px; opacity:0.6; font-size:11px; text-align:center;">
            Softvim Browser v0.1.0 • Powered by Chromium CEF
        </div>
    `;

    container.querySelectorAll('input[name="search-engine"]').forEach(radio => {
        radio.addEventListener('change', (e) => {
            if (e.target.checked) {
                setSearchEngine(e.target.value, true);
            }
        });
    });

    const themeToggle = container.querySelector('#setting-toggle-theme');
    if (themeToggle) {
        themeToggle.addEventListener('click', () => NativeHost.toggleTheme());
    }
}

function renderPanelItems(container, items, onClick, onDelete) {
    if (!items || items.length === 0) {
        container.innerHTML = '<div style="color:var(--text-secondary); text-align:center; padding:var(--space-4);">No items found</div>';
        return;
    }
    container.innerHTML = '';
    items.forEach(item => {
        const row = document.createElement('div');
        row.className = 'panel-item';
        row.innerHTML = `
            <div class="panel-item-left">
                <span class="panel-item-title">${item.title || item.url}</span>
                <span class="panel-item-subtitle">${item.url}</span>
            </div>
            ${onDelete ? `<button class="panel-item-delete" title="Delete">
                <svg viewBox="0 0 24 24" width="14" height="14" stroke="currentColor" stroke-width="2" fill="none">
                    <line x1="18" y1="6" x2="6" y2="18"></line>
                    <line x1="6" y1="6" x2="18" y2="18"></line>
                </svg>
            </button>` : ''}
        `;
        row.querySelector('.panel-item-left').onclick = () => onClick(item);
        if (onDelete) {
            const delBtn = row.querySelector('.panel-item-delete');
            if (delBtn) {
                delBtn.onclick = (e) => {
                    e.stopPropagation();
                    onDelete(item.id);
                };
            }
        }
        container.appendChild(row);
    });
}

// ──────────────────────────────────────────────
// Host State Updates
// ──────────────────────────────────────────────
window.onTabCreated = function(tabData) {
    const tabBar = document.querySelector('.tab-bar');
    const newTabBtn = document.querySelector('.new-tab-btn');
    document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));

    const isPrivate = tabData.title && tabData.title.includes('Private');

    const tabElem = document.createElement('div');
    tabElem.className = 'tab active' + (isPrivate ? ' private-tab' : '');
    tabElem.setAttribute('data-tab-id', tabData.id);
    tabElem.innerHTML = `
        <svg class="tab-favicon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            ${isPrivate ? '<path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"></path>' : '<circle cx="12" cy="12" r="10"></circle><polygon points="16.24 7.76 14.12 14.12 7.76 16.24 9.88 9.88 16.24 7.76"></polygon>'}
        </svg>
        <span class="tab-title">${tabData.title || 'New Tab'}</span>
        <span class="tab-close" title="Close Tab (Ctrl+W)">
            <svg viewBox="0 0 24 24" width="10" height="10" stroke="currentColor" stroke-width="2.5" fill="none">
                <line x1="18" y1="6" x2="6" y2="18"></line>
                <line x1="6" y1="6" x2="18" y2="18"></line>
            </svg>
        </span>
    `;

    tabBar.insertBefore(tabElem, newTabBtn);
    bindTabEvents();
};

window.onTabClosed = function(closedId, newActiveId) {
    const tabToClose = document.querySelector(`.tab[data-tab-id="${closedId}"]`);
    if (tabToClose) tabToClose.remove();

    if (newActiveId !== -1) {
        document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
        const newActive = document.querySelector(`.tab[data-tab-id="${newActiveId}"]`);
        if (newActive) newActive.classList.add('active');
    }
};

window.onTabSwitched = function(state) {
    document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
    const targetTab = document.querySelector(`.tab[data-tab-id="${state.id}"]`);
    if (targetTab) targetTab.classList.add('active');

    const addressInput = document.getElementById('address-input');
    if (addressInput) addressInput.value = state.url;

    const btnBack = document.getElementById('btn-back');
    const btnForward = document.getElementById('btn-forward');
    if (btnBack) btnBack.disabled = !state.canGoBack;
    if (btnForward) btnForward.disabled = !state.canGoForward;
};

window.updateBrowserState = function(state) {
    const activeTab = document.querySelector('.tab.active');
    if (activeTab) {
        const activeId = parseInt(activeTab.getAttribute('data-tab-id'));
        if (state.tabId === activeId) {
            if (state.url) {
                const addressInput = document.getElementById('address-input');
                if (addressInput && document.activeElement !== addressInput) {
                    addressInput.value = state.url;
                }
            }
            if (state.title) {
                const titleSpan = activeTab.querySelector('.tab-title');
                if (titleSpan) titleSpan.textContent = state.title;
            }
            if (typeof state.canGoBack !== 'undefined') {
                const btnBack = document.getElementById('btn-back');
                if (btnBack) btnBack.disabled = !state.canGoBack;
            }
            if (typeof state.canGoForward !== 'undefined') {
                const btnForward = document.getElementById('btn-forward');
                if (btnForward) btnForward.disabled = !state.canGoForward;
            }
        }
    }
};
