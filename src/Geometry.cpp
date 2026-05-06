#include "Geometry.h"

#include <algorithm>
#include <cmath>

namespace geom {

Point::Point() : x(0.0), y(0.0) {}

Point::Point(double px, double py) : x(px), y(py) {}

SegmentIntersection::SegmentIntersection()
    : valid(false), overlap(false), point(), t(0.0), u(0.0) {}

Point operator+(const Point &a, const Point &b)
{
    return Point(a.x + b.x, a.y + b.y);
}

Point operator-(const Point &a, const Point &b)
{
    return Point(a.x - b.x, a.y - b.y);
}

Point operator*(const Point &a, double k)
{
    return Point(a.x * k, a.y * k);
}

double dot(const Point &a, const Point &b)
{
    return a.x * b.x + a.y * b.y;
}

double cross(const Point &a, const Point &b)
{
    return a.x * b.y - a.y * b.x;
}

double cross(const Point &a, const Point &b, const Point &c)
{
    return cross(b - a, c - a);
}

double lengthSquared(const Point &a)
{
    return dot(a, a);
}

double distanceSquared(const Point &a, const Point &b)
{
    return lengthSquared(a - b);
}

bool nearlyEqual(double a, double b, double eps)
{
    return std::fabs(a - b) <= eps;
}

bool samePoint(const Point &a, const Point &b, double eps)
{
    return distanceSquared(a, b) <= eps * eps;
}

double polygonArea(const std::vector<Point> &poly)
{
    if (poly.size() < 3) {
        return 0.0;
    }

    double area = 0.0;
    for (size_t i = 0; i < poly.size(); ++i) {
        const Point &a = poly[i];
        const Point &b = poly[(i + 1) % poly.size()];
        area += cross(a, b);
    }
    return area * 0.5;
}

void normalizeCCW(std::vector<Point> &poly)
{
    if (polygonArea(poly) < 0.0) {
        std::reverse(poly.begin(), poly.end());
    }
}

bool pointOnSegment(const Point &p, const Point &a, const Point &b)
{
    if (std::fabs(cross(a, b, p)) > EPS) {
        return false;
    }

    double minX = std::min(a.x, b.x) - EPS;
    double maxX = std::max(a.x, b.x) + EPS;
    double minY = std::min(a.y, b.y) - EPS;
    double maxY = std::max(a.y, b.y) + EPS;
    return p.x >= minX && p.x <= maxX && p.y >= minY && p.y <= maxY;
}

bool pointInPolygon(const std::vector<Point> &poly, const Point &p, bool includeBoundary)
{
    if (poly.size() < 3) {
        return false;
    }

    for (size_t i = 0; i < poly.size(); ++i) {
        if (pointOnSegment(p, poly[i], poly[(i + 1) % poly.size()])) {
            return includeBoundary;
        }
    }

    bool inside = false;
    for (size_t i = 0, j = poly.size() - 1; i < poly.size(); j = i++) {
        const Point &pi = poly[i];
        const Point &pj = poly[j];

        bool crossesRay = ((pi.y > p.y) != (pj.y > p.y));
        if (crossesRay) {
            double xAtY = (pj.x - pi.x) * (p.y - pi.y) / (pj.y - pi.y) + pi.x;
            if (p.x < xAtY) {
                inside = !inside;
            }
        }
    }
    return inside;
}

double pointSegmentDistance(const Point &p, const Point &a, const Point &b)
{
    Point ab = b - a;
    double len2 = lengthSquared(ab);
    if (len2 <= EPS) {
        return std::sqrt(distanceSquared(p, a));
    }

    double t = dot(p - a, ab) / len2;
    if (t < 0.0) {
        t = 0.0;
    } else if (t > 1.0) {
        t = 1.0;
    }

    Point projection = a + ab * t;
    return std::sqrt(distanceSquared(p, projection));
}

double segmentParameter(const Point &a, const Point &b, const Point &p)
{
    Point ab = b - a;
    if (std::fabs(ab.x) >= std::fabs(ab.y)) {
        if (std::fabs(ab.x) < EPS) {
            return 0.0;
        }
        return (p.x - a.x) / ab.x;
    }

    if (std::fabs(ab.y) < EPS) {
        return 0.0;
    }
    return (p.y - a.y) / ab.y;
}

static double clamp01(double v)
{
    if (v < 0.0 && v > -EPS) {
        return 0.0;
    }
    if (v > 1.0 && v < 1.0 + EPS) {
        return 1.0;
    }
    return v;
}

static void addUniqueIntersection(
    std::vector<SegmentIntersection> &items,
    const Point &p,
    const Point &a,
    const Point &b,
    const Point &c,
    const Point &d,
    bool overlap)
{
    for (size_t i = 0; i < items.size(); ++i) {
        if (samePoint(items[i].point, p, EPS * 100.0)) {
            return;
        }
    }

    SegmentIntersection inter;
    inter.valid = true;
    inter.overlap = overlap;
    inter.point = p;
    inter.t = clamp01(segmentParameter(a, b, p));
    inter.u = clamp01(segmentParameter(c, d, p));
    items.push_back(inter);
}

std::vector<SegmentIntersection> segmentIntersections(
    const Point &a,
    const Point &b,
    const Point &c,
    const Point &d)
{
    std::vector<SegmentIntersection> result;

    Point r = b - a;
    Point s = d - c;
    double denom = cross(r, s);
    double cmaCrossR = cross(c - a, r);

    if (std::fabs(denom) > EPS) {
        double t = cross(c - a, s) / denom;
        double u = cross(c - a, r) / denom;

        if (t >= -EPS && t <= 1.0 + EPS && u >= -EPS && u <= 1.0 + EPS) {
            SegmentIntersection inter;
            inter.valid = true;
            inter.overlap = false;
            inter.t = clamp01(t);
            inter.u = clamp01(u);
            inter.point = a + r * inter.t;
            result.push_back(inter);
        }
        return result;
    }

    if (std::fabs(cmaCrossR) > EPS) {
        return result;
    }

    // 共线重叠时，把重叠段的端点作为边界交点。完整重叠边不是本演示的重点，
    // 但这样可以让“顶点落在另一条边上”的情况被识别并显示出来。
    if (pointOnSegment(a, c, d)) {
        addUniqueIntersection(result, a, a, b, c, d, true);
    }
    if (pointOnSegment(b, c, d)) {
        addUniqueIntersection(result, b, a, b, c, d, true);
    }
    if (pointOnSegment(c, a, b)) {
        addUniqueIntersection(result, c, a, b, c, d, true);
    }
    if (pointOnSegment(d, a, b)) {
        addUniqueIntersection(result, d, a, b, c, d, true);
    }

    std::sort(result.begin(), result.end(), [](const SegmentIntersection &lhs, const SegmentIntersection &rhs) {
        return lhs.t < rhs.t;
    });

    return result;
}

} // namespace geom
