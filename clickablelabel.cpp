#include "clickablelabel.h"

ClickableLabel::ClickableLabel(QWidget *parent) : QLabel(parent) {
    setMouseTracking(true);
}

void ClickableLabel::mousePressEvent(QMouseEvent *event) {
    emit clicked(event->pos());
}
