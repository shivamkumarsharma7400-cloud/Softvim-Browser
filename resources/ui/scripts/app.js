/**
 * Softvim Browser — UI Controller
 * Manages tabs, address bar, navigation, and IPC with the C++ host.
 */

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

    if (btnBack) btnBack.addEventListener('click', () => NativeHost.goBack());
    if (btnForward) btnForward.addEventListener('click', () => NativeHost.goForward());
    if (btnReload) btnReload.addEventListener('click', () => NativeHost.reload());
    if (btnTheme) btnTheme.addEventListener('click', () => NativeHost.toggleTheme());
    if (btnNewTab) btnNewTab.addEventListener('click', () => NativeHost.newTab());

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

        addressInput.addEventListener('focus', () => {
            addressInput.select();
        });
    }

    // Attach click listeners to existing tabs
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
// Host Callback Handlers
// ──────────────────────────────────────────────

window.onTabCreated = function(tabData) {
    const tabBar = document.querySelector('.tab-bar');
    const newTabBtn = document.querySelector('.new-tab-btn');

    // Deselect current active tab
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
