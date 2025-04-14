
#ifndef THEMES_H
#define THEMES_H

typedef struct {
    uint32_t title_bg;
    uint32_t title_fg;
    uint32_t body_bg;
    uint32_t border;
} Theme;

Theme get_active_theme(void);

#endif
