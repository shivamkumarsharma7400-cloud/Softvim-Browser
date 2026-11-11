/**
 * Softvim Browser — UI Controller
 * Manages UI interactions and communicates with the C++ host application.
 */

// Host communication wrapper
const NativeHost = {
    navigate: function(url) {
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'navigate', url: url }),
                onSuccess: function() {},
                onFailure: function() {}
            });
        } else {
            console.log('[Softvim NativeHost] Navigate to:', url);
        }
    },
    goBack: function() {
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'back' }),
                onSuccess: function() {},
                onFailure: function() {}
            });
        } else {
            console.log('[Softvim NativeHost] Back');
        }
    },
    goForward: function() {
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'forward' }),
                onSuccess: function() {},
                onFailure: function() {}
            });
        } else {
            console.log('[Softvim NativeHost] Forward');
        }
    },
    reload: function() {
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify({ action: 'reload' }),
                onSuccess: function() {},
                onFailure: function() {}
            });
        } else {
            console.log('[Softvim NativeHost] Reload');
        }
    },
    toggleTheme: function() {
        const html = document.documentElement;
        const currentTheme = html.getAttribute('data-theme') || 'dark';
        const nextTheme = currentTheme === 'dark' ? 'light' : 'dark';
        html.setAttribute('data-theme', nextTheme);
    }
};

// UI Initialization
document.addEventListener('DOMContentLoaded', () => {
    const addressInput = document.getElementById('address-input');
    const btnBack = document.getElementById('btn-back');
    const btnForward = document.getElementById('btn-forward');
    const btnReload = document.getElementById('btn-reload');
    const btnTheme = document.getElementById('btn-theme');

    if (btnBack) btnBack.addEventListener('click', () => NativeHost.goBack());
    if (btnForward) btnForward.addEventListener('click', () => NativeHost.goForward());
    if (btnReload) btnReload.addEventListener('click', () => NativeHost.reload());
    if (btnTheme) btnTheme.addEventListener('click', () => NativeHost.toggleTheme());

    if (addressInput) {
        addressInput.addEventListener('keydown', (e) => {
            if (e.key === 'Enter') {
                let target = addressInput.value.trim();
                if (!target) return;

                // Simple URL detection
                if (!target.startsWith('http://') && !target.startsWith('https://')) {
                    if (target.includes('.') && !target.includes(' ')) {
                        target = 'https://' + target;
                    } else {
                        // Default search with Google
                        target = `https://www.google.com/search?q=${encodeURIComponent(target)}`;
                    }
                }
                NativeHost.navigate(target);
                addressInput.blur();
            }
        });

        // Select all text on click for rapid typing
        addressInput.addEventListener('focus', () => {
            addressInput.select();
        });
    }
});

// Updates dispatched from C++ Host
window.updateBrowserState = function(state) {
    if (state.url) {
        const addressInput = document.getElementById('address-input');
        if (addressInput && document.activeElement !== addressInput) {
            addressInput.value = state.url;
        }
    }
    if (state.title) {
        const activeTabTitle = document.querySelector('.tab.active .tab-title');
        if (activeTabTitle) {
            activeTabTitle.textContent = state.title;
        }
    }
    if (typeof state.canGoBack !== 'undefined') {
        const btnBack = document.getElementById('btn-back');
        if (btnBack) btnBack.disabled = !state.canGoBack;
    }
    if (typeof state.canGoForward !== 'undefined') {
        const btnForward = document.getElementById('btn-forward');
        if (btnForward) btnForward.disabled = !state.canGoForward;
    }
};
