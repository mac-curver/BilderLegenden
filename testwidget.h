#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include <QWidget>

class TestWidget : public QWidget {
    Q_OBJECT
    typedef QWidget Super;
public:
    explicit TestWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

    QSize sizeHint() const override;
    virtual void keyPressEvent(QKeyEvent *event) override;


signals:

private:
    double displayFactor = 1.0;
};

#endif // TESTWIDGET_H
