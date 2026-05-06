#include "MainWindow.h"

#include "CanvasWidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_canvas(new CanvasWidget(this)),
      m_logEdit(new QTextEdit(this)),
      m_subjectCountLabel(new QLabel(this)),
      m_clipCountLabel(new QLabel(this))
{
    QWidget *central = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);

    QVBoxLayout *sideLayout = new QVBoxLayout();
    QLabel *title = new QLabel(QString::fromUtf8("算法步骤日志"), this);
    QLabel *editTargetLabel = new QLabel(QString::fromUtf8("当前编辑对象"), this);
    QLabel *presetTitle = new QLabel(QString::fromUtf8("预设案例"), this);
    QComboBox *editTargetCombo = new QComboBox(this);
    QPushButton *recutButton = new QPushButton(QString::fromUtf8("重新裁切"), this);
    QPushButton *defaultButton = new QPushButton(QString::fromUtf8("默认示例"), this);
    QPushButton *concaveButton = new QPushButton(QString::fromUtf8("凹多边形裁切"), this);
    QPushButton *convexButton = new QPushButton(QString::fromUtf8("凸多边形裁切"), this);
    QPushButton *insideButton = new QPushButton(QString::fromUtf8("Subject 完全在 Clip 内"), this);
    QPushButton *disjointButton = new QPushButton(QString::fromUtf8("完全不相交"), this);
    QPushButton *vertexOnEdgeButton = new QPushButton(QString::fromUtf8("顶点落在边上"), this);
    QPushButton *tangentButton = new QPushButton(QString::fromUtf8("边界相切"), this);
    QCheckBox *showIntersectionsCheck = new QCheckBox(QString::fromUtf8("显示交点"), this);
    QCheckBox *showEntryExitCheck = new QCheckBox(QString::fromUtf8("显示 entry/exit 标记"), this);
    QLabel *tipsLabel = new QLabel(QString::fromUtf8("双击新增顶点，右键删除顶点，拖拽移动顶点"), this);

    m_logEdit->setReadOnly(true);
    m_logEdit->setMinimumWidth(360);
    tipsLabel->setWordWrap(true);
    editTargetCombo->addItem(QString::fromUtf8("Subject Polygon"));
    editTargetCombo->addItem(QString::fromUtf8("Clip Polygon"));
    showIntersectionsCheck->setChecked(true);
    showEntryExitCheck->setChecked(true);

    sideLayout->addWidget(editTargetLabel);
    sideLayout->addWidget(editTargetCombo);
    sideLayout->addWidget(m_subjectCountLabel);
    sideLayout->addWidget(m_clipCountLabel);
    sideLayout->addWidget(tipsLabel);
    sideLayout->addSpacing(10);
    sideLayout->addWidget(recutButton);
    sideLayout->addWidget(defaultButton);
    sideLayout->addWidget(showIntersectionsCheck);
    sideLayout->addWidget(showEntryExitCheck);
    sideLayout->addSpacing(10);
    sideLayout->addWidget(presetTitle);
    sideLayout->addWidget(concaveButton);
    sideLayout->addWidget(convexButton);
    sideLayout->addWidget(insideButton);
    sideLayout->addWidget(disjointButton);
    sideLayout->addWidget(vertexOnEdgeButton);
    sideLayout->addWidget(tangentButton);
    sideLayout->addSpacing(10);
    sideLayout->addWidget(title);
    sideLayout->addWidget(m_logEdit, 1);

    mainLayout->addWidget(m_canvas, 1);
    mainLayout->addLayout(sideLayout);

    setCentralWidget(central);
    setWindowTitle(QString::fromUtf8("Weiler-Atherton 多边形裁剪演示"));
    resize(1180, 650);

    connect(recutButton, SIGNAL(clicked()), m_canvas, SLOT(recut()));
    connect(defaultButton, SIGNAL(clicked()), m_canvas, SLOT(loadDefaultExample()));
    connect(concaveButton, SIGNAL(clicked()), m_canvas, SLOT(loadConcaveExample()));
    connect(convexButton, SIGNAL(clicked()), m_canvas, SLOT(loadConvexExample()));
    connect(insideButton, SIGNAL(clicked()), m_canvas, SLOT(loadSubjectInsideExample()));
    connect(disjointButton, SIGNAL(clicked()), m_canvas, SLOT(loadDisjointExample()));
    connect(vertexOnEdgeButton, SIGNAL(clicked()), m_canvas, SLOT(loadVertexOnEdgeExample()));
    connect(tangentButton, SIGNAL(clicked()), m_canvas, SLOT(loadTangentExample()));
    connect(showIntersectionsCheck, SIGNAL(toggled(bool)), m_canvas, SLOT(setShowIntersections(bool)));
    connect(showEntryExitCheck, SIGNAL(toggled(bool)), m_canvas, SLOT(setShowEntryExitLabels(bool)));
    connect(editTargetCombo, SIGNAL(currentIndexChanged(int)), m_canvas, SLOT(setEditTarget(int)));
    connect(m_canvas, SIGNAL(logChanged(QString)), this, SLOT(updateLog(QString)));
    connect(m_canvas, SIGNAL(polygonCountsChanged(int,int)), this, SLOT(updateVertexCounts(int,int)));

    updateLog(QString());
    updateVertexCounts(m_canvas->subjectVertexCount(), m_canvas->clipVertexCount());
    m_canvas->recut();
}

void MainWindow::updateLog(const QString &text)
{
    m_logEdit->setPlainText(text);
}

void MainWindow::updateVertexCounts(int subjectCount, int clipCount)
{
    m_subjectCountLabel->setText(QString::fromUtf8("Subject 顶点数：%1").arg(subjectCount));
    m_clipCountLabel->setText(QString::fromUtf8("Clip 顶点数：%1").arg(clipCount));
}
