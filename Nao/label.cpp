#include "label.h"

#include "utils.h"

#include <strings.h>

label::label(ui_element* parent, const std::string& text, label_type type)
    : ui_element(parent, WC_STATICW, win32::style | type) {
    set_font(win32::stock_object<HFONT>(DEFAULT_GUI_FONT));

    set_text(text);
}

dimensions label::text_extent_point() const {
    return ui_element::text_extent_point(_text);
}

void label::set_text(const std::string& text) {
    _text = text;
    SetWindowTextW(handle(), strings::to_utf16(text).c_str());
}

const std::string& label::text() const {
    return _text;
}
