#ifndef CANVAS_WIDGET_H
#define CANVAS_WIDGET_H

#include "Geometry.h"
#include "WeilerAtherton.h"

#include <QColor>
#include <QString>
#include <QWidget>

#include <vector>

class QPainter;
class QMouseEvent;
class QPaintEvent;

class CanvasWidget : public QWidget {
    Q_OBJECT

public:
    explicit CanvasWidget(QWidget *parent = NULL);
    int subjectVertexCount() const;
    int clipVertexCount() const;

public slots:
    void recut();
    void resetExample();
    void loadDefaultExample();
    void loadConcaveExample();
    void loadConvexExample();
    void loadSubjectInsideExample();
    void loadDisjointExample();
    void loadVertexOnEdgeExample();
    void loadTangentExample();
    void toggleIntersections();
    void toggleEntryExitLabels();
    void setShowIntersections(bool enabled);
    void setShowEntryExitLabels(bool enabled);
    void setEditTarget(int index);

signals:
    void logChanged(const QString &text);
    void polygonCountsChanged(int subjectCount, int clipCount);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    enum EditTarget {
        EditSubject,
        EditClip
    };

    enum DragTarget {
        DragNone,
        DragSubject,
        DragClip
    };

    std::vector<geom::Point> m_subject;
    std::vector<geom::Point> m_clip;
    ClipResult m_result;
    std::vector<QString> m_statusLogs;
    bool m_showIntersections;
    bool m_showEntryExitLabels;
    bool m_subjectSimple;
    bool m_clipSimple;
    DragTarget m_dragTarget;
    int m_dragIndex;
    EditTarget m_editTarget;

    void drawPolygon(QPainter &painter,
                     const std::vector<geom::Point> &poly,
                     const QColor &stroke,
                     const QColor &fill,
                     int vertexRadius,
                     const QString &name,
                     const QString &prefix,
                     bool valid);
    void drawResults(QPainter &painter);
    void drawIntersections(QPainter &painter);
    void drawLegend(QPainter &painter);
    void drawInvalidStatus(QPainter &painter);
    int nearestVertex(const QPointF &pos, const std::vector<geom::Point> &poly, double maxDistance) const;
    int findNearestEdge(const geom::Point &p, const std::vector<geom::Point> &poly) const;
    bool tryInsertVertex(std::vector<geom::Point> &poly, const geom::Point &p, int *insertedAfterEdge);
    bool tryRemoveVertex(std::vector<geom::Point> &poly, int vertexIndex);
    std::vector<geom::Point> &editablePolygon();
    const std::vector<geom::Point> &editablePolygon() const;
    DragTarget dragTargetForEditTarget() const;
    QString editTargetLogName() const;
    QString editTargetVertexPrefix() const;
    void setExample(const QString &caseName,
                    const QString &description,
                    const std::vector<geom::Point> &subject,
                    const std::vector<geom::Point> &clip);
    void addStatusLog(const QString &text);
    void emitCurrentLog();
    void emitPolygonCounts();
    QPointF toQPoint(const geom::Point &p) const;
    geom::Point fromQPoint(const QPointF &p) const;
    QString logsToQString() const;
};

#endif // CANVAS_WIDGET_H
