#include "WeilerAtherton.h"

#include <iostream>
#include <vector>

static void printPoint(const geom::Point &p)
{
    std::cout << "(" << p.x << ", " << p.y << ")";
}

static void printPolygon(const std::vector<geom::Point> &poly)
{
    for (size_t i = 0; i < poly.size(); ++i) {
        std::cout << "  P" << i << " = ";
        printPoint(poly[i]);
        std::cout << "\n";
    }
}

int main()
{
    std::vector<geom::Point> subject;
    subject.push_back(geom::Point(120.0, 130.0));
    subject.push_back(geom::Point(330.0, 95.0));
    subject.push_back(geom::Point(420.0, 210.0));
    subject.push_back(geom::Point(290.0, 250.0));
    subject.push_back(geom::Point(390.0, 420.0));
    subject.push_back(geom::Point(155.0, 375.0));
    subject.push_back(geom::Point(95.0, 245.0));

    std::vector<geom::Point> clip;
    clip.push_back(geom::Point(235.0, 145.0));
    clip.push_back(geom::Point(540.0, 165.0));
    clip.push_back(geom::Point(575.0, 355.0));
    clip.push_back(geom::Point(370.0, 470.0));
    clip.push_back(geom::Point(190.0, 330.0));

    ClipResult result = WeilerAtherton::clip(subject, clip);

    std::cout << "Result polygon count: " << result.polygons.size() << "\n";
    for (size_t i = 0; i < result.polygons.size(); ++i) {
        std::cout << "Result polygon " << i << " vertex count: " << result.polygons[i].size() << "\n";
        printPolygon(result.polygons[i]);
    }

    std::cout << "Intersection count: " << result.intersections.size() << "\n";
    for (size_t i = 0; i < result.intersections.size(); ++i) {
        const IntersectionInfo &info = result.intersections[i];
        std::cout << "  I" << i << " = ";
        printPoint(info.point);
        std::cout << ", ";
        if (info.crossing) {
            std::cout << (info.entry ? "entry" : "exit");
        } else {
            std::cout << "touching/non-crossing";
        }
        std::cout << "\n";
    }

    return 0;
}
