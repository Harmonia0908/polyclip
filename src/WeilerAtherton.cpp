#include "WeilerAtherton.h"

#include <algorithm>
#include <cmath>
#include <sstream>

using geom::Point;

namespace {

const double DISPLAY_EPS = 1e-7;

struct Node {
    Point p;
    Node *next;
    Node *prev;
    Node *pair;
    bool intersection;
    bool entry;
    bool crossing;
    bool visited;
    bool subjectNode;
    double alpha;
    int edgeIndex;

    Node(const Point &point, bool belongsToSubject)
        : p(point),
          next(NULL),
          prev(NULL),
          pair(NULL),
          intersection(false),
          entry(false),
          crossing(false),
          visited(false),
          subjectNode(belongsToSubject),
          alpha(0.0),
          edgeIndex(-1)
    {
    }
};

struct PendingNode {
    double alpha;
    Node *node;
};

struct Event {
    Point p;
    int subjectEdgeIndex;
    int clipEdgeIndex;
    double subjectAlpha;
    double clipAlpha;
    bool overlap;
};

Node *createNode(std::vector<Node *> &owned, const Point &p, bool subjectNode)
{
    Node *node = new Node(p, subjectNode);
    owned.push_back(node);
    return node;
}

void deleteNodes(std::vector<Node *> &owned)
{
    for (size_t i = 0; i < owned.size(); ++i) {
        delete owned[i];
    }
    owned.clear();
}

std::string pointToString(const Point &p)
{
    std::ostringstream oss;
    oss.precision(3);
    oss << std::fixed << "(" << p.x << ", " << p.y << ")";
    return oss.str();
}

std::string alphaToString(double alpha)
{
    std::ostringstream oss;
    oss.precision(6);
    oss << std::fixed << alpha;
    return oss.str();
}

bool sameDisplayPoint(const Point &a, const Point &b)
{
    return geom::samePoint(a, b, DISPLAY_EPS);
}

void appendUniquePoint(std::vector<Point> &poly, const Point &p)
{
    if (!poly.empty() && sameDisplayPoint(poly.back(), p)) {
        return;
    }
    poly.push_back(p);
}

std::vector<Point> cleanPolygon(const std::vector<Point> &poly)
{
    std::vector<Point> cleaned;
    for (size_t i = 0; i < poly.size(); ++i) {
        appendUniquePoint(cleaned, poly[i]);
    }

    while (cleaned.size() > 1 && sameDisplayPoint(cleaned.front(), cleaned.back())) {
        cleaned.pop_back();
    }

    if (cleaned.size() < 3) {
        return std::vector<Point>();
    }

    std::vector<Point> noCollinear;
    for (size_t i = 0; i < cleaned.size(); ++i) {
        const Point &prev = cleaned[(i + cleaned.size() - 1) % cleaned.size()];
        const Point &cur = cleaned[i];
        const Point &next = cleaned[(i + 1) % cleaned.size()];

        if (std::fabs(geom::cross(prev, cur, next)) <= geom::EPS * 100.0 &&
            geom::pointOnSegment(cur, prev, next)) {
            continue;
        }
        noCollinear.push_back(cur);
    }

    if (noCollinear.size() < 3 || std::fabs(geom::polygonArea(noCollinear)) <= geom::EPS) {
        return std::vector<Point>();
    }

    geom::normalizeCCW(noCollinear);
    return noCollinear;
}

bool allPointsInside(const std::vector<Point> &poly, const std::vector<Point> &container)
{
    if (poly.empty() || container.empty()) {
        return false;
    }

    for (size_t i = 0; i < poly.size(); ++i) {
        if (!geom::pointInPolygon(container, poly[i], true)) {
            return false;
        }
    }
    return true;
}

void insertPendingNodes(std::vector<Node *> &baseNodes, std::vector<std::vector<PendingNode> > &pending)
{
    for (size_t edge = 0; edge < baseNodes.size(); ++edge) {
        std::vector<PendingNode> &items = pending[edge];
        std::sort(items.begin(), items.end(), [](const PendingNode &lhs, const PendingNode &rhs) {
            return lhs.alpha < rhs.alpha;
        });

        Node *tail = baseNodes[edge];
        for (size_t i = 0; i < items.size(); ++i) {
            Node *node = items[i].node;
            node->prev = tail;
            node->next = tail->next;
            tail->next->prev = node;
            tail->next = node;
            tail = node;
        }
    }
}

Node *findNextDifferent(Node *node, bool forward)
{
    Node *cur = forward ? node->next : node->prev;
    int guard = 0;
    while (cur != node && guard < 10000) {
        if (!sameDisplayPoint(cur->p, node->p)) {
            return cur;
        }
        cur = forward ? cur->next : cur->prev;
        ++guard;
    }
    return cur;
}

bool insideNear(Node *node, bool forward, const std::vector<Point> &clipPolygon)
{
    Node *other = findNextDifferent(node, forward);
    Point dir = other->p - node->p;
    double len2 = geom::lengthSquared(dir);
    if (len2 <= geom::EPS) {
        return geom::pointInPolygon(clipPolygon, node->p, true);
    }

    // 在交点前后取一个很近的采样点，用它判断沿 subject 方向是在进入还是离开。
    // 采样距离用边长比例，避免坐标尺度稍大时固定 EPS 太小导致仍落在边界上。
    Point sample = node->p + dir * 1e-6;
    return geom::pointInPolygon(clipPolygon, sample, false);
}

void classifyIntersections(Node *subjectStart, const std::vector<Point> &clipPolygon, ClipResult &result)
{
    Node *cur = subjectStart;
    int guard = 0;
    do {
        if (cur->intersection && cur->pair != NULL) {
            bool insideBefore = insideNear(cur, false, clipPolygon);
            bool insideAfter = insideNear(cur, true, clipPolygon);

            cur->crossing = (insideBefore != insideAfter);
            cur->entry = (!insideBefore && insideAfter);

            cur->pair->crossing = cur->crossing;
            cur->pair->entry = !cur->entry;

            IntersectionInfo info;
            info.point = cur->p;
            info.entry = cur->entry;
            info.crossing = cur->crossing;
            result.intersections.push_back(info);

            if (cur->crossing) {
                result.logs.push_back(
                    std::string("交点 ") + pointToString(cur->p) +
                    (cur->entry ? " 标记为 entry：沿 Subject 从 Clip 外进入 Clip 内。"
                                : " 标记为 exit：沿 Subject 从 Clip 内离开 Clip 外。"));
            } else {
                result.logs.push_back(
                    std::string("交点 ") + pointToString(cur->p) +
                    " 是切触或边界接触点，不作为穿越切换点。");
            }
        }

        cur = cur->next;
        ++guard;
    } while (cur != subjectStart && guard < 10000);
}

void traceOneResult(Node *start, std::vector<Point> &poly)
{
    Node *cur = start;
    bool first = true;
    int guard = 0;

    while (cur != NULL && guard < 10000) {
        appendUniquePoint(poly, cur->p);

        if (cur->intersection && cur->crossing && cur->pair != NULL) {
            cur->visited = true;
            cur->pair->visited = true;

            // 从 Subject 的 entry 起点出发时，第一次遇到起点不能立刻切到 Clip，
            // 需要先沿 Subject 边界进入裁剪区域，直到遇到 exit 再切换。
            if (!first && cur->pair == start) {
                break;
            }

            if (!first) {
                cur = cur->pair;
            }
            first = false;
        }

        cur = cur->next;
        if (cur == start) {
            break;
        }
        ++guard;
    }
}

void traceResults(Node *subjectStart, ClipResult &result)
{
    Node *cur = subjectStart;
    int guard = 0;
    do {
        if (cur->intersection && cur->crossing && cur->entry && !cur->visited) {
            std::vector<Point> poly;
            result.logs.push_back(std::string("从 entry 点 ") + pointToString(cur->p) + " 开始追踪结果环。");
            traceOneResult(cur, poly);

            std::vector<Point> cleaned = cleanPolygon(poly);
            if (!cleaned.empty()) {
                result.polygons.push_back(cleaned);
                result.logs.push_back("完成一个结果多边形，顶点数：" + std::to_string(cleaned.size()) + "。");
            } else {
                result.logs.push_back("追踪得到的结果环面积过小，已忽略。");
            }
        }

        cur = cur->next;
        ++guard;
    } while (cur != subjectStart && guard < 10000);
}

void snapIntersectionPoint(Event &event, const std::vector<Point> &subject, const std::vector<Point> &clipPolygon)
{
    int si = event.subjectEdgeIndex;
    int sj = (si + 1) % static_cast<int>(subject.size());
    int ci = event.clipEdgeIndex;
    int cj = (ci + 1) % static_cast<int>(clipPolygon.size());

    if (sameDisplayPoint(event.p, subject[si])) {
        event.p = subject[si];
        event.subjectAlpha = 0.0;
    } else if (sameDisplayPoint(event.p, subject[sj])) {
        event.p = subject[sj];
        event.subjectAlpha = 1.0;
    }

    if (sameDisplayPoint(event.p, clipPolygon[ci])) {
        event.p = clipPolygon[ci];
        event.clipAlpha = 0.0;
    } else if (sameDisplayPoint(event.p, clipPolygon[cj])) {
        event.p = clipPolygon[cj];
        event.clipAlpha = 1.0;
    }
}

bool sameEvent(const Event &a, const Event &b)
{
    return a.subjectEdgeIndex == b.subjectEdgeIndex &&
           a.clipEdgeIndex == b.clipEdgeIndex &&
           geom::nearlyEqual(a.subjectAlpha, b.subjectAlpha, geom::EPS) &&
           geom::nearlyEqual(a.clipAlpha, b.clipAlpha, geom::EPS) &&
           geom::samePoint(a.p, b.p, geom::EPS);
}

bool eventAlreadyExists(const std::vector<Event> &events, const Event &event)
{
    for (size_t i = 0; i < events.size(); ++i) {
        if (sameEvent(events[i], event)) {
            return true;
        }
    }
    return false;
}

Node *nodeForEvent(
    const Event &event,
    bool subjectNode,
    std::vector<Node *> &baseNodes,
    std::vector<std::vector<PendingNode> > &pending,
    std::vector<Node *> &owned)
{
    double alpha = subjectNode ? event.subjectAlpha : event.clipAlpha;
    int edge = subjectNode ? event.subjectEdgeIndex : event.clipEdgeIndex;

    // 交点即使落在原始顶点上，也作为“边上的事件节点”插入。
    // 这样同一个坐标处来自不同边对的事件不会复用同一个顶点 Node，
    // 避免 pair 指针被后续事件覆盖，提升端点相交和顶点落边场景的稳定性。
    Node *node = createNode(owned, event.p, subjectNode);
    node->alpha = alpha;
    node->edgeIndex = edge;

    PendingNode pendingNode;
    pendingNode.alpha = alpha;
    pendingNode.node = node;
    pending[edge].push_back(pendingNode);

    node->p = event.p;
    node->intersection = true;
    node->alpha = alpha;
    node->edgeIndex = edge;
    return node;
}

} // namespace

ClipResult WeilerAtherton::clip(
    const std::vector<Point> &subjectInput,
    const std::vector<Point> &clipInput)
{
    ClipResult result;
    result.logs.push_back("开始 Weiler-Atherton 多边形裁剪。");

    if (subjectInput.size() < 3 || clipInput.size() < 3) {
        result.logs.push_back("Subject 或 Clip 顶点数不足 3，无法裁剪。");
        return result;
    }

    std::vector<Point> subject = subjectInput;
    std::vector<Point> clipPolygon = clipInput;
    geom::normalizeCCW(subject);
    geom::normalizeCCW(clipPolygon);
    result.logs.push_back("已将 Subject 和 Clip 的方向归一化为逆时针。");

    std::vector<Node *> owned;
    std::vector<Node *> subjectNodes(subject.size(), NULL);
    std::vector<Node *> clipNodes(clipPolygon.size(), NULL);

    for (size_t i = 0; i < subject.size(); ++i) {
        subjectNodes[i] = createNode(owned, subject[i], true);
    }
    for (size_t i = 0; i < clipPolygon.size(); ++i) {
        clipNodes[i] = createNode(owned, clipPolygon[i], false);
    }

    for (size_t i = 0; i < subjectNodes.size(); ++i) {
        subjectNodes[i]->next = subjectNodes[(i + 1) % subjectNodes.size()];
        subjectNodes[i]->prev = subjectNodes[(i + subjectNodes.size() - 1) % subjectNodes.size()];
    }
    for (size_t i = 0; i < clipNodes.size(); ++i) {
        clipNodes[i]->next = clipNodes[(i + 1) % clipNodes.size()];
        clipNodes[i]->prev = clipNodes[(i + clipNodes.size() - 1) % clipNodes.size()];
    }

    std::vector<Event> events;
    for (size_t i = 0; i < subject.size(); ++i) {
        Point a = subject[i];
        Point b = subject[(i + 1) % subject.size()];

        for (size_t j = 0; j < clipPolygon.size(); ++j) {
            Point c = clipPolygon[j];
            Point d = clipPolygon[(j + 1) % clipPolygon.size()];
            std::vector<geom::SegmentIntersection> intersections = geom::segmentIntersections(a, b, c, d);

            for (size_t k = 0; k < intersections.size(); ++k) {
                if (!intersections[k].valid) {
                    continue;
                }

                Event event;
                event.p = intersections[k].point;
                event.subjectEdgeIndex = static_cast<int>(i);
                event.clipEdgeIndex = static_cast<int>(j);
                event.subjectAlpha = intersections[k].t;
                event.clipAlpha = intersections[k].u;
                event.overlap = intersections[k].overlap;
                snapIntersectionPoint(event, subject, clipPolygon);

                if (!eventAlreadyExists(events, event)) {
                    events.push_back(event);
                }
            }
        }
    }

    result.logs.push_back("线段相交检测完成，交点数量：" + std::to_string(events.size()) + "。");

    if (events.empty()) {
        if (allPointsInside(subject, clipPolygon)) {
            result.polygons.push_back(subject);
            result.logs.push_back("没有交点，Subject 完全位于 Clip 内，结果为 Subject。");
        } else if (allPointsInside(clipPolygon, subject)) {
            result.polygons.push_back(clipPolygon);
            result.logs.push_back("没有交点，Clip 完全位于 Subject 内，结果为 Clip。");
        } else {
            result.logs.push_back("没有交点，且两者互不包含，结果为空。");
        }

        deleteNodes(owned);
        return result;
    }

    std::vector<std::vector<PendingNode> > subjectPending(subject.size());
    std::vector<std::vector<PendingNode> > clipPending(clipPolygon.size());

    for (size_t i = 0; i < events.size(); ++i) {
        Node *sNode = nodeForEvent(events[i], true, subjectNodes, subjectPending, owned);
        Node *cNode = nodeForEvent(events[i], false, clipNodes, clipPending, owned);
        sNode->pair = cNode;
        cNode->pair = sNode;

        result.logs.push_back(
            std::string("交点：Subject edge ") + std::to_string(events[i].subjectEdgeIndex) +
            ", Clip edge " + std::to_string(events[i].clipEdgeIndex) +
            ", alphaS=" + alphaToString(events[i].subjectAlpha) +
            ", alphaC=" + alphaToString(events[i].clipAlpha) +
            ", point=" + pointToString(events[i].p) + "。");
    }

    insertPendingNodes(subjectNodes, subjectPending);
    insertPendingNodes(clipNodes, clipPending);
    result.logs.push_back("已按边参数顺序把交点插入两个多边形的循环链表。");

    classifyIntersections(subjectNodes[0], clipPolygon, result);
    traceResults(subjectNodes[0], result);

    if (result.polygons.empty()) {
        if (allPointsInside(subject, clipPolygon)) {
            result.polygons.push_back(subject);
            result.logs.push_back("没有追踪到穿越结果，但 Subject 位于 Clip 内，使用 Subject 作为结果。");
        } else if (allPointsInside(clipPolygon, subject)) {
            result.polygons.push_back(clipPolygon);
            result.logs.push_back("没有追踪到穿越结果，但 Clip 位于 Subject 内，使用 Clip 作为结果。");
        } else {
            result.logs.push_back("没有可用结果环，结果为空。");
        }
    }

    result.logs.push_back("裁剪完成，输出结果多边形数量：" + std::to_string(result.polygons.size()) + "。");
    deleteNodes(owned);
    return result;
}
