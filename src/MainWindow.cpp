#include "MainWindow.h"

#include "CanvasWidget.h"

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
    QComboBox *editTargetCombo = new QComboBox(this);
    QPushButton *recutButton = new QPushButton(QString::fromUtf8("重新裁切"), this);
    QPushButton *resetButton = new QPushButton(QString::fromUtf8("重置示例"), this);
    QPushButton *toggleIntersectionsButton = new QPushButton(QString::fromUtf8("显示/隐藏交点"), this);
    QPushButton *toggleEntryExitButton = new QPushButton(QString::fromUtf8("显示/隐藏 entry/exit 标记"), this);
    QLabel *tipsLabel = new QLabel(QString::fromUtf8("双击新增顶点，右键删除顶点，拖拽移动顶点"), this);

    m_logEdit->setReadOnly(true);
    m_logEdit->setMinimumWidth(360);
    tipsLabel->setWordWrap(true);
    editTargetCombo->addItem(QString::fromUtf8("Subject Polygon"));
    editTargetCombo->addItem(QString::fromUtf8("Clip Polygon"));

    sideLayout->addWidget(editTargetLabel);
    sideLayout->addWidget(editTargetCombo);
    sideLayout->addWidget(m_subjectCountLabel);
    sideLayout->addWidget(m_clipCountLabel);
    sideLayout->addWidget(tipsLabel);
    sideLayout->addSpacing(10);
    sideLayout->addWidget(recutButton);
    sideLayout->addWidget(resetButton);
    sideLayout->addWidget(toggleIntersectionsButton);
    sideLayout->addWidget(toggleEntryExitButton);
    sideLayout->addSpacing(10);
    sideLayout->addWidget(title);
    sideLayout->addWidget(m_logEdit, 1);

    mainLayout->addWidget(m_canvas, 1);
    mainLayout->addLayout(sideLayout);

    setCentralWidget(central);
    setWindowTitle(QString::fromUtf8("Weiler-Atherton 多边形裁剪演示"));
    resize(1180, 650);

    connect(recutButton, SIGNAL(clicked()), m_canvas, SLOT(recut()));
    connect(resetButton, SIGNAL(clicked()), m_canvas, SLOT(resetExample()));
    connect(toggleIntersectionsButton, SIGNAL(clicked()), m_canvas, SLOT(toggleIntersections()));
    connect(toggleEntryExitButton, SIGNAL(clicked()), m_canvas, SLOT(toggleEntryExitLabels()));
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
