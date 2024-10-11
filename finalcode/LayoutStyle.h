#ifndef LAYOUT_STYLE_H
#define LAYOUT_STYLE_H

#include <string>

enum class LayoutStyle {
    Meeting,
    Lecture,
    Wedding,
    Dance
};

inline std::string layoutStyleToString(LayoutStyle style) {
    switch (style) {
        case LayoutStyle::Meeting: return "Meeting";
        case LayoutStyle::Lecture: return "Lecture";
        case LayoutStyle::Wedding: return "Wedding";
        case LayoutStyle::Dance: return "Dance";
        default: return "Unknown";
    }
}

#endif // LAYOUT_STYLE_H