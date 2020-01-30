#include "push_button.h"
#include "utils.h"

push_button::push_button(ui_element* parent) : ui_element(parent) {
    HWND handle = create_window(WC_BUTTON, L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP, { }, parent);

    ASSERT(handle);

    set_handle(handle);
    set_font(stock_object<HFONT>(DEFAULT_GUI_FONT));
}
push_button::push_button(ui_element* parent, const std::string& text) : push_button(parent) {
    set_text(text);
}

push_button::push_button(ui_element* parent, const icon& icon) : push_button(parent) {
    ui_element::set_style(BS_ICON, true);
    set_icon(icon);
}

push_button::push_button(ui_element* parent, const std::string& text, const icon& icon)
    : push_button(parent) {
    set_text(text);
    set_icon(icon);
}


void push_button::set_text(const std::string& text) const {
    send_message(WM_SETTEXT, 0, LPARAM(utils::utf16(text).c_str()));
}

void push_button::set_icon(const icon& icon) const {
    (void) send_message(BM_SETIMAGE, IMAGE_ICON, icon);
}

void push_button::set_font(HFONT font) const {
    send_message(WM_SETFONT, WPARAM(font), 0);
}