#include "ui_element.h"

#include "utils.h"

HINSTANCE ui_element::_instance = nullptr;

std::wstring ui_element::load_wstring(int resource) {
    union {
        LPCWSTR str;
        WCHAR buf[sizeof(str) / sizeof(WCHAR)];
    } pun { };

    int length = LoadStringW(GetModuleHandleW(nullptr), resource, pun.buf, 0) + 1;
    std::wstring str(length, L'\0');
    wcsncpy_s(str.data(), length, pun.str, length - 1i64);

    return str;
}

icon ui_element::load_icon(int resource) {
    return icon(LoadIconW(instance(), MAKEINTRESOURCEW(resource)), true);
}

HGDIOBJ ui_element::stock_object(int obj) {
    return GetStockObject(obj);
}

void ui_element::set_instance(HINSTANCE instance) {
    _instance = instance;
}

HINSTANCE ui_element::instance() {
    if (!_instance) {
        _instance = GetModuleHandleW(nullptr);
    }

    return _instance;
}

HWND ui_element::create_window(const std::wstring& class_name, const std::wstring& window_name, DWORD style, const ::dimensions& at, ui_element* parent, void* param) {
    return create_window_ex(class_name, window_name, style, at, parent, 0, param);
}

HWND ui_element::create_window_ex(const std::wstring& class_name, const std::wstring& window_name, DWORD style, const ::dimensions& at, ui_element* parent, DWORD ex_style, void* param) {
    return CreateWindowExW(ex_style, class_name.c_str(), window_name.c_str(), style,
        at.x, at.y, at.width, at.height, parent ? parent->handle() : nullptr, nullptr, instance(), param);
}


ui_element::ui_element(ui_element* parent)
    : _m_parent { parent }
    , _m_handle { }
    , _m_created { } {

}

ui_element::~ui_element() {
    destroy();
}

bool ui_element::destroy() {
    HWND handle = _m_handle;
    _m_handle = nullptr;
    _m_created = false;
    return DestroyWindow(handle);
}

ui_element* ui_element::parent() const {
    return _m_parent;
}

HWND ui_element::handle() const {
    return _m_handle;
}

long ui_element::width() const {
    RECT rect;
    bool res = GetWindowRect(handle(), &rect);
    ASSERT(res);

    return rect.right - rect.left;
}

long ui_element::height() const {
    RECT rect;
    bool res = GetWindowRect(handle(), &rect);
    ASSERT(res);

    return rect.bottom - rect.top;
}

coords ui_element::position() const {
    auto [x, y, w, h] = dimensions();

    return { .x = x, .y = y };
}

dimensions ui_element::dimensions() const {
    RECT rect;
    GetClientRect(handle(), &rect);

    return {
        .x = rect.left,
        .y = rect.top,
        .width = rect.right,
        .height = rect.bottom
    };
}

void ui_element::move(int x, int y, int width, int height) {
    SetWindowPos(handle(), nullptr, x, y, width, height, 0);
}

HDWP& ui_element::move_dwp(HDWP& dwp, int x, int y, int width, int height) {
    dwp = DeferWindowPos(dwp, handle(), nullptr, x, y, width, height, 0);

    return dwp;
}

void ui_element::set_style(DWORD style, bool enable) {
    DWORD old_style = DWORD(GetWindowLongPtrW(handle(), GWL_STYLE));

    // Don't re-apply style
    if (enable && !(old_style & style)) {
        SetWindowLongPtrW(handle(), GWL_STYLE, old_style | style);
    } else if (!enable && (old_style & style)) {
        SetWindowLongPtrW(handle(), GWL_STYLE, old_style & ~style);
    }
}

void ui_element::set_enabled(bool enabled) {
    EnableWindow(handle(), enabled);
}

LRESULT ui_element::send_message(UINT msg, WPARAM wparam, LPARAM lparam) const {
    return SendMessageW(handle(), msg, wparam, lparam);
}

bool ui_element::post_message(UINT msg, WPARAM wparam, LPARAM lparam) const {
    return PostMessageW(handle(), msg, wparam, lparam);
}

void ui_element::set_handle(HWND handle) {
    ASSERT(!_m_handle && handle);

    _m_handle = handle;
}

bool ui_element::wm_create(CREATESTRUCTW* create) {
    if (_m_wnd_proc) {
        return _m_wnd_proc(handle(), WM_CREATE, 0, LPARAM(create)) == 0;
    }

    return DefWindowProcW(handle(), WM_CREATE, 0, LPARAM(create)) == 0;
}

void ui_element::wm_destroy() {
    DefWindowProcW(handle(), WM_DESTROY, 0, 0);
}

void ui_element::wm_size(int type, int width, int height) {
    if (_m_wnd_proc) {
        _m_wnd_proc(handle(), WM_SIZE, type, MAKELPARAM(width, height));
    } else {
        DefWindowProcW(handle(), WM_SIZE, type, MAKELPARAM(width, height));
    }
}

void ui_element::wm_paint() {
    // API dislikes no painting being done
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(handle(), &ps);
    (void) hdc;
    EndPaint(handle(), &ps);
}

void ui_element::wm_command(WPARAM wparam, LPARAM lparam) {
    if (_m_wnd_proc) {
        _m_wnd_proc(handle(), WM_COMMAND, wparam, lparam);
    } else {
        DefWindowProcW(handle(), WM_COMMAND, wparam, lparam);
    }
}

ui_element::wnd_init::wnd_init(ui_element* element, const wnd_proc_func& proc, void* replacement)
    : element(element), proc(proc), replacement(replacement){

}
LRESULT ui_element::wnd_proc_fwd(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    // Retrieve instance
    ui_element* _this = reinterpret_cast<ui_element*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (_this) {
        // Predefined handlers
        switch (msg) {
            case WM_DESTROY:
                _this->wm_destroy();
                return 0;
            
            case WM_SIZE:
                _this->wm_size(int(wparam), LOWORD(lparam), HIWORD(lparam));
                return 0;

            case WM_PAINT:
                _this->wm_paint();
                return 0;

            case WM_COMMAND:
                _this->wm_command(wparam, lparam);
                return 0;

            default: break;
        }
        
        // Custom callback is set
        if (_this->_m_wnd_proc) {
            return _this->_m_wnd_proc(hwnd, msg, wparam, lparam);
        }
    }

    // Some default handling if the instance is not yet set or not yet completed
    if (!_this || !_this->_m_created) {
        if (msg == WM_CREATE) {
            CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
            wnd_init* init = static_cast<wnd_init*>(create->lpCreateParams);

            // Initialisation given
            if (init) {
                ui_element* element = init->element;
                if (element) {
                    // Set handle and `this` pointer
                    element->set_handle(hwnd);
                    SetWindowLongPtrW(hwnd, GWLP_USERDATA, LONG_PTR(element));

                    // Set custom callback
                    if (init->proc) {
                        element->_m_wnd_proc = init->proc;
                    }

                    element->_m_created = true;
                }

                create->lpCreateParams = init->replacement;

                delete init;

                if (element) {
                    // Should automatically forward
                    return element->wm_create(create) ? 0 : -1;
                }

                // Call default
            }
        }
    }
    
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}
