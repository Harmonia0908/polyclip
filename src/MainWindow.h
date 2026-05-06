#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QString>

class CanvasWidget;
class QLabel;
class QTextEdit;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = NULL);

private slots:
    void updateLog(const QString &text);
    void updateVertexCounts(int subjectCount, int clipCount);

private:
    CanvasWidget *m_canvas;
    QTextEdit *m_logEdit;
    QLabel *m_subjectCountLabel;
    QLabel *m_clipCountLabel;
};

#endif // MAIN_WINDOW_H
