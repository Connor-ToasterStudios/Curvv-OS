
#include "themes.h"

Theme get_active_theme() {
    return (Theme){
        .title_bg = 0x003366,
        .title_fg = 0xFFFFFF,
        .body_bg  = 0x222222,
        .border   = 0x888888
    };
}
