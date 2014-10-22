#ifndef NITRISHAPEWIDGET_H
#define NITRISHAPEWIDGET_H

#include <QListWidget>
#include <QListWidgetItem>

#include "NifHeaders.h"

class NiTriShapeWidget : public QListWidgetItem
{
public:
    NiTriShapeWidget(Niflib::NiTriShapeRef triShape, QListWidget *parent = 0);
    ~NiTriShapeWidget();

    Niflib::NiTriShapeRef GetNiTriShape() { return m_triShape; }

private:
    Niflib::NiTriShapeRef m_triShape;
};

#endif // NITRISHAPEWIDGET_H
