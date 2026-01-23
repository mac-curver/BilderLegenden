#include <QPainter>
#include <QKeyEvent>
#include "testwidget.h"

TestWidget::TestWidget(QWidget *parent)
    : Super{parent}
{
    setFocusPolicy(Qt::StrongFocus);
}

void TestWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
    QPainter painter(this);
    QPen pen(Qt::red, 40.0);
    QRect myRect(100*displayFactor,100*displayFactor,100*displayFactor,200*displayFactor);

    painter.setPen(pen);
    painter.drawRect(myRect);
    pen.setColor(Qt::blue);
    painter.setPen(pen);
    painter.drawRect(rect());
}

QSize TestWidget::sizeHint() const {
    return QSize(400*displayFactor, 400*displayFactor);
}

void TestWidget::keyPressEvent(QKeyEvent *event) {
    qDebug() << "TestWidget::keyPressEvent";
    switch (event->key()) {
    case Qt::Key_Plus:
        displayFactor *= 1.2;
        break;
    case Qt::Key_Minus:
        displayFactor /= 1.2;
        break;
    case Qt::Key_Escape:
        displayFactor = 1.0;
        break;
    default:
        QWidget::keyPressEvent(event);
        break;

    }
    resize(QSize(400*displayFactor, 400*displayFactor));
    update();

}
