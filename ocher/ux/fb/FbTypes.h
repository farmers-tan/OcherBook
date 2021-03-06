/*
 * Copyright (c) 2015, Chuck Coffing
 * OcherBook is released under the GPLv3.  See COPYING.
 */

#ifndef OCHER_UX_FB_TYPES_H
#define OCHER_UX_FB_TYPES_H

#include "ux/Types.h"

#include <cstddef>
#include <cstdint>
#include <new>


struct Rect {
    int16_t x, y;
    uint16_t w, h;

    Rect()
    {
        setInvalid();
    }

    Rect(int16_t _x, int16_t _y, uint16_t _w, uint16_t _h) :
        x(_x),
        y(_y),
        w(_w),
        h(_h)
    {
    }

    const Pos* pos() const
    {
        return reinterpret_cast<const Pos*>(this);
    }

    void offsetBy(const Pos* p)
    {
        x += p->x;
        y += p->y;
    }

    void unionRect(const Rect* r);

    void unionRects(const Rect* r1, const Rect* r2);

    bool intersects(const Rect& r) const
    {
        return r.x < x + w && r.x + r.w > x &&
               r.y < y + h && r.y + r.h > y;
    }

    bool contains(const Pos& p) const
    {
        return p.x >= x && p.y >= y && p.x < x + w && p.y < y + h;
    }

    bool valid() const
    {
        return x + w >= 0 && y + h >= 0;
    }

    void setInvalid()
    {
        x = y = -1;
        w = h = 0;
    }

    void inset(int i)
    {
        x += i;
        y += i;
        w -= (i + i);
        h -= (i + i);
    }
};

struct Glyph {
    static struct Glyph* create(size_t size)
    {
        uint8_t* p = new (std::nothrow) uint8_t[sizeof(struct Glyph) - 1 + size];
        return reinterpret_cast<struct Glyph*>(p);
    }
    static void destroy(struct Glyph* g)
    {
        uint8_t* p = reinterpret_cast<uint8_t*>(g);
        delete[] p;
    }

    int w;
    int h;
    int offsetX;
    int offsetY;
    int advanceX;
    int advanceY;
    int height;
    uint8_t bitmap[1];
};

struct GlyphDescr {
    GlyphDescr() : v(0) {}

    bool operator<(const GlyphDescr& r) const
    {
        return v < r.v;
    }

    union {
        struct {
            uint32_t c;
            uint8_t faceId;
            uint8_t points;
            int underline : 1;
            int bold : 1;
            int italic : 1;
        };
        uint64_t v;
    };
} __attribute__((packed));

#endif
