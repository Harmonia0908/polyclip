#include "CanvasWidget.h"

#include <QFont>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include <cmath>
#include <sstream>

CanvasWidget::CanvasWidget(QWidget *parent)
    : QWidget(parent),
      m_showIntersections(true),
      m_showEntryExitLabels(true),
      m_dragTarget(DragNone),
      m_dragIndex(-1),
      m_editTarget(EditSubject)
{
    setMinimumSize(760, 560);
    setMouseTracking(true);
    resetExample();
}

int CanvasWidget::subjectVertexCount() const
{
    return static_cast<int>(m_subject.size());
}

int CanvasWidget::clipVertexCount() const
{
    return static_cast<int>(m_clip.size());
}

void CanvasWidget::resetExample()
{
    m_subject.clear();
    m_clip.clear();

    // 初始 Subject 是一个凹多边形，便于观察裁剪后的分段追踪。
    m_subject.push_back(geom::Point(120.0, 130.0));
    m_subject.push_back(geom::Point(330.0, 95.0));
    m_subject.push_back(geom::Point(420.0, 210.0));
    m_subject.push_back(geom::Point(290.0, 250.0));
    m_subject.push_back(geom::Point(390.0, 420.0));
    m_subject.push_back(geom::Point(155.0, 375.0));
    m_subject.push_back(geom::Point(95.0, 245.0));

    // 初始 Clip 是一个凸多边形。
    m_clip.push_back(geom::Point(235.0, 145.0));
    m_clip.push_back(geom::Point(540.0, 165.0));
    m_clip.push_back(geom::Point(575.0, 355.0));
    m_clip.push_back(geom::Point(370.0, 470.0));
    m_clip.push_back(geom::Point(190.0, 330.0));

    recut();
    emitPolygonCounts();
}

void CanvasWidget::recut()
{
    m_result = WeilerAtherton::clip(m_subject, m_clip);
    emit logChanged(logsToQString());
    update();
}

void CanvasWidget::toggleIntersections()
{
    m_showIntersections = !m_showIntersections;
    update();
}

void CanvasWidget::toggleEntryExitLabels()
{
    m_showEntryExitLabels = !m_showEntryExitLabels;
    update();
}

void CanvasWidget::setEditTarget(int index)
{
    m_editTarget = (index == 1) ? EditClip : EditSubject;
    update();
}

void CanvasWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor(248, 249, 250));

    drawResults(painter);
    drawPolygon(painter, m_subject, QColor(33, 108, 220), QColor(33, 108, 220, 35), 5, "Subject", "S");
    drawPolygon(painter, m_clip, QColor(220, 92, 45), QColor(220, 92, 45, 28), 5, "Clip", "C");

    if (m_showIntersections) {
        drawIntersections(painter);
    }

    drawLegend(painter);
}

void CanvasWidget::mousePressEvent(QMouseEvent *event)
{
    QPointF pos = event->pos();
    int subjectIndex = nearestVertex(pos, m_subject, 12.0);
    int clipIndex = nearestVertex(pos, m_clip, 12.0);

    if (event->button() == Qt::RightButton) {
        if (subjectIndex >= 0 && tryRemoveVertex(m_subject, subjectIndex)) {
            recut();
            emitPolygonCounts();
        } else if (clipIndex >= 0 && tryRemoveVertex(m_clip, clipIndex)) {
            recut();
            emitPolygonCounts();
        }
        m_dragTarget = DragNone;
        m_dragIndex = -1;
        return;
    }

    if (event->button() != Qt::LeftButton) {
        return;
    }

    if (subjectIndex >= 0) {
        m_dragTarget = DragSubject;
        m_dragIndex = subjectIndex;
    } else if (clipIndex >= 0) {
        m_dragTarget = DragClip;
        m_dragIndex = clipIndex;
    } else {
        m_dragTarget = DragNone;
        m_dragIndex = -1;
    }
}

void CanvasWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    QPointF pos = event->pos();
    if (nearestVertex(pos, m_subject, 12.0) >= 0 || nearestVertex(pos, m_clip, 12.0) >= 0) {
        return;
    }

    geom::Point p = fromQPoint(pos);
    bool changed = false;
    if (m_editTarget == EditSubject) {
        changed = tryInsertVertex(m_subject, p);
    } else {
        changed = tryInsertVertex(m_clip, p);
    }

    if (changed) {
        recut();
        emitPolygonCounts();
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragTarget == DragNone || m_dragIndex < 0) {
        return;
    }

    geom::Point p = fromQPoint(event->pos());
    if (m_dragTarget == DragSubject && m_dragIndex < static_cast<int>(m_subject.size())) {
        m_subject[static_cast<size_t>(m_dragIndex)] = p;
    } else if (m_dragTarget == DragClip && m_dragIndex < static_cast<int>(m_clip.size())) {
        m_clip[static_cast<size_t>(m_dragIndex)] = p;
    }

    recut();
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_dragTarget = DragNone;
    m_dragIndex = -1;
}

void CanvasWidget::drawPolygon(QPainter &painter,
                               const std::vector<geom::Point> &poly,
                               const QColor &stroke,
                               const QColor &fill,
                               int vertexRadius,
                               const QString &name,
                               const QString &prefix)
{
    if (poly.empty()) {
        return;
    }

    QPainterPath path;
    path.moveTo(toQPoint(poly[0]));
    for (size_t i = 1; i < poly.size(); ++i) {
        path.lineTo(toQPoint(poly[i]));
    }
    path.closeSubpath();

    painter.setPen(QPen(stroke, 2.2));
    painter.setBrush(fill);
    painter.drawPath(path);

    painter.setBrush(stroke);
    painter.setPen(Qt::NoPen);
    for (size_t i = 0; i < poly.size(); ++i) {
        QPointF pt = toQPoint(poly[i]);
        painter.drawEllipse(pt, vertexRadius, vertexRadius);
    }

    painter.setPen(stroke);
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(toQPoint(poly[0]) + QPointF(8.0, -8.0), name);

    painter.setFont(QFont("Arial", 9, QFont::Bold));
    for (size_t i = 0; i < poly.size(); ++i) {
        QPointF pt = toQPoint(poly[i]);
        painter.drawText(pt + QPointF(8.0, 14.0), prefix + QString::number(static_cast<int>(i)));
    }
}

void CanvasWidget::drawResults(QPainter &painter)
{
    painter.setPen(QPen(QColor(25, 145, 95), 2.0));
    painter.setBrush(QColor(30, 180, 120, 95));

    for (size_t i = 0; i < m_result.polygons.size(); ++i) {
        const std::vector<geom::Point> &poly = m_result.polygons[i];
        if (poly.empty()) {
            continue;
        }

        QPainterPath path;
        path.moveTo(toQPoint(poly[0]));
        for (size_t j = 1; j < poly.size(); ++j) {
            path.lineTo(toQPoint(poly[j]));
        }
        path.closeSubpath();
        painter.drawPath(path);
    }
}

void CanvasWidget::drawIntersections(QPainter &painter)
{
    painter.setFont(QFont("Arial", 9, QFont::Bold));

    for (size_t i = 0; i < m_result.intersections.size(); ++i) {
        const IntersectionInfo &info = m_result.intersections[i];
        QPointF pt = toQPoint(info.point);

        QColor color = QColor(30, 30, 30);
        QString label = "T";
        if (info.crossing) {
            if (info.entry) {
                color = QColor(25, 150, 75);
                label = "entry";
            } else {
                color = QColor(205, 55, 55);
                label = "exit";
            }
        }

        painter.setPen(QPen(Qt::white, 2.0));
        painter.setBrush(color);
        painter.drawEllipse(pt, 6.0, 6.0);

        if (m_showEntryExitLabels) {
            painter.setPen(color);
            painter.drawText(pt + QPointF(8.0, -8.0), label);
        }
    }
}

void CanvasWidget::drawLegend(QPainter &painter)
{
    painter.setFont(QFont("Arial", 10));
    painter.setPen(QColor(45, 45, 45));

    const int x = 14;
    const int y = 20;
    painter.drawText(x, y, "双击空白处新增当前对象顶点，右键删除顶点，拖拽移动顶点");

    int rowY = y + 22;
    painter.setPen(QPen(QColor(33, 108, 220), 3));
    painter.drawLine(x, rowY, x + 24, rowY);
    painter.setPen(QColor(45, 45, 45));
    painter.drawText(x + 32, rowY + 4, "Subject");

    painter.setPen(QPen(QColor(220, 92, 45), 3));
    painter.drawLine(x + 118, rowY, x + 142, rowY);
    painter.setPen(QColor(45, 45, 45));
    painter.drawText(x + 150, rowY + 4, "Clip");

    painter.setPen(QPen(QColor(25, 145, 95), 3));
    painter.drawLine(x + 212, rowY, x + 236, rowY);
    painter.setPen(QColor(45, 45, 45));
    painter.drawText(x + 244, rowY + 4, "Result");

    painter.setPen(QColor(45, 45, 45));
    QString editing = (m_editTarget == EditSubject)
        ? QString::fromUtf8("当前编辑：Subject")
        : QString::fromUtf8("当前编辑：Clip");
    painter.drawText(x + 318, rowY + 4, editing);
}

int CanvasWidget::nearestVertex(const QPointF &pos, const std::vector<geom::Point> &poly, double maxDistance) const
{
    int best = -1;
    double bestDist2 = maxDistance * maxDistance;

    for (size_t i = 0; i < poly.size(); ++i) {
        QPointF pt = toQPoint(poly[i]);
        double dx = pt.x() - pos.x();
        double dy = pt.y() - pos.y();
        double d2 = dx * dx + dy * dy;
        if (d2 <= bestDist2) {
            bestDist2 = d2;
            best = static_cast<int>(i);
        }
    }

    return best;
}

int CanvasWidget::findNearestEdge(const geom::Point &p, const std::vector<geom::Point> &poly) const
{
    if (poly.size() < 2) {
        return -1;
    }

    int bestEdge = -1;
    double bestDistance = 0.0;
    for (size_t i = 0; i < poly.size(); ++i) {
        const geom::Point &a = poly[i];
        const geom::Point &b = poly[(i + 1) % poly.size()];
        double distance = geom::pointSegmentDistance(p, a, b);
        if (bestEdge < 0 || distance < bestDistance) {
            bestEdge = static_cast<int>(i);
            bestDistance = distance;
        }
    }

    return bestEdge;
}

bool CanvasWidget::tryInsertVertex(std::vector<geom::Point> &poly, const geom::Point &p)
{
    int edge = findNearestEdge(p, poly);
    if (edge < 0) {
        return false;
    }

    // Weiler-Atherton 使用有序多边形边界，新增点必须插入最近边之后，
    // 这样新顶点会落在原边界序列中，而不是破坏拓扑顺序地追加到末尾。
    std::vector<geom::Point>::iterator it = poly.begin() + edge + 1;
    poly.insert(it, p);
    return true;
}

bool CanvasWidget::tryRemoveVertex(std::vector<geom::Point> &poly, int vertexIndex)
{
    if (poly.size() <= 3 || vertexIndex < 0 || vertexIndex >= static_cast<int>(poly.size())) {
        return false;
    }

    poly.erase(poly.begin() + vertexIndex);
    return true;
}

void CanvasWidget::emitPolygonCounts()
{
    emit polygonCountsChanged(subjectVertexCount(), clipVertexCount());
}

QPointF CanvasWidget::toQPoint(const geom::Point &p) const
{
    return QPointF(p.x, p.y);
}

geom::Point CanvasWidget::fromQPoint(const QPointF &p) const
{
    return geom::Point(p.x(), p.y());
}

QString CanvasWidget::logsToQString() const
{
    QString text;
    for (size_t i = 0; i < m_result.logs.size(); ++i) {
        text += QString::fromStdString(m_result.logs[i]);
        text += "\n";
    }
    return text;
}
