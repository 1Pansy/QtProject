#ifndef CHARTS_H
#define CHARTS_H

#include <QWidget>
#include <QtCharts>
#include "usersql.h"

namespace Ui {
class charts;
}

class charts : public QWidget
{
    Q_OBJECT

public:
    explicit charts(QWidget *parent = nullptr);
    ~charts();
    void createBarChart(BaseInfo info);

private:
    Ui::charts *ui;
};

#endif // CHARTS_H
