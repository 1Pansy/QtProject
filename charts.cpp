#include "charts.h"
#include "ui_charts.h"
#include <QDebug>

charts::charts(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::charts)
{
    ui->setupUi(this);

    // 基本窗口设置
    setWindowTitle("统计图表");
    setMinimumSize(800, 600);
}

charts::~charts()
{
    delete ui;
}

void charts::createBarChart(BaseInfo info)
{
    qDebug() << "创建图表，数据值:"
             << "姓名:" << info.name
             << "体温:" << info.temperature
             << "血压:" << info.bloodpressure
             << "心电:" << info.ECGsignal
             << "血氧:" << info.bloodoxygen
             << "呼吸:" << info.respiratoryrate;

    // 创建数据集
    QBarSet *set = new QBarSet("生理参数");
    *set << info.temperature << info.bloodpressure
         << info.ECGsignal << info.bloodoxygen << info.respiratoryrate;

    // 设置柱状图颜色
    set->setColor(QColor(0, 120, 215));

    // 创建柱状图系列
    QBarSeries *series = new QBarSeries();
    series->append(set);

    // 创建图表
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(QString(" %1 的生理数据图表").arg(info.name));
    chart->setTitleFont(QFont("Microsoft YaHei", 12, QFont::Bold));
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // 设置分类轴
    QStringList categories;
    categories << "体温(℃)" << "血压(mmHg)"
               << "心电信号(BPM)" << "血氧(%)" << "呼吸率(BPM)";

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsAngle(-30);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // 设置值轴
    QValueAxis *axisY = new QValueAxis();

    // 计算最大值
    int maxVal = info.temperature;
    if (info.bloodpressure > maxVal) maxVal = info.bloodpressure;
    if (info.ECGsignal > maxVal) maxVal = info.ECGsignal;
    if (info.bloodoxygen > maxVal) maxVal = info.bloodoxygen;
    if (info.respiratoryrate > maxVal) maxVal = info.respiratoryrate;

    axisY->setRange(0, maxVal * 1.2);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // 设置图表主题
    chart->setTheme(QChart::ChartThemeLight);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    // 创建图表视图
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // 删除现有布局中的所有部件
    QLayoutItem *child;
    while ((child = ui->gridLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }

    // 将图表视图添加到网格布局
    ui->gridLayout->addWidget(chartView, 0, 0);

    qDebug() << "图表已创建并添加到布局";
}
