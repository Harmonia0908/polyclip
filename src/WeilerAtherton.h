#ifndef WEILER_ATHERTON_H
#define WEILER_ATHERTON_H

#include "Geometry.h"

#include <string>
#include <vector>

struct IntersectionInfo {
    geom::Point point;
    bool entry;
    bool crossing;
};

struct ClipResult {
    std::vector<std::vector<geom::Point> > polygons;
    std::vector<IntersectionInfo> intersections;
    std::vector<std::string> logs;
};

class WeilerAtherton {
public:
    static ClipResult clip(
        const std::vector<geom::Point> &subject,
        const std::vector<geom::Point> &clipPolygon);
};

#endif // WEILER_ATHERTON_H
