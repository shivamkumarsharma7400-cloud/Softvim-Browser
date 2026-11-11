/**
 * Softvim Browser — UI Controller
 * Manages Tabs, Address Bar, Bookmarks, History, Downloads, and Settings.
 */

let currentSidebarMode = null;

const NativeHost = {
    navigate: function(url) {
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
    newTab: function() {
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'new_tab' }),
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
    // Panels & Storage Requests
    getBookmarks: function(callback) {
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'get_bookmarks' }),
                onSuccess: function(response) { callback(JSON.parse(response)); },
                onFailure: function() { callback([]); }
            });
        }
    },
    addBookmark: function(title, url) {
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
                onSuccess: function(response) { callback(JSON.parse(response)); },
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
    getDownloads: function(callback) {
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'get_downloads' }),
                onSuccess: function(response) { callback(JSON.parse(response)); },
                onFailure: function() { callback([]); }
            });
        }
    },
    toggleTheme: function() {
        const html = document.documentElement;
        const currentTheme = html.getAttribute('data-theme') || 'dark';
        const nextTheme = currentTheme === 'dark' ? 'light' : 'dark';
        html.setAttribute('data-theme', nextTheme);
    }
};

document.addEventListener('DOMContentLoaded', () => {
    const addressInput = document.getElementById('address-input');
    const btnBack = document.getElementById('btn-back');
    const btnForward = document.getElementById('btn-forward');
    const btnReload = document.getElementById('btn-reload');
    const btnTheme = document.getElementById('btn-theme');
    const btnNewTab = document.querySelector('.new-tab-btn');
    const btnBookmarks = document.getElementById('btn-bookmarks');
    const btnHistory = document.getElementById('btn-history');
    const btnDownloads = document.getElementById('btn-downloads');
    const btnAddBookmark = document.getElementById('btn-add-bookmark');
    const sidebarClose = document.getElementById('sidebar-close');

    if (btnBack) btnBack.addEventListener('click', () => NativeHost.goBack());
    if (btnForward) btnForward.addEventListener('click', () => NativeHost.goForward());
    if (btnReload) btnReload.addEventListener('click', () => NativeHost.reload());
    if (btnTheme) btnTheme.addEventListener('click', () => NativeHost.toggleTheme());
    if (btnNewTab) btnNewTab.addEventListener('click', () => NativeHost.newTab());

    if (btnBookmarks) btnBookmarks.addEventListener('click', () => toggleSidebar('bookmarks'));
    if (btnHistory) btnHistory.addEventListener('click', () => toggleSidebar('history'));
    if (btnDownloads) btnDownloads.addEventListener('click', () => toggleSidebar('downloads'));
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
                let target = addressInput.value.trim();
                if (!target) return;

                if (!target.startsWith('http://') && !target.startsWith('https://')) {
                    if (target.includes('.') && !target.includes(' ')) {
                        target = 'https://' + target;
                    } else {
                        target = `https://www.google.com/search?q=${encodeURIComponent(target)}`;
                    }
                }
                NativeHost.navigate(target);
                addressInput.blur();
            }
        });

        addressInput.addEventListener('focus', () => addressInput.select());
    }

    bindTabEvents();
});

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
    content.innerHTML = '<div style="color:var(--text-secondary); padding:var(--space-2);">Loading...</div>';

    if (mode === 'bookmarks') {
        title.textContent = 'Bookmarks';
        NativeHost.getBookmarks((items) => {
            renderPanelItems(content, items, (item) => {
                NativeHost.navigate(item.url);
                closeSidebar();
            }, (id) => NativeHost.deleteBookmark(id));
        });
    } else if (mode === 'history') {
        title.textContent = 'History';
        NativeHost.getHistory((items) => {
            renderPanelItems(content, items, (item) => {
                NativeHost.navigate(item.url);
                closeSidebar();
            }, null);
        });
    } else if (mode === 'downloads') {
        title.textContent = 'Downloads';
        NativeHost.getDownloads((items) => {
            renderDownloads(content, items);
        });
    }
}

function closeSidebar() {
    currentSidebarMode = null;
    const sidebar = document.getElementById('sidebar-panel');
    if (sidebar) sidebar.classList.remove('open');
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

function renderDownloads(container, items) {
    if (!items || items.length === 0) {
        container.innerHTML = '<div style="color:var(--text-secondary); text-align:center; padding:var(--space-4);">No downloads yet</div>';
        return;
    }
    container.innerHTML = '';
    items.forEach(item => {
        const row = document.createElement('div');
        row.className = 'panel-item';
        row.innerHTML = `
            <div class="panel-item-left">
                <span class="panel-item-title">${item.file_name}</span>
                <span class="panel-item-subtitle">${item.state} — ${item.percent}%</span>
            </div>
        `;
        container.appendChild(row);
    });
}

// ──────────────────────────────────────────────
// Host Callbacks
// ──────────────────────────────────────────────

window.onTabCreated = function(tabData) {
    const tabBar = document.querySelector('.tab-bar');
    const newTabBtn = document.querySelector('.new-tab-btn');
    document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));

    const tabElem = document.createElement('div');
    tabElem.className = 'tab active';
    tabElem.setAttribute('data-tab-id', tabData.id);
    tabElem.innerHTML = `
        <svg class="tab-favicon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <circle cx="12" cy="12" r="10"></circle>
            <polygon points="16.24 7.76 14.12 14.12 7.76 16.24 9.88 9.88 16.24 7.76"></polygon>
        </svg>
        <span class="tab-title">${tabData.title || 'New Tab'}</span>
        <span class="tab-close">
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
