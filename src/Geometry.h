#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>

namespace geom {

const double EPS = 1e-9;

struct Point {
    double x;
    double y;

    Point();
    Point(double px, double py);
};

struct SegmentIntersection {
    bool valid;
    bool overlap;
    Point point;
    double t;
    double u;

    SegmentIntersection();
};

Point operator+(const Point &a, const Point &b);
Point operator-(const Point &a, const Point &b);
Point operator*(const Point &a, double k);

double dot(const Point &a, const Point &b);
double cross(const Point &a, const Point &b);
double cross(const Point &a, const Point &b, const Point &c);
double lengthSquared(const Point &a);
double distanceSquared(const Point &a, const Point &b);
bool nearlyEqual(double a, double b, double eps = EPS);
bool samePoint(const Point &a, const Point &b, double eps = EPS);

double polygonArea(const std::vector<Point> &poly);
void normalizeCCW(std::vector<Point> &poly);
bool pointOnSegment(const Point &p, const Point &a, const Point &b);
bool pointInPolygon(const std::vector<Point> &poly, const Point &p, bool includeBoundary);

double pointSegmentDistance(const Point &p, const Point &a, const Point &b);
double segmentParameter(const Point &a, const Point &b, const Point &p);
std::vector<SegmentIntersection> segmentIntersections(
    const Point &a,
    const Point &b,
    const Point &c,
    const Point &d);

} // namespace geom

#endif // GEOMETRY_H
