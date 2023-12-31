/* See LICENSE file for copyright and license details. */
#include "dwm.h"
#include <X11/XF86keysym.h>

/* appearance */
static const unsigned int borderpx = 3; /* border pixel of windows */
static const unsigned int snap = 32;    /* snap pixel */
static const int showbar = 1;           /* 0 means no bar */
static const int topbar = 1;            /* 0 means bottom bar */
static const char *fonts[] = {"monospace:size=10"};
static const char dmenufont[] = "monospace:size=10";
static const char col_fg[] = "#F8F8F2";
static const char col_bg[] = "#282A36";
static const char col_primary[] = "#BD93F9";
static const char *colors[][3] = {
    /*               fg         bg         border   */
    [SchemeNorm] = {col_fg, col_bg, col_bg},
    [SchemeSel] = {col_primary, col_bg, col_primary},
};

/* tagging */
static const char *tags[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9"};

static const Rule rules[] = {
    /* xprop(1):
     *	WM_CLASS(STRING) = instance, class
     *	WM_NAME(STRING) = title
     */
    /* class      instance    title       tags mask     isfloating   monitor */
    {"Brave-browser", NULL, NULL, 1 << 1, 0, -1},
    {"Firefox", NULL, NULL, 1 << 2, 0, -1},
    {"thunderbird", NULL, NULL, 1 << 2, 0, -1},
    {"zoom", NULL, NULL, 1 << 8, 1, -1},
    {"Swayosd-server", NULL, NULL, 0, 1, -1},
    {"Gimp", NULL, NULL, 0, 1, -1},
    {NULL, "goldendict", NULL, 0, 1, -1},
};

/* layout(s) */
static const float mfact = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster = 1;    /* number of clients in master area */
static const int resizehints =
    1; /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen =
    1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
    /* symbol     arrange function */
    {"[]=", tile}, /* first entry is default */
    {"><>", NULL}, /* no layout function means floating behavior */
    {"[M]", monocle},
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY, TAG)                                                      \
  {MODKEY, KEY, view, {.ui = 1 << TAG}},                                       \
      {MODKEY | ControlMask, KEY, toggleview, {.ui = 1 << TAG}},               \
      {MODKEY | ShiftMask, KEY, tag, {.ui = 1 << TAG}},                        \
      {MODKEY | ControlMask | ShiftMask, KEY, toggletag, {.ui = 1 << TAG}},

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd)                                                             \
  {                                                                            \
    .v = (const char *[]) { "/bin/sh", "-c", cmd, NULL }                       \
  }

/* commands */
static char dmenumon[2] =
    "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = {
    "dmenu_run", "-m",   dmenumon, "-fn",       dmenufont, "-nb",  col_bg,
    "-nf",       col_fg, "-sb",    col_primary, "-sf",     col_bg, NULL};
static const char *termcmd[] = {"st", NULL};

static const char *upbrightcmd[] = {"swayosd-client", "--brightness", "raise",
                                    NULL};
static const char *downbrightcmd[] = {"swayosd-client", "--brightness", "lower",
                                      NULL};

static const char *upvolcmd[] = {
    "swayosd-client", "--output-volume", "raise", "--max-volume", "150", NULL};
static const char *downvolcmd[] = {
    "swayosd-client", "--output-volume", "lower", "--max-volume", "150", NULL};
static const char *mutecmd[] = {"swayosd-client",
                                "--output-volume",
                                "mute-toggle",
                                "--max-volume",
                                "150",
                                NULL};
static const char *mutemiccmd[] = {"swayosd-client", "--input-volume",
                                   "mute-toggle", NULL};

static const Key keys[] = {
    /* modifier                     key        function        argument */
    {MODKEY, XK_p, spawn, {.v = dmenucmd}},
    {MODKEY | ShiftMask, XK_Return, spawn, {.v = termcmd}},
    {MODKEY, XK_b, togglebar, {0}},
    {MODKEY, XK_j, focusstack, {.i = +1}},
    {MODKEY, XK_k, focusstack, {.i = -1}},
    {MODKEY, XK_i, incnmaster, {.i = +1}},
    {MODKEY, XK_d, incnmaster, {.i = -1}},
    {MODKEY, XK_h, setmfact, {.f = -0.05}},
    {MODKEY, XK_l, setmfact, {.f = +0.05}},
    {MODKEY, XK_Return, zoom, {0}},
    {MODKEY, XK_Tab, view, {0}},
    {MODKEY | ShiftMask, XK_c, killclient, {0}},
    {MODKEY, XK_t, setlayout, {.v = &layouts[0]}},
    {MODKEY, XK_f, setlayout, {.v = &layouts[1]}},
    {MODKEY, XK_m, setlayout, {.v = &layouts[2]}},
    {MODKEY, XK_space, setlayout, {0}},
    {MODKEY | ShiftMask, XK_space, togglefloating, {0}},
    {MODKEY, XK_0, view, {.ui = ~0}},
    {MODKEY | ShiftMask, XK_0, tag, {.ui = ~0}},
    {MODKEY, XK_comma, focusmon, {.i = -1}},
    {MODKEY, XK_period, focusmon, {.i = +1}},
    {MODKEY | ShiftMask, XK_comma, tagmon, {.i = -1}},
    {MODKEY | ShiftMask, XK_period, tagmon, {.i = +1}},
    TAGKEYS(XK_1, 0) TAGKEYS(XK_2, 1) TAGKEYS(XK_3, 2) TAGKEYS(XK_4, 3)
        TAGKEYS(XK_5, 4) TAGKEYS(XK_6, 5) TAGKEYS(XK_7, 6) TAGKEYS(XK_8, 7)
            TAGKEYS(XK_9, 8){MODKEY | ShiftMask, XK_q, quit, {0}},
    {ControlMask, XK_Up, spawn, {.v = upvolcmd}},
    {ControlMask, XK_Down, spawn, {.v = downvolcmd}},
    {0, XF86XK_AudioMute, spawn, {.v = mutecmd}},
    {0, XF86XK_AudioMicMute, spawn, {.v = mutemiccmd}},
    {0, XF86XK_AudioRaiseVolume, spawn, {.v = upvolcmd}},
    {0, XF86XK_AudioLowerVolume, spawn, {.v = downvolcmd}},
    {0, XF86XK_MonBrightnessUp, spawn, {.v = upbrightcmd}},
    {0, XF86XK_MonBrightnessDown, spawn, {.v = downbrightcmd}},
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle,
 * ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
    /* click                event mask      button          function argument */
    {ClkLtSymbol, 0, Button1, setlayout, {0}},
    {ClkLtSymbol, 0, Button3, setlayout, {.v = &layouts[2]}},
    {ClkWinTitle, 0, Button2, zoom, {0}},
    {ClkStatusText, 0, Button2, spawn, {.v = termcmd}},
    {ClkClientWin, MODKEY, Button1, movemouse, {0}},
    {ClkClientWin, MODKEY, Button2, togglefloating, {0}},
    {ClkClientWin, MODKEY, Button3, resizemouse, {0}},
    {ClkTagBar, 0, Button1, view, {0}},
    {ClkTagBar, 0, Button3, toggleview, {0}},
    {ClkTagBar, MODKEY, Button1, tag, {0}},
    {ClkTagBar, MODKEY, Button3, toggletag, {0}},
};
