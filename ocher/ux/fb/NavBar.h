#ifndef OCHER_UX_FB_NAVBAR_H
#define OCHER_UX_FB_NAVBAR_H

#include "ocher/ux/fb/Widgets.h"


class NavBar : public Window
{
public:
    NavBar();

protected:
    void drawContent(Rect*);
};

#endif