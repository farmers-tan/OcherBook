#include "clc/support/Logger.h"
#ifdef OCHER_EPUB
#include "ocher/fmt/epub/Epub.h"
#include "ocher/fmt/epub/LayoutEpub.h"
#endif
#if 1
#include "ocher/fmt/text/LayoutText.h"
#include "ocher/fmt/text/Text.h"
#endif
#include "ocher/settings/Settings.h"
#include "ocher/shelf/Meta.h"
#include "ocher/ux/Controller.h"
#include "ocher/ux/Factory.h"
#include "ocher/ux/ReadActivity.h"

#define LOG_NAME "ocher.ux.Read"


// TODO: if (m_pagesSinceRefresh >= settings.fullRefreshPages) {

int ReadActivity::evtKey(struct OcherKeyEvent* evt)
{
    UiBits& ui = m_controller->ui;
    if (evt->subtype == OEVT_KEY_DOWN) {
        if (evt->key == OEVTK_HOME) {
            clc::Log::info(LOG_NAME, "home");
            // TODO  visually turn page down
            return ACTIVITY_HOME;
        } else if (evt->key == OEVTK_LEFT || evt->key == OEVTK_UP || evt->key == OEVTK_PAGEUP) {
            clc::Log::info(LOG_NAME, "back from page %d", m_pageNum);
            if (m_pageNum > 0) {
                m_pageNum--;
                ui.m_systemBar.hide();
                ui.m_navBar.hide();
                dirty();
            }
            return -1;
        } else if (evt->key == OEVTK_RIGHT || evt->key == OEVTK_DOWN || evt->key == OEVTK_PAGEDOWN) {
            clc::Log::info(LOG_NAME, "forward from page %d", m_pageNum);
            if (! atEnd) {
                m_pageNum++;
                ui.m_systemBar.hide();
                ui.m_navBar.hide();
                dirty();
            }
            return -1;
        }
    }
    return -2;
}

int ReadActivity::evtMouse(struct OcherMouseEvent* evt)
{
    UiBits& ui = m_controller->ui;
    if (evt->subtype == OEVT_MOUSE1_UP) {
        Pos pos(evt->x, evt->y);
        if (ui.m_systemBar.m_rect.contains(&pos) || ui.m_navBar.m_rect.contains(&pos)) {
            if (ui.m_systemBar.m_flags & WIDGET_HIDDEN) {
                clc::Log::info(LOG_NAME, "show system bar");
                ui.m_systemBar.show();
                g_fb->update(&ui.m_systemBar.m_rect);
                ui.m_navBar.show();
                g_fb->update(&ui.m_navBar.m_rect);
            } else {
                clc::Log::info(LOG_NAME, "interact bar");
                // TODO interact
            }
        } else {
            if (! (ui.m_systemBar.m_flags & WIDGET_HIDDEN)) {
                clc::Log::info(LOG_NAME, "hide system bar");
                ui.m_systemBar.hide();
                ui.m_navBar.hide();
                dirty();
            } else {
                if (evt->x < g_fb->width()/2) {
                    if (m_pageNum > 0) {
                        clc::Log::info(LOG_NAME, "back from page %d", m_pageNum);
                        m_pageNum--;
                        dirty();
                    }
                } else {
                    if (!atEnd) {
                        clc::Log::info(LOG_NAME, "forward from page %d", m_pageNum);
                        m_pageNum++;
                        dirty();
                    }
                }
            }
        }
        return -1;
    }
    return -2;
}

ReadActivity::ReadActivity(Controller* c) :
    m_controller(c),
    m_pagesSinceRefresh(0)
{
}

Rect ReadActivity::draw(Pos*)
{
    Rect drawn;
    drawn.setInvalid();

    if (m_flags & WIDGET_DIRTY) {
        clc::Log::info(LOG_NAME, "draw");
        m_flags &= ~WIDGET_DIRTY;
        drawn = m_rect;
        m_pagesSinceRefresh++;
        atEnd = renderer->render(&meta->m_pagination, m_pageNum, true);
    }
    Rect d2 = drawChildren(m_rect.pos());
    drawn.unionRect(&d2);
    return drawn;
}

void ReadActivity::onAttach()
{
    clc::Log::info(LOG_NAME, "attach");
    meta = m_controller->ctx.selected;
    ASSERT(meta);
    clc::Log::debug(LOG_NAME, "selected %p", meta);

    g_fb->clear();
    g_fb->update(NULL);

    // TODO:  rework Layout constructors to have separate init due to scoping
    Layout* layout = 0;
    clc::Buffer memLayout;
    const char* file = meta->relPath.c_str();
    clc::Log::info(LOG_NAME, "Loading %s: %s", Meta::fmtToStr(meta->format), file);
    switch (meta->format) {
        case OCHER_FMT_TEXT: {
            Text text(file);
            layout = new LayoutText(&text);
            memLayout = layout->unlock();
            break;
        }
#ifdef OCHER_EPUB
        case OCHER_FMT_EPUB: {
            Epub epub(file);
            layout = new LayoutEpub(&epub);
            clc::Buffer html;
            for (int i = 0; ; i++) {
                if (epub.getSpineItemByIndex(i, html) != 0)
                    break;
                mxml_node_t* tree = epub.parseXml(html);
                if (tree) {
                    ((LayoutEpub*)layout)->append(tree);
                    mxmlDelete(tree);
                } else {
                    clc::Log::warn(LOG_NAME, "No tree found for spine item %d", i);
                }
            }
            memLayout = layout->unlock();
            break;
        }
#endif
        default: {
            clc::Log::warn(LOG_NAME, "Unhandled format %d", meta->format);
        }
    }

    renderer = uiFactory->getRenderer();
    renderer->set(memLayout);

    // Optionally, run through all pages without blitting to get an accurate
    // page count.  Alternative is to do some sort of "idealize" page layout that might be faster.
#if 1
    if (meta->m_pagination.numPages() == 0) {
        for (int pageNum = 0; ; pageNum++) {
            clc::Log::info(LOG_NAME, "Paginating page %d", pageNum);
            int r = renderer->render(&meta->m_pagination, pageNum, false);

            if (r != 0) {
                meta->pages = pageNum + 1;
                break;
            }
        }
    }
#endif

    m_controller->ui.m_systemBar.m_sep = true;
    m_controller->ui.m_systemBar.m_title = meta->title;
    m_controller->ui.m_systemBar.hide();
    addChild(m_controller->ui.m_systemBar);

    m_controller->ui.m_navBar.hide();
    addChild(m_controller->ui.m_navBar);

    meta->record.touch();
    m_pageNum = meta->record.activePage;
    clc::Log::info(LOG_NAME, "Starting on page %u", m_pageNum);
    dirty();
}

void ReadActivity::onDetach()
{
    clc::Log::info(LOG_NAME, "Quitting on page %u", m_pageNum);
    meta->record.activePage = m_pageNum;

    // TODO delete layout;
}
