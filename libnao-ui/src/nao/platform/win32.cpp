/*  This file is part of libnao-ui.

    libnao-ui is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libnao-ui is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libnao-ui.  If not, see <https://www.gnu.org/licenses/>.   */

#include "nao/platform/win32.h"

namespace nao::ui::platform::win32 {
    const string& default_window_class() {
        static string classname { "NaoDefaultWindowClass" };
        return classname;
    }

}