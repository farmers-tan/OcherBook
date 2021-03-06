/*
 * Copyright (c) 2015, Chuck Coffing
 * OcherBook is released under the GPLv3.  See COPYING.
 */

#ifndef OCHER_UX_FB_UXCONTROLLER_H
#define OCHER_UX_FB_UXCONTROLLER_H

#include "ux/Controller.h"
#include "ux/fb/FontEngine.h"
#include "ux/fb/FrameBuffer.h"
#include "ux/fb/RendererFb.h"
#include "ux/fb/Widgets.h"

class ActivityFb;

class UxControllerFb : public UxController {
public:
    UxControllerFb();
    ~UxControllerFb() = default;

    const std::string& getName() const override
    {
        return m_name;
    }

    bool init() override;

    FrameBuffer* getFrameBuffer() override
    {
        return m_frameBuffer.get();
    }

    FontEngine* getFontEngine() override
    {
        return m_fontEngine.get();
    }

    Renderer* getRenderer() override
    {
        return m_renderer.get();
    }

    Activity::Type previousActivity() override { return m_activityType; }
    void setNextActivity(Activity::Type a) override;

protected:
    std::string m_name;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<FontEngine> m_fontEngine;
    std::unique_ptr<FrameBuffer> m_frameBuffer;
    FbScreen m_screen;
    Activity::Type m_activityType = Activity::Type::Quit;
    ActivityFb* m_activity = nullptr;
};

#endif
