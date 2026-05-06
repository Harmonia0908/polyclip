#include "CanvasWidget.h"

#include <QFont>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include <cmath>
#include <sstream>

namespace {
const double INSERT_EDGE_MAX_DISTANCE = 35.0;
}

CanvasWidget::CanvasWidget(QWidget *parent)
    : QWidget(parent),
      m_showIntersections(true),
      m_showEntryExitLabels(true),
      m_subjectSimple(true),
      m_clipSimple(true),
      m_dragTarget(DragNone),
      m_dragIndex(-1),
      m_editTarget(EditSubject)
{
    setMinimumSize(760, 560);
    setMouseTracking(true);
    loadDefaultExample();
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
    loadDefaultExample();
}

void CanvasWidget::loadDefaultExample()
{
    m_statusLogs.clear();
    std::vector<geom::Point> subject;
    std::vector<geom::Point> clip;

    subject.push_back(geom::Point(120.0, 130.0));
    subject.push_back(geom::Point(330.0, 95.0));
    subject.push_back(geom::Point(420.0, 210.0));
    subject.push_back(geom::Point(290.0, 250.0));
    subject.push_back(geom::Point(390.0, 420.0));
    subject.push_back(geom::Point(155.0, 375.0));
    subject.push_back(geom::Point(95.0, 245.0));

    clip.push_back(geom::Point(235.0, 145.0));
    clip.push_back(geom::Point(540.0, 165.0));
    clip.push_back(geom::Point(575.0, 355.0));
    clip.push_back(geom::Point(370.0, 470.0));
    clip.push_back(geom::Point(190.0, 330.0));

    setExample(QString::fromUtf8("默认示例"),
               QString::fromUtf8("凹 Subject 与凸 Clip 相交，适合观察 entry/exit 标记和结果环追踪。"),
               subject,
               clip);
}

void CanvasWidget::loadConcaveExample()
{
    std::vector<geom::Point> subject;
    std::vector<geom::Point> clip;

    subject.push_back(geom::Point(110.0, 120.0));
    subject.push_back(geom::Point(365.0, 95.0));
    subject.push_back(geom::Point(450.0, 190.0));
    subject.push_back(geom::Point(315.0, 235.0));
    subject.push_back(geom::Point(455.0, 420.0));
    subject.push_back(geom::Point(185.0, 405.0));
    subject.push_back(geom::Point(85.0, 260.0));

    clip.push_back(geom::Point(230.0, 150.0));
    clip.push_back(geom::Point(535.0, 145.0));
    clip.push_back(geom::Point(585.0, 345.0));
    clip.push_back(geom::Point(375.0, 470.0));
    clip.push_back(geom::Point(185.0, 335.0));

    setExample(QString::fromUtf8("凹多边形裁切"),
               QString::fromUtf8("Subject 为明显凹多边形，Clip 为凸多边形，用于展示凹边界穿入穿出。"),
               subject,
               clip);
}

void CanvasWidget::loadConvexExample()
{
    std::vector<geom::Point> subject;
    std::vector<geom::Point> clip;

    subject.push_back(geom::Point(120.0, 150.0));
    subject.push_back(geom::Point(390.0, 110.0));
    subject.push_back(geom::Point(520.0, 290.0));
    subject.push_back(geom::Point(350.0, 455.0));
    subject.push_back(geom::Point(135.0, 375.0));

    clip.push_back(geom::Point(265.0, 95.0));
    clip.push_back(geom::Point(585.0, 215.0));
    clip.push_back(geom::Point(485.0, 500.0));
    clip.push_back(geom::Point(205.0, 430.0));
    clip.push_back(geom::Point(165.0, 225.0));

    setExample(QString::fromUtf8("凸多边形裁切"),
               QString::fromUtf8("Subject 和 Clip 都是凸多边形，结果通常为单个凸多边形。"),
               subject,
               clip);
}

void CanvasWidget::loadSubjectInsideExample()
{
    std::vector<geom::Point> subject;
    std::vector<geom::Point> clip;

    subject.push_back(geom::Point(270.0, 210.0));
    subject.push_back(geom::Point(420.0, 220.0));
    subject.push_back(geom::Point(445.0, 330.0));
    subject.push_back(geom::Point(330.0, 390.0));
    subject.push_back(geom::Point(245.0, 310.0));

    clip.push_back(geom::Point(155.0, 105.0));
    clip.push_back(geom::Point(575.0, 125.0));
    clip.push_back(geom::Point(625.0, 425.0));
    clip.push_back(geom::Point(345.0, 515.0));
    clip.push_back(geom::Point(105.0, 360.0));

    setExample(QString::fromUtf8("Subject 完全在 Clip 内"),
               QString::fromUtf8("两者没有交点，Subject 全部位于 Clip 内，结果应直接为 Subject。"),
               subject,
               clip);
}

void CanvasWidget::loadDisjointExample()
{
    std::vector<geom::Point> subject;
    std::vector<geom::Point> clip;

    subject.push_back(geom::Point(90.0, 130.0));
    subject.push_back(geom::Point(285.0, 125.0));
    subject.push_back(geom::Point(320.0, 275.0));
    subject.push_back(geom::Point(180.0, 355.0));
    subject.push_back(geom::Point(70.0, 270.0));

    clip.push_back(geom::Point(430.0, 235.0));
    clip.push_back(geom::Point(620.0, 250.0));
    clip.push_back(geom::Point(650.0, 410.0));
    clip.push_back(geom::Point(500.0, 500.0));
    clip.push_back(geom::Point(390.0, 390.0));

    setExample(QString::fromUtf8("完全不相交"),
               QString::fromUtf8("Subject 与 Clip 空间分离，没有交点也不存在包含关系，结果为空。"),
               subject,
               clip);
}

void CanvasWidget::loadVertexOnEdgeExample()
{
    std::vector<geom::Point> subject;
    std::vector<geom::Point> clip;

    subject.push_back(geom::Point(145.0, 150.0));
    subject.push_back(geom::Point(365.0, 150.0));
    subject.push_back(geom::Point(455.0, 270.0));
    subject.push_back(geom::Point(305.0, 425.0));
    subject.push_back(geom::Point(130.0, 330.0));

    clip.push_back(geom::Point(250.0, 90.0));
    clip.push_back(geom::Point(560.0, 150.0));
    clip.push_back(geom::Point(540.0, 395.0));
    clip.push_back(geom::Point(295.0, 500.0));
    clip.push_back(geom::Point(145.0, 150.0));

    setExample(QString::fromUtf8("顶点落在边上"),
               QString::fromUtf8("Clip 的一个顶点正好落在 Subject 的一条边上，用于观察边界交点处理。"),
               subject,
               clip);
}

void CanvasWidget::loadTangentExample()
{
    std::vector<geom::Point> subject;
    std::vector<geom::Point> clip;

    subject.push_back(geom::Point(150.0, 150.0));
    subject.push_back(geom::Point(350.0, 150.0));
    subject.push_back(geom::Point(350.0, 350.0));
    subject.push_back(geom::Point(150.0, 350.0));

    clip.push_back(geom::Point(350.0, 210.0));
    clip.push_back(geom::Point(560.0, 210.0));
    clip.push_back(geom::Point(560.0, 430.0));
    clip.push_back(geom::Point(350.0, 430.0));

    setExample(QString::fromUtf8("边界相切"),
               QString::fromUtf8("两个矩形只在边界线段上相切，结果面积应为空或退化，适合观察切触点标记。"),
               subject,
               clip);
}

void CanvasWidget::recut()
{
    m_subjectSimple = geom::isSimplePolygon(m_subject);
    m_clipSimple = geom::isSimplePolygon(m_clip);

    if (!m_subjectSimple || !m_clipSimple) {
        m_result = ClipResult();
        if (!m_subjectSimple) {
            m_result.logs.push_back("错误：Subject Polygon 存在自交，Weiler-Atherton 算法要求输入为简单多边形。");
        }
        if (!m_clipSimple) {
            m_result.logs.push_back("错误：Clip Polygon 存在自交，无法裁切。");
        }
        m_result.logs.push_back("请拖拽、删除或重新插入顶点，使多边形恢复为简单多边形后再裁切。");
        emit logChanged(logsToQString());
        update();
        return;
    }

    m_result = WeilerAtherton::clip(m_subject, m_clip);
    emit logChanged(logsToQString());
    update();
}

void CanvasWidget::toggleIntersections()
{
    setShowIntersections(!m_showIntersections);
}

void CanvasWidget::toggleEntryExitLabels()
{
    setShowEntryExitLabels(!m_showEntryExitLabels);
}

void CanvasWidget::setShowIntersections(bool enabled)
{
    m_showIntersections = enabled;
    addStatusLog(enabled ? QString::fromUtf8("已显示交点")
                         : QString::fromUtf8("已隐藏交点"));
    emitCurrentLog();
    update();
}

void CanvasWidget::setShowEntryExitLabels(bool enabled)
{
    m_showEntryExitLabels = enabled;
    addStatusLog(enabled ? QString::fromUtf8("已显示 entry/exit 标记")
                         : QString::fromUtf8("已隐藏 entry/exit 标记"));
    emitCurrentLog();
    update();
}

void CanvasWidget::setEditTarget(int index)
{
    m_editTarget = (index == 1) ? EditClip : EditSubject;
    addStatusLog(QString::fromUtf8("当前编辑对象：%1 Polygon").arg(editTargetLogName()));
    emitCurrentLog();
    update();
}

void CanvasWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor(248, 249, 250));

    drawResults(painter);
    drawPolygon(painter, m_subject, QColor(33, 108, 220), QColor(33, 108, 220, 35), 5, "Subject", "S", m_subjectSimple);
    drawPolygon(painter, m_clip, QColor(220, 92, 45), QColor(220, 92, 45, 28), 5, "Clip", "C", m_clipSimple);

    if (m_showIntersections) {
        drawIntersections(painter);
    }

    drawLegend(painter);
    drawInvalidStatus(painter);
}

void CanvasWidget::mousePressEvent(QMouseEvent *event)
{
    QPointF pos = event->pos();
    std::vector<geom::Point> &poly = editablePolygon();
    int vertexIndex = nearestVertex(pos, poly, 12.0);

    if (event->button() == Qt::RightButton) {
        if (vertexIndex >= 0 && tryRemoveVertex(poly, vertexIndex)) {
            addStatusLog(QString::fromUtf8("删除 %1 顶点 %2%3")
                             .arg(editTargetLogName())
                             .arg(editTargetVertexPrefix())
                             .arg(vertexIndex));
            recut();
            emitPolygonCounts();
        } else if (vertexIndex >= 0) {
            addStatusLog(QString::fromUtf8("删除失败：多边形至少需要保留 3 个顶点。"));
            emitCurrentLog();
            update();
        }
        m_dragTarget = DragNone;
        m_dragIndex = -1;
        return;
    }

    if (event->button() != Qt::LeftButton) {
        return;
    }

    if (vertexIndex >= 0) {
        m_dragTarget = dragTargetForEditTarget();
        m_dragIndex = vertexIndex;
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
    if (nearestVertex(pos, editablePolygon(), 12.0) >= 0) {
        return;
    }

    geom::Point p = fromQPoint(pos);
    int insertedAfterEdge = -1;
    bool changed = tryInsertVertex(editablePolygon(), p, &insertedAfterEdge);

    if (changed) {
        int nextEdge = (insertedAfterEdge + 1) % static_cast<int>(editablePolygon().size() - 1);
        addStatusLog(QString::fromUtf8("已向 %1 的边 %2%3-%2%4 之间插入新顶点。")
                         .arg(editTargetLogName())
                         .arg(editTargetVertexPrefix())
                         .arg(insertedAfterEdge)
                         .arg(nextEdge));
        recut();
        emitPolygonCounts();
    } else {
        addStatusLog(QString::fromUtf8("新增顶点失败：请双击靠近当前编辑多边形边的位置。"));
        emitCurrentLog();
        update();
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
                               const QString &prefix,
                               bool valid)
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

    QColor drawStroke = valid ? stroke : QColor(210, 35, 35);
    QColor drawFill = valid ? fill : QColor(210, 35, 35, 30);

    painter.setPen(QPen(drawStroke, valid ? 2.2 : 3.4));
    painter.setBrush(drawFill);
    painter.drawPath(path);

    painter.setBrush(drawStroke);
    painter.setPen(Qt::NoPen);
    for (size_t i = 0; i < poly.size(); ++i) {
        QPointF pt = toQPoint(poly[i]);
        painter.drawEllipse(pt, vertexRadius, vertexRadius);
    }

    painter.setPen(drawStroke);
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

void CanvasWidget::drawInvalidStatus(QPainter &painter)
{
    if (m_subjectSimple && m_clipSimple) {
        return;
    }

    painter.setPen(QColor(190, 30, 30));
    painter.setFont(QFont("Arial", 11, QFont::Bold));

    int y = 72;
    if (!m_subjectSimple) {
        painter.drawText(14, y, QString::fromUtf8("错误：Subject Polygon 存在自交，当前无法裁切"));
        y += 22;
    }
    if (!m_clipSimple) {
        painter.drawText(14, y, QString::fromUtf8("错误：Clip Polygon 存在自交，当前无法裁切"));
    }
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

bool CanvasWidget::tryInsertVertex(std::vector<geom::Point> &poly, const geom::Point &p, int *insertedAfterEdge)
{
    int edge = findNearestEdge(p, poly);
    if (edge < 0) {
        return false;
    }

    const geom::Point &a = poly[static_cast<size_t>(edge)];
    const geom::Point &b = poly[(static_cast<size_t>(edge) + 1) % poly.size()];
    if (geom::pointSegmentDistance(p, a, b) > INSERT_EDGE_MAX_DISTANCE) {
        return false;
    }

    // Weiler-Atherton 使用有序多边形边界，新增点必须插入最近边之后，
    // 这样新顶点会落在原边界序列中，而不是破坏拓扑顺序地追加到末尾。
    std::vector<geom::Point>::iterator it = poly.begin() + edge + 1;
    poly.insert(it, p);
    if (insertedAfterEdge != NULL) {
        *insertedAfterEdge = edge;
    }
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

std::vector<geom::Point> &CanvasWidget::editablePolygon()
{
    return (m_editTarget == EditSubject) ? m_subject : m_clip;
}

const std::vector<geom::Point> &CanvasWidget::editablePolygon() const
{
    return (m_editTarget == EditSubject) ? m_subject : m_clip;
}

CanvasWidget::DragTarget CanvasWidget::dragTargetForEditTarget() const
{
    return (m_editTarget == EditSubject) ? DragSubject : DragClip;
}

QString CanvasWidget::editTargetLogName() const
{
    return (m_editTarget == EditSubject) ? QString::fromUtf8("Subject") : QString::fromUtf8("Clip");
}

QString CanvasWidget::editTargetVertexPrefix() const
{
    return (m_editTarget == EditSubject) ? QString::fromUtf8("S") : QString::fromUtf8("C");
}

void CanvasWidget::setExample(const QString &caseName,
                              const QString &description,
                              const std::vector<geom::Point> &subject,
                              const std::vector<geom::Point> &clip)
{
    m_subject = subject;
    m_clip = clip;
    geom::normalizeCCW(m_subject);
    geom::normalizeCCW(m_clip);
    m_dragTarget = DragNone;
    m_dragIndex = -1;

    addStatusLog(QString::fromUtf8("预设案例：%1").arg(caseName));
    addStatusLog(description);
    addStatusLog(QString::fromUtf8("当前编辑对象：%1 Polygon").arg(editTargetLogName()));
    recut();
    emitPolygonCounts();
}

void CanvasWidget::addStatusLog(const QString &text)
{
    m_statusLogs.push_back(text);
    if (m_statusLogs.size() > 20) {
        m_statusLogs.erase(m_statusLogs.begin());
    }
}

void CanvasWidget::emitCurrentLog()
{
    emit logChanged(logsToQString());
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
    for (size_t i = 0; i < m_statusLogs.size(); ++i) {
        text += m_statusLogs[i];
        text += "\n";
    }
    if (!m_statusLogs.empty() && !m_result.logs.empty()) {
        text += "\n";
    }
    for (size_t i = 0; i < m_result.logs.size(); ++i) {
        text += QString::fromStdString(m_result.logs[i]);
        text += "\n";
    }
    return text;
}
