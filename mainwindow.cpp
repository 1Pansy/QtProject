#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCoreApplication>
#include <QFile>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QSqlQuery>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_ptruserSql(nullptr)
{
    ui->setupUi(this);

    // 安装事件过滤器
    ui->tableWidget->installEventFilter(this);

    // 设置输入框自动查询 & 隐藏查询按钮
    connect(ui->le_research, &QLineEdit::textChanged, this, &MainWindow::on_btn_search_clicked);
    connect(ui->le_research_2, &QLineEdit::textChanged, this, &MainWindow::on_btn_search_2_clicked);

    // 隐藏查询按钮
    ui->btn_search->setVisible(false);
    ui->btn_search_2->setVisible(false);

    // 设置表格选择整行
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget_2->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 设置表格允许多行选择
    ui->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableWidget_2->setSelectionMode(QAbstractItemView::ExtendedSelection);

    QFile f;
    f.setFileName(":/dlgdata.css");
    f.open(QIODevice::ReadOnly);
    QString strQss = f.readAll();
    m_dlgLogin.setStyleSheet(strQss);


    f.close();
    f.setFileName(":/main.css");
    f.open(QIODevice::ReadOnly);
    strQss = f.readAll();
    this->setStyleSheet(strQss);

    m_dlgLogin.show();
    auto g = [&](){
        this->show();
        qDebug()<<m_dlgLogin.m_username;
        ui->lb_user->setText(m_dlgLogin.m_username);

        setupUserAccess();
    };

    connect(&m_dlgLogin,&page_login::sendLoginSuccess,this,g);

    // 修改树形菜单项，添加更多功能节点
    ui->treeWidget->setColumnCount(1);
    QStringList l;
    l << "人体生理参数检测仪管理系统";

    QTreeWidgetItem *pf = new QTreeWidgetItem(ui->treeWidget,l);
    ui->treeWidget->addTopLevelItem(pf);

    l.clear();
    l<<"用户访问";
    QTreeWidgetItem *p1 = new QTreeWidgetItem(pf,l);

    l.clear();
    l<<"管理员管理";
    QTreeWidgetItem *p2 = new QTreeWidgetItem(pf,l);

    l.clear();
    l<<"数据查看";
    QTreeWidgetItem *p1_1 = new QTreeWidgetItem(p1, l);

    l.clear();
    l<<"数据统计";
    QTreeWidgetItem *p1_2 = new QTreeWidgetItem(p1, l);

    l.clear();
    l<<"用户管理";
    QTreeWidgetItem *p2_1 = new QTreeWidgetItem(p2, l);

    l.clear();
    l<<"系统日志";
    QTreeWidgetItem *p2_2 = new QTreeWidgetItem(p2, l);

    pf->addChild(p1);
    pf->addChild(p2);
    p1->addChild(p1_1);
    p1->addChild(p1_2);
    p2->addChild(p2_1);
    p2->addChild(p2_2);

    ui->treeWidget->expandAll();
    ui->stackedWidget->setCurrentIndex(0);

    m_ptruserSql = userSql::getinstance();
    m_ptruserSql->init();

    updateTable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_F6)
    {


    }
}

void MainWindow::on_btn_exit_clicked()
{
    exit(0);
}


void MainWindow::on_btn_simulation_clicked()
{
    // 弹出确认对话框
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认",
        "是否要生成默认模拟数据？\n这将创建约40条测试数据。",
        QMessageBox::Yes | QMessageBox::No);

    if(reply != QMessageBox::Yes) {
        return;
    }

    // 制作数据
    QRandomGenerator a,b,c,d,e,f;
    a.seed(0);
    b.seed(0);
    c.seed(0);
    d.seed(0);
    e.seed(0);
    f.seed(0);


    QList<BaseInfo> l;

    for (int i = 0; i <m_lNames.size(); i++) {

        double temperature = a.bounded(1.0) + 36.0;
        auto bloodPressure = b.bounded(40,211);
        auto ECGSignal = c.bounded(30,281);
        auto bloodOxygen = d.bounded(80,101);
        auto respiratoryRate = e.bounded(20,61);
        auto checkProjectNumber = f.bounded(1,m_lNames.size());

        BaseInfo info;

        info.name = m_lNames[i];
        info.time = "2023-12-1";
        if(i%3)
        {
            info.age = 17;
            info.time = "2023-12-1";
        }

        if(i%7)
        {
            info.age = 18;
            info.time = "2023-12-4";
        }

        if(i%2)
        {
            info.age = 20;
            info.time = "2023-12-2";
        }

        if(i%5)
        {
            info.age = 19;
            info.time = "2023-12-3";
        }

        if(i == 0)
            info.age = 18;

        QString str = QString::number(temperature, 'f', 1);
        temperature = str.toDouble();
        info.temperature = temperature;
        info.bloodpressure = bloodPressure;
        info.ECGsignal = ECGSignal;
        info.bloodoxygen = bloodOxygen;
        info.respiratoryrate = respiratoryRate;
        info.checkProjectNumber = checkProjectNumber;

        l.append(info);
    }

    if (m_ptruserSql->addData(l)) {
        // 记录操作日志
        m_ptruserSql->addLogRecord(m_dlgLogin.m_username, "批量生成数据",
                QString("生成了%1条默认模拟数据").arg(l.size()));
        QMessageBox::information(this, "成功",
                QString("成功生成%1条默认模拟数据").arg(l.size()));
    } else {
        QMessageBox::warning(this, "失败", "数据生成失败");
    }

    updateTable();
}

void MainWindow::on_btn_add_clicked()
{
    m_dlgAddData.setType(true);
    m_dlgAddData.exec();
    updateTable();
}

void MainWindow::on_btn_clear_clicked()
{
    // 第一次确认
    QMessageBox::StandardButton reply = QMessageBox::question(this, "警告",
        "确定要清空所有生理参数数据和关联的用户账号吗？\n此操作不可恢复！",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 第二次确认
        QMessageBox::StandardButton secondReply = QMessageBox::question(this, "最终确认",
            "再次确认：您真的要清空所有数据吗？\n所有生理参数数据和关联的用户账号将被永久删除！",
            QMessageBox::Yes | QMessageBox::No);

        if (secondReply == QMessageBox::Yes) {
            // 执行清空操作
            m_ptruserSql->clearDataTable();
            updateTable();
            QMessageBox::information(this, "成功", "所有数据已清空");
        }
    }
}

void MainWindow::updateTable()
{
    if (m_page == 0) { // 生理参数数据表
        // 清空表格
        ui->tableWidget->clearContents();
        ui->tableWidget->setRowCount(0);

        QList<BaseInfo> dataList;

        // 根据用户角色获取不同数据
        if (m_dlgLogin.m_userRole == ROLE_PATIENT) {
            // 患者只能看自己的数据 - 根据关联的userid获取
            int patientUserId = 0;

            // 获取当前登录患者的userid
            QList<UserInfo> users = m_ptruserSql->getAllUser();
            for (const UserInfo& user : users) {
                if (user.username == m_dlgLogin.m_username) {
                    patientUserId = user.userid;
                    break;
                }
            }

            // 只获取患者自己的数据
            if (patientUserId > 0) {
                dataList = m_ptruserSql->getDataByPatientId(patientUserId);
            }
        } else {
            // 管理员、医生和护士可以查看所有数据，使用分页
            dataList = m_ptruserSql->getPageData(m_currentPage, m_pageSize);

            // 更新分页控件状态
            updatePageControls();
        }

        // 设置表格列数和表头
        ui->tableWidget->setColumnCount(10);
        QStringList headers = {"ID", "姓名", "年龄", "体温(℃)", "血压(mmHg)",
                             "心电信号(BPM)", "血氧(%)", "呼吸率(BPM)", "检查时间", "检查项目编号"};
        ui->tableWidget->setHorizontalHeaderLabels(headers);

        // 填充数据
        for (int i = 0; i < dataList.size(); ++i) {
            ui->tableWidget->insertRow(i);
            ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(dataList[i].id)));
            ui->tableWidget->setItem(i, 1, new QTableWidgetItem(dataList[i].name));
            ui->tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(dataList[i].age)));
            ui->tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(dataList[i].temperature, 'f', 1)));
            ui->tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(dataList[i].bloodpressure)));
            ui->tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(dataList[i].ECGsignal)));
            ui->tableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(dataList[i].bloodoxygen)));
            ui->tableWidget->setItem(i, 7, new QTableWidgetItem(QString::number(dataList[i].respiratoryrate)));
            ui->tableWidget->setItem(i, 8, new QTableWidgetItem(dataList[i].time));
            ui->tableWidget->setItem(i, 9, new QTableWidgetItem(QString::number(dataList[i].checkProjectNumber)));
        }

        adjustTableRowHeightNoEmptyRows();

        // 更新数据计数和分页信息
        quint32 totalRecords = m_ptruserSql->getDataCnt();
        ui->lb_cnt->setText(QString("数量:%1").arg(totalRecords));

        // 更新分页控件
        updatePageControls();
    }
    else if (m_page == 1) { // 用户表
        ui->tableWidget_2->clearContents();
        ui->tableWidget_2->setRowCount(0);

        QList<UserInfo> userList = m_ptruserSql->getAllUser();

        // 设置列
        ui->tableWidget_2->setColumnCount(5); // 增加一列显示对应的生理参数姓名
        QStringList headers;
        headers << "用户名" << "密码" << "角色" << "用户ID" << "对应姓名";
        ui->tableWidget_2->setHorizontalHeaderLabels(headers);

        // 设置列宽均匀分布
        int tableWidth = ui->tableWidget_2->width();
        for (int i = 0; i < 5; i++) {
            ui->tableWidget_2->setColumnWidth(i, tableWidth / 5 - 2);
        }

        // 填充数据
        for (int i = 0; i < userList.size(); ++i) {
            ui->tableWidget_2->insertRow(i);

            // // 设置用户名、密码、角色和userid
            // ui->tableWidget_2->setItem(i, 0, new QTableWidgetItem(userList[i].username));
            // ui->tableWidget_2->setItem(i, 1, new QTableWidgetItem("*****")); // 不显示明文密码

            QTableWidgetItem *usernameItem = new QTableWidgetItem(userList[i].username);
            usernameItem->setTextAlignment(Qt::AlignCenter);

            // 显示明文密码
            QTableWidgetItem *passwordItem = new QTableWidgetItem(userList[i].password);
            passwordItem->setTextAlignment(Qt::AlignCenter);

            // 获取角色文本
            QString roleText;
            switch (userList[i].role) {
            case ROLE_ADMIN: roleText = "管理员"; break;
            case ROLE_DOCTOR: roleText = "医生"; break;
                case ROLE_NURSE: roleText = "护士"; break;
                case ROLE_PATIENT: roleText = "患者"; break;
                    default: roleText = "未知"; break;
            }
            ui->tableWidget_2->setItem(i, 2, new QTableWidgetItem(roleText));

            // 设置userid
            ui->tableWidget_2->setItem(i, 3, new QTableWidgetItem(QString::number(userList[i].userid)));

            ui->tableWidget_2->setItem(i, 0, usernameItem);
            ui->tableWidget_2->setItem(i, 1, passwordItem);

            // 获取并显示对应的生理参数姓名（只对患者角色）
            QString realName = "";
            if (userList[i].role == ROLE_PATIENT && userList[i].userid > 0) {
                // 使用公共方法获取姓名
                realName = m_ptruserSql->getNameById(userList[i].userid);
            }
            ui->tableWidget_2->setItem(i, 4, new QTableWidgetItem(realName));
        }

        ui->lb_cnt_2->setText(QString("数量:%1").arg(userList.size()));
    }
}


void MainWindow::on_btn_del_clicked()
{
    // 检查权限
    if (m_dlgLogin.m_userRole != ROLE_ADMIN && m_dlgLogin.m_userRole != ROLE_DOCTOR) {
        QMessageBox::warning(this, "权限不足", "只有管理员和医生可以删除数据");
        return;
    }

    // 获取所有选中行
    QList<int> selectedRows;
    QModelIndexList selection = ui->tableWidget->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::information(this, "提示", "请选择要删除的数据");
        return;
    }

    // 提取所有选中行的ID
    QList<int> ids;
    for (const QModelIndex& index : selection) {
        int row = index.row();
        QTableWidgetItem* idItem = ui->tableWidget->item(row, 0);

        // 防止空行和无效数据
        if (!idItem || idItem->text().isEmpty()) {
            QMessageBox::warning(this, "警告",
                "第 " + QString::number(row+1) + " 行没有有效的ID，无法删除");
            continue;
        }

        bool ok;
        int id = idItem->text().toInt(&ok);
        if (!ok || id <= 0) {
            QMessageBox::warning(this, "警告",
                "第 " + QString::number(row+1) + " 行的ID无效，无法删除");
            continue;
        }

        ids.append(id);
    }

    if (ids.isEmpty()) {
        QMessageBox::warning(this, "警告", "没有有效的数据可删除");
        return;
    }

    // 添加确认对话框
    QMessageBox::StandardButton reply = QMessageBox::question(this, "删除确认",
        QString("确定要删除选中的 %1 条数据吗？").arg(ids.size()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        int successCount = 0;
        for (int id : ids) {
            if (m_ptruserSql->delData(id)) {
                successCount++;
            }
        }

        // 记录日志
        m_ptruserSql->addLogRecord(m_ptruserSql->m_currentUser, "批量删除数据",
                        QString("删除了%1条生理参数数据").arg(successCount));

        QMessageBox::information(this, "删除结果",
                QString("成功删除 %1 条数据，失败 %2 条").arg(successCount).arg(ids.size() - successCount));
        // 删除后更新表格
        if (m_isSearchMode) {
            updateTableWithSearch();
        } else {
            updateTable();
        }
    }
}


void MainWindow::on_btn_update_clicked()
{
    QModelIndexList selectedList = ui->tableWidget->selectionModel()->selectedRows();

    // 检查是否选中了多行数据
    if (selectedList.size() > 1) {
        QMessageBox::warning(this, "提示", "只能选择一行数据进行修改");
        return;
    }

    BaseInfo info;
    int i = ui->tableWidget->currentRow();
    if(i>=0)
    {
        info.id = ui->tableWidget->item(i,0)->text().toUInt();
        info.name = ui->tableWidget->item(i,1)->text();
        info.age = ui->tableWidget->item(i,2)->text().toUInt();
        info.temperature = ui->tableWidget->item(i,3)->text().toDouble();
        info.bloodpressure = ui->tableWidget->item(i,4)->text().toUInt();
        info.ECGsignal = ui->tableWidget->item(i,5)->text().toUInt();
        info.bloodoxygen = ui->tableWidget->item(i,6)->text().toUInt();
        info.respiratoryrate = ui->tableWidget->item(i,7)->text().toUInt();
        info.time = ui->tableWidget->item(i,8)->text();
        info.checkProjectNumber = ui->tableWidget->item(i,9)->text().toUInt();

        m_dlgAddData.setType(false,info);
        m_dlgAddData.exec();
        updateTable();
    }
    else
    {
        QMessageBox::information(nullptr,"警告","请选中一行数据");
    }
}


void MainWindow::on_btn_search_clicked()
{
    m_currentSearchText = ui->le_research->text();
    m_isSearchMode = !m_currentSearchText.isEmpty();

    if(!m_isSearchMode)
    {
        // 清空搜索状态，返回正常显示
        updateTable();
        return;
    }

    // 重置到第一页
    m_currentPage = 1;

    // 执行搜索并显示结果（带分页）
    updateTableWithSearch();
}



void MainWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    // 获取点击项的文本
    QString itemText = item->text(column);

    // 根据点击项的文本切换页面
    if (itemText == "用户访问" || itemText == "数据查看") {
        ui->stackedWidget->setCurrentIndex(0); // 生理参数数据表页面
        m_page = 0;

        // 清空查询框和搜索状态
        ui->le_research->clear(); // 假设搜索框的对象名是 le_research

        // 如果有搜索模式标志，也要重置它
        m_isSearchMode = false;
        m_currentSearchText = "";

        updateTable(); // 更新数据表
    }
    else if (itemText == "数据统计") {
        // 调用数据统计功能
        showStatisticsDialog();
    }
    else if (itemText == "管理员管理" || itemText == "用户管理") {
        // 清空用户管理页的搜索框
        ui->le_research_2->clear();

        ui->stackedWidget->setCurrentIndex(1); // 用户管理页面
        m_page = 1;
        updateTable(); // 更新用户表
    }
    else if (itemText == "系统日志") {
        // 显示系统日志对话框
        on_btn_viewLogs_clicked();
    }
}

void MainWindow::showStatisticsDialog()
{
    // 创建统计结果对话框
    QDialog statsDialog(this);
    statsDialog.setWindowTitle("生理参数统计分析");
    statsDialog.setMinimumSize(800, 600);

    // 创建布局
    QVBoxLayout *layout = new QVBoxLayout(&statsDialog);

    // 获取统计数据
    double avgTemp = 0.0, minTemp = 0.0, maxTemp = 0.0;
    int avgBP = 0, minBP = 0, maxBP = 0;
    int avgECG = 0, minECG = 0, maxECG = 0;
    int avgO2 = 0, minO2 = 0, maxO2 = 0;
    int avgRR = 0, minRR = 0, maxRR = 0;
    int totalCount = 0;

    if (m_ptruserSql->getStatisticsData(avgTemp, minTemp, maxTemp,
                                      avgBP, minBP, maxBP,
                                      avgECG, minECG, maxECG,
                                      avgO2, minO2, maxO2,
                                      avgRR, minRR, maxRR,
                                      totalCount)) {
        // 创建统计信息表格
        QTableWidget *statsTable = new QTableWidget(5, 4, &statsDialog);
        statsTable->setHorizontalHeaderLabels({"生理参数", "平均值", "最小值", "最大值"});
        statsTable->setVerticalHeaderLabels({"体温(℃)", "血压(mmHg)", "心电信号(BPM)", "血氧(%)", "呼吸率(BPM)"});

        // 设置表格大小
        statsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        statsTable->verticalHeader()->setDefaultSectionSize(40);

        // 填充表格
        // 体温行
        statsTable->setItem(0, 0, new QTableWidgetItem("体温(℃)"));
        statsTable->setItem(0, 1, new QTableWidgetItem(QString::number(avgTemp, 'f', 1)));
        statsTable->setItem(0, 2, new QTableWidgetItem(QString::number(minTemp, 'f', 1)));
        statsTable->setItem(0, 3, new QTableWidgetItem(QString::number(maxTemp, 'f', 1)));

        // 血压行
        statsTable->setItem(1, 0, new QTableWidgetItem("血压(mmHg)"));
        statsTable->setItem(1, 1, new QTableWidgetItem(QString::number(avgBP)));
        statsTable->setItem(1, 2, new QTableWidgetItem(QString::number(minBP)));
        statsTable->setItem(1, 3, new QTableWidgetItem(QString::number(maxBP)));

        // 心电信号行
        statsTable->setItem(2, 0, new QTableWidgetItem("心电信号(BPM)"));
        statsTable->setItem(2, 1, new QTableWidgetItem(QString::number(avgECG)));
        statsTable->setItem(2, 2, new QTableWidgetItem(QString::number(minECG)));
        statsTable->setItem(2, 3, new QTableWidgetItem(QString::number(maxECG)));

        // 血氧行
        statsTable->setItem(3, 0, new QTableWidgetItem("血氧(%)"));
        statsTable->setItem(3, 1, new QTableWidgetItem(QString::number(avgO2)));
        statsTable->setItem(3, 2, new QTableWidgetItem(QString::number(minO2)));
        statsTable->setItem(3, 3, new QTableWidgetItem(QString::number(maxO2)));

        // 呼吸率行
        statsTable->setItem(4, 0, new QTableWidgetItem("呼吸率(BPM)"));
        statsTable->setItem(4, 1, new QTableWidgetItem(QString::number(avgRR)));
        statsTable->setItem(4, 2, new QTableWidgetItem(QString::number(minRR)));
        statsTable->setItem(4, 3, new QTableWidgetItem(QString::number(maxRR)));

        // 设置表格样式
        for (int row = 0; row < statsTable->rowCount(); ++row) {
            for (int col = 0; col < statsTable->columnCount(); ++col) {
                if (statsTable->item(row, col)) {
                    statsTable->item(row, col)->setTextAlignment(Qt::AlignCenter);
                }
            }
        }

        // 添加标题和汇总信息
        QLabel *titleLabel = new QLabel("生理参数统计分析", &statsDialog);
        titleLabel->setAlignment(Qt::AlignCenter);
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(14);
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);

        QLabel *summaryLabel = new QLabel(QString("分析样本总数：%1条记录").arg(totalCount), &statsDialog);
        summaryLabel->setAlignment(Qt::AlignCenter);
        QFont summaryFont = summaryLabel->font();
        summaryFont.setPointSize(11);
        summaryLabel->setFont(summaryFont);

        // 添加到布局
        layout->addWidget(titleLabel);
        layout->addWidget(summaryLabel);
        layout->addWidget(statsTable);

        // 创建统计图表的标签页控件
        QTabWidget *chartTabs = new QTabWidget(&statsDialog);

        // 创建平均值柱状图
        QWidget *avgChartWidget = new QWidget(chartTabs);
        QVBoxLayout *avgChartLayout = new QVBoxLayout(avgChartWidget);

        QBarSet *avgSet = new QBarSet("平均值");
        *avgSet << avgTemp << avgBP << avgECG << avgO2 << avgRR;

        QBarSeries *avgSeries = new QBarSeries();
        avgSeries->append(avgSet);

        QChart *avgChart = new QChart();
        avgChart->addSeries(avgSeries);
        avgChart->setTitle("各生理参数平均值");
        avgChart->setAnimationOptions(QChart::SeriesAnimations);

        QStringList categories;
        categories << "体温(℃)" << "血压(mmHg)" << "心电信号(BPM)" << "血氧(%)" << "呼吸率(BPM)";

        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        axisX->append(categories);
        avgChart->addAxis(axisX, Qt::AlignBottom);
        avgSeries->attachAxis(axisX);

        QValueAxis *axisY = new QValueAxis();
        axisY->setRange(0, qMax(qMax(qMax(qMax(avgTemp, (double)avgBP), (double)avgECG), (double)avgO2), (double)avgRR) * 1.2);
        avgChart->addAxis(axisY, Qt::AlignLeft);
        avgSeries->attachAxis(axisY);

        QChartView *avgChartView = new QChartView(avgChart);
        avgChartView->setRenderHint(QPainter::Antialiasing);
        avgChartLayout->addWidget(avgChartView);

        // 创建最大值-最小值对比柱状图
        QWidget *minMaxChartWidget = new QWidget(chartTabs);
        QVBoxLayout *minMaxChartLayout = new QVBoxLayout(minMaxChartWidget);

        QBarSet *minSet = new QBarSet("最小值");
        QBarSet *maxSet = new QBarSet("最大值");

        *minSet << minTemp << minBP << minECG << minO2 << minRR;
        *maxSet << maxTemp << maxBP << maxECG << maxO2 << maxRR;

        QBarSeries *minMaxSeries = new QBarSeries();
        minMaxSeries->append(minSet);
        minMaxSeries->append(maxSet);

        QChart *minMaxChart = new QChart();
        minMaxChart->addSeries(minMaxSeries);
        minMaxChart->setTitle("各生理参数最大值-最小值对比");
        minMaxChart->setAnimationOptions(QChart::SeriesAnimations);

        QBarCategoryAxis *minMaxAxisX = new QBarCategoryAxis();
        minMaxAxisX->append(categories);
        minMaxChart->addAxis(minMaxAxisX, Qt::AlignBottom);
        minMaxSeries->attachAxis(minMaxAxisX);

        QValueAxis *minMaxAxisY = new QValueAxis();
        double yMax = qMax(qMax(qMax(qMax(maxTemp, (double)maxBP), (double)maxECG), (double)maxO2), (double)maxRR) * 1.2;
        minMaxAxisY->setRange(0, yMax);
        minMaxChart->addAxis(minMaxAxisY, Qt::AlignLeft);
        minMaxSeries->attachAxis(minMaxAxisY);

        QChartView *minMaxChartView = new QChartView(minMaxChart);
        minMaxChartView->setRenderHint(QPainter::Antialiasing);
        minMaxChartLayout->addWidget(minMaxChartView);

        // 添加标签页
        chartTabs->addTab(avgChartWidget, "平均值统计图");
        chartTabs->addTab(minMaxChartWidget, "最大值-最小值对比图");

        // 添加图表到主布局
        layout->addWidget(chartTabs);

        // 添加关闭按钮
        QPushButton *closeButton = new QPushButton("关闭", &statsDialog);
        layout->addWidget(closeButton);
        connect(closeButton, &QPushButton::clicked, &statsDialog, &QDialog::accept);

        // 记录日志
        m_ptruserSql->addLogRecord(m_ptruserSql->m_currentUser, "查看统计分析",
                                 QString("查看了%1条记录的统计分析数据").arg(totalCount));
    } else {
        QLabel *noDataLabel = new QLabel("没有可用的统计数据。", &statsDialog);
        noDataLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(noDataLabel);

        QPushButton *closeButton = new QPushButton("关闭", &statsDialog);
        layout->addWidget(closeButton);
        connect(closeButton, &QPushButton::clicked, &statsDialog, &QDialog::accept);
    }

    // 显示对话框
    statsDialog.exec();
}

void MainWindow::on_btn_add_2_clicked()
{
    m_dlgAddUser.setType(true);
    m_dlgAddUser.exec();
    updateTable();
}


void MainWindow::on_btn_update_2_clicked()
{
    QModelIndexList selectedList = ui->tableWidget_2->selectionModel()->selectedRows();

    // 检查是否选中了多行数据
    if (selectedList.size() > 1) {
        QMessageBox::warning(this, "提示", "只能选择一行用户数据进行修改");
        return;
    }

    UserInfo info;
    int i = ui->tableWidget_2->currentRow();

    if(i >= 0)
    {
        // 从表格中获取数据
        info.username = ui->tableWidget_2->item(i, 0)->text();
        info.password = ui->tableWidget_2->item(i, 1)->text();

        // 根据角色文本设置角色
        QString roleText = ui->tableWidget_2->item(i, 2)->text();
        if (roleText == "管理员") {
            info.role = ROLE_ADMIN;
            info.aut = "admin";
        } else if (roleText == "医生") {
            info.role = ROLE_DOCTOR;
            info.aut = "doctor";
        } else if (roleText == "护士") {
            info.role = ROLE_NURSE;
            info.aut = "nurse";
        } else {
            info.role = ROLE_PATIENT;
            info.aut = "user";
        }

        // 获取userid
        info.userid = ui->tableWidget_2->item(i, 3)->text().toInt();

        m_dlgAddUser.setType(false, info);
        m_dlgAddUser.exec();
        updateTable();
    }
    else
    {
        QMessageBox::information(nullptr, "警告", "请选中一行数据");
    }
}


void MainWindow::on_btn_del_2_clicked()
{
    // 检查权限
    if (m_dlgLogin.m_userRole != ROLE_ADMIN) {
        QMessageBox::warning(this, "权限不足", "只有管理员可以删除用户");
        return;
    }

    // 获取所有选中行
    QModelIndexList selection = ui->tableWidget_2->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::information(this, "提示", "请选择要删除的用户");
        return;
    }

    // 提取所有选中行的用户名
    QStringList usernames;
    for (const QModelIndex& index : selection) {
        int row = index.row();
        QString username = ui->tableWidget_2->item(row, 0)->text();
        usernames.append(username);
    }

    // 添加确认对话框
    QMessageBox::StandardButton reply = QMessageBox::question(this, "删除确认",
        QString("确定要删除选中的 %1 个用户吗？\n注意：关联的生理参数数据也将被删除！").arg(usernames.size()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        int successCount = 0;
        for (const QString& username : usernames) {
            if (m_ptruserSql->delUser(username)) {
                successCount++;
            }
        }

        // 记录日志
        m_ptruserSql->addLogRecord(m_ptruserSql->m_currentUser, "批量删除用户",
                        QString("删除了%1个用户账号").arg(successCount));

        QMessageBox::information(this, "删除结果",
                QString("成功删除 %1 个用户，失败 %2 个").arg(successCount).arg(usernames.size() - successCount));
        updateTable();
    }
}


void MainWindow::on_btn_search_2_clicked()
{
    QString strFilter = ui->le_research_2->text();
    if(strFilter.isEmpty())
    {
        updateTable();
        return;
    }

    ui->tableWidget_2->clear();
    ui->tableWidget_2->setColumnCount(4); // 修改为4列
    QStringList l;
    l << "用户名" << "密码" << "角色" << "用户ID"; // 添加用户ID
    ui->tableWidget_2->setHorizontalHeaderLabels(l);

    // 设置列宽均匀分布
    int tableWidth = ui->tableWidget_2->width();
    for (int i = 0; i < 4; i++) {
        ui->tableWidget_2->setColumnWidth(i, tableWidth / 4 - 2);
    }

    // 设置自动调整列宽
    ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QList<UserInfo> luser = m_ptruserSql->getAllUser();
    auto cnt = luser.size();
    ui->lb_cnt_2->setText(QString("数量:%1").arg(cnt));

    int index = 0;
    for (int i = 0; i < luser.size(); ++i) {
        if(!luser[i].username.contains(strFilter))
        {
            continue;
        }

        // 创建带居中对齐的单元格项
        ui->tableWidget_2->insertRow(index);

        QTableWidgetItem *usernameItem = new QTableWidgetItem(luser[i].username);
        usernameItem->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *passwordItem = new QTableWidgetItem(luser[i].password);
        passwordItem->setTextAlignment(Qt::AlignCenter);

        // 角色文本
        QString roleText;
        switch (luser[i].role) {
            case ROLE_ADMIN: roleText = "管理员"; break;
            case ROLE_DOCTOR: roleText = "医生"; break;
            case ROLE_NURSE: roleText = "护士"; break;
            case ROLE_PATIENT: roleText = "患者"; break;
            default: roleText = "未知"; break;
        }

        QTableWidgetItem *roleItem = new QTableWidgetItem(roleText);
        roleItem->setTextAlignment(Qt::AlignCenter);

        // 添加userid项
        QTableWidgetItem *useridItem = new QTableWidgetItem(QString::number(luser[i].userid));
        useridItem->setTextAlignment(Qt::AlignCenter);

        ui->tableWidget_2->setItem(index, 0, usernameItem);
        ui->tableWidget_2->setItem(index, 1, passwordItem);
        ui->tableWidget_2->setItem(index, 2, roleItem);
        ui->tableWidget_2->setItem(index, 3, useridItem);
        index++;
    }

    ui->tableWidget_2->setRowCount(index);
    ui->lb_cnt_2->setText(QString("数量:%1").arg(index));
}

// void MainWindow::on_btn_chart_clicked()
// {
//     m_viewCharts.exec();
//     this->hide();
// }


void MainWindow::on_btn_chart_clicked()
{
    // 检查是否有选中的数据行
    QModelIndexList selectedList = ui->tableWidget->selectionModel()->selectedRows();
    if (selectedList.size() <= 0) {
        QMessageBox::warning(this, "提示", "请先选择一条数据用于生成图表");
        return;
    }

    // 检查是否选中了多行数据
    if (selectedList.size() > 1) {
        QMessageBox::warning(this, "提示", "只能选择一行数据用于生成图表");
        return;
    }

    // 获取选中行的数据
    int row = selectedList.at(0).row();
    int id = ui->tableWidget->item(row, 0)->text().toInt();

    // 从表格获取数据
    BaseInfo info;
    info.id = id;
    info.name = ui->tableWidget->item(row, 1)->text();
    info.age = ui->tableWidget->item(row, 2)->text().toInt();
    info.temperature = ui->tableWidget->item(row, 3)->text().toDouble();
    info.bloodpressure = ui->tableWidget->item(row, 4)->text().toInt();
    info.ECGsignal = ui->tableWidget->item(row, 5)->text().toInt();
    info.bloodoxygen = ui->tableWidget->item(row, 6)->text().toInt();
    info.respiratoryrate = ui->tableWidget->item(row, 7)->text().toInt();
    info.time = ui->tableWidget->item(row, 8)->text();
    info.checkProjectNumber = ui->tableWidget->item(row, 9)->text().toInt();

    // 创建并显示图表窗口
    // 使用this作为父窗口，确保子窗口随主窗口关闭
    charts *chartWindow = new charts(this);

    // 设置窗口标志，让它看起来像独立窗口但仍然与父窗口相关联
    chartWindow->setWindowFlags(Qt::Window);

    chartWindow->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动删除
    chartWindow->createBarChart(info);
    chartWindow->show();

    // 记录日志
    m_ptruserSql->addLogRecord(m_ptruserSql->m_currentUser, "查看图表",
            "查看了ID为 " + QString::number(id) + " 的数据图表");
}

void MainWindow::on_pushButton_clicked()
{
    updateTable();
}

void MainWindow::on_btn_viewLogs_clicked()
{
    // 检查用户权限
    if (m_dlgLogin.m_userRole != ROLE_ADMIN) {
        QMessageBox::warning(this, "权限不足", "只有管理员可以查看系统日志");
        return;
    }

    // 刷新日志并显示日志查看器
    m_logViewer.refreshLogs();
    m_logViewer.exec();
}

void MainWindow::on_btn_batchGenerate_clicked()
{
    // 批量生成数据对话框
    QDialog dlg(this);
    dlg.setWindowTitle("批量生成模拟数据");
    dlg.setMinimumWidth(300);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);

    // 添加输入控件
    QLabel *lblCount = new QLabel("生成数量:", &dlg);
    QSpinBox *spinCount = new QSpinBox(&dlg);
    spinCount->setRange(1, 1000);
    spinCount->setValue(20);

    QLabel *lblAgeRange = new QLabel("年龄范围:", &dlg);
    QHBoxLayout *ageLayout = new QHBoxLayout();
    QSpinBox *spinMinAge = new QSpinBox(&dlg);
    spinMinAge->setRange(1, 120);
    spinMinAge->setValue(18);
    QLabel *lblTo = new QLabel("至", &dlg);
    QSpinBox *spinMaxAge = new QSpinBox(&dlg);
    spinMaxAge->setRange(1, 120);
    spinMaxAge->setValue(65);
    ageLayout->addWidget(spinMinAge);
    ageLayout->addWidget(lblTo);
    ageLayout->addWidget(spinMaxAge);

    // 日期选择
    QLabel *lblDate = new QLabel("检查日期:", &dlg);
    QDateEdit *dateEdit = new QDateEdit(QDate::currentDate(), &dlg);
    dateEdit->setCalendarPopup(true);

    // 按钮
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);

    // 添加到布局
    layout->addWidget(lblCount);
    layout->addWidget(spinCount);
    layout->addWidget(lblAgeRange);
    layout->addLayout(ageLayout);
    layout->addWidget(lblDate);
    layout->addWidget(dateEdit);
    layout->addWidget(buttonBox);

    // 连接按钮信号
    connect(buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    // 执行对话框
    if(dlg.exec() == QDialog::Accepted) {
        int count = spinCount->value();
        int minAge = spinMinAge->value();
        int maxAge = spinMaxAge->value();
        QString dateStr = dateEdit->date().toString("yyyy-MM-dd");

        // 生成随机数据
        QRandomGenerator random;
        random.seed(QDateTime::currentMSecsSinceEpoch());

        // 随机姓名生成器
        QStringList familyNames = {
            "李", "王", "张", "刘", "陈", "杨", "赵", "黄", "周", "吴",
            "徐", "孙", "胡", "朱", "高", "林", "何", "郭", "马", "罗",
            "梁", "宋", "郑", "谢", "韩", "唐", "冯", "于", "董", "萧",
            "程", "曹", "袁", "邓", "许", "傅", "沈", "曾", "彭", "吕",
            "苏", "卢", "蒋", "蔡", "贾", "丁", "魏", "薛", "叶", "阎"
        };

        QStringList givenNames = {
            "伟", "芳", "娜", "秀英", "敏", "静", "丽", "强", "磊", "军",
            "洋", "勇", "艳", "杰", "娟", "涛", "明", "超", "秀兰", "霞",
            "平", "刚", "桂英", "文", "云", "建", "民", "华", "红", "燕",
            "晓", "玲", "鑫", "欣", "龙", "玉", "凯", "毅", "锋", "浩"
        };

        QList<BaseInfo> dataList;

        for(int i = 0; i < count; ++i) {
            BaseInfo info;

            // 随机姓名
            QString familyName = familyNames[random.bounded(familyNames.size())];
            QString givenName = givenNames[random.bounded(givenNames.size())];
            info.name = familyName + givenName;

            // 随机年龄
            info.age = random.bounded(minAge, maxAge + 1);

            // 随机生理数据 - 在合理范围内
            info.temperature = random.generateDouble() * (39.5 - 36.0) + 36.0;
            info.temperature = double(int(info.temperature * 10)) / 10.0; // 保留一位小数

            info.bloodpressure = random.bounded(40, 211);
            info.ECGsignal = random.bounded(30, 281);
            info.bloodoxygen = random.bounded(80, 100);
            info.respiratoryrate = random.bounded(20, 60);

            // 检查时间
            info.time = dateStr;

            // 检查项目编号
            info.checkProjectNumber = random.bounded(1, 1001);

            dataList.append(info);
        }

        qDebug() << "准备批量插入" << dataList.size() << "条记录";

        // 批量插入数据
        bool success = m_ptruserSql->addData(dataList);

        if(success) {
            // 记录操作日志
            m_ptruserSql->addLogRecord(m_ptruserSql->m_currentUser, "批量生成数据",
                QString("生成了%1条模拟数据").arg(count));

            QMessageBox::information(this, "成功",
                QString("已成功生成%1条模拟数据").arg(count));

            // 更新表格显示
            updateTable();
        } else {
            QMessageBox::critical(this, "错误", "生成数据失败，请检查数据有效性");
        }
    }
}

void MainWindow::setupUserAccess()
{
    // 获取当前用户角色和信息
    UserRole userRole = m_dlgLogin.m_userRole;
    QString currentUser = m_ptruserSql->m_currentUser;

    // 检查是否为管理员
    bool isAdmin = (userRole == ROLE_ADMIN);
    bool isDoctor = (userRole == ROLE_DOCTOR);
    bool isNurse = (userRole == ROLE_NURSE);
    bool isPatient = (userRole == ROLE_PATIENT);

    qDebug() << "设置权限 - 用户:" << currentUser
             << "角色:" << userRole
             << "是管理员:" << isAdmin;

    // 设置数据管理权限
    // 1. 医生和管理员可以新增和删除数据 - 护士和患者看不到这些按钮
    ui->btn_add->setVisible(isAdmin || isDoctor);
    ui->btn_del->setVisible(isAdmin || isDoctor);
    ui->btn_batchGenerate->setVisible(isAdmin || isDoctor);
    ui->btn_clear->setVisible(isAdmin || isDoctor);

    // 2. 医生、管理员和护士可以修改数据
    ui->btn_update->setVisible(isAdmin || isDoctor || isNurse);

    // 设置分页控件可见性
    bool canViewAllData = (isAdmin || isDoctor || isNurse);
    // 确保分页控件容器对管理员、医生和护士可见
    ui->widget->setVisible(canViewAllData);
    // 如果是患者 隐藏分页控件
    if (isPatient) {
        ui->widget->setVisible(false);
    }

    // 3. 设置管理员功能按钮可见性
    ui->btn_add_2->setVisible(isAdmin);
    ui->btn_update_2->setVisible(isAdmin);
    ui->btn_del_2->setVisible(isAdmin);

    // 4. 设置数据导入导出按钮可见性
    // 生理参数数据的导入/导出只允许管理员和医生使用
    ui->btn_exportData->setVisible(isAdmin || isDoctor);
    ui->btn_importData->setVisible(isAdmin || isDoctor);

    // 用户表数据的导入/导出只允许管理员使用
    ui->btn_exportUser->setVisible(isAdmin);
    ui->btn_importUser->setVisible(isAdmin);

    // 5. 根据角色更新数据表显示
    updateTable();

    // 6. 如果不是管理员，则禁用树形菜单中的管理员相关项
    if (!isAdmin) {
        // 找到管理员管理项并禁用它
        QTreeWidgetItemIterator it(ui->treeWidget);
        while (*it) {
            if ((*it)->text(0) == "管理员管理") {
                (*it)->setHidden(true);
                break;
            }
            ++it;
        }
    } else {
        // 管理员可以看到所有菜单项
        QTreeWidgetItemIterator it(ui->treeWidget);
        while (*it) {
            (*it)->setHidden(false);
            ++it;
        }
    }

    // 7. 隐藏医生不需要的菜单项
    if (isDoctor) {
        QTreeWidgetItemIterator it(ui->treeWidget);
        while (*it) {
            if ((*it)->text(0) == "管理员管理" ||
                (*it)->text(0) == "用户管理"   ||
                (*it)->text(0) == "系统日志") {
                (*it)->setHidden(true);
            }
            ++it;
        }
    }

    // 8. 隐藏护士不需要的菜单项
    if (isNurse) {
        QTreeWidgetItemIterator it(ui->treeWidget);
        while (*it) {
            if ((*it)->text(0) == "管理员管理" ||
                (*it)->text(0) == "用户管理"   ||
                (*it)->text(0) == "系统日志") {
                (*it)->setHidden(true);
            }
            ++it;
        }
    }

    // 9. 患者只能看到数据查看
    if (isPatient) {
        QTreeWidgetItemIterator it(ui->treeWidget);
        while (*it) {
            if ((*it)->text(0) != "用户访问" &&
                (*it)->text(0) != "数据查看" &&
                (*it)->text(0) != "人体生理参数检测仪管理系统") {
                (*it)->setHidden(true);
            }
            ++it;
        }
    }
}

void MainWindow::on_btn_swuser_clicked()
{
    qDebug() << "切换用户按钮被点击"; // 添加调试输出

    // 提示确认
    QMessageBox::StandardButton reply = QMessageBox::question(this, "切换用户",
        "确定要切换用户吗？\n当前未保存的操作可能会丢失。",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {
        return;
    }

    // 记录日志
    m_ptruserSql->addLogRecord(m_ptruserSql->m_currentUser, "切换用户",
        QString("用户 %1 执行切换用户操作").arg(m_ptruserSql->m_currentUser));

    // 隐藏主窗口
    hide();

    // 重置页面状态
    ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(0));
    m_page = 0;

    m_dlgLogin.clearInputs();

    // 重新连接登录成功信号
    disconnect(&m_dlgLogin, &page_login::sendLoginSuccess, nullptr, nullptr);

    connect(&m_dlgLogin, &page_login::sendLoginSuccess, this, [this]() {
        // 登录成功，刷新界面状态

        // 更新UI显示的用户名
        ui->lb_user->setText(m_dlgLogin.m_username);

        // 保存新的当前用户到userSql类
        m_ptruserSql->setCurrentUser(m_dlgLogin.m_username);

        // 根据新用户角色更新权限
        setupUserAccess();

        // 重置到第一页
        m_currentPage = 1;

        // 如果非管理员用户，强制切换到数据查看页（避免数据泄露）
        if (m_dlgLogin.m_userRole != ROLE_ADMIN && ui->stackedWidget->currentIndex() == 1) {
            ui->stackedWidget->setCurrentIndex(0);
            m_page = 0;
        }

        // 更新数据表格
        updateTable();

        // 显示主窗口
        show();

        // 记录登录成功日志
        m_ptruserSql->addLogRecord(m_dlgLogin.m_username, "用户切换",
            QString("用户 %1 切换登录成功").arg(m_dlgLogin.m_username));
    });

    // 显示登录窗口
    m_dlgLogin.show();

    // 如果登录窗口被关闭而不是登录成功，则退出程序
    connect(&m_dlgLogin, &QWidget::destroyed, this, [this]() {
        if (!isVisible()) {
            QApplication::quit();
        }
    });
}

void MainWindow::on_actionSwitchUser_triggered()
{
    // 空实现 解决链接错误
    // 可以转发到切换用户按钮的功能
    on_btn_swuser_clicked();
}

void MainWindow::on_btn_exportData_clicked()
{
    // 权限检查
    if (m_dlgLogin.m_userRole != ROLE_ADMIN && m_dlgLogin.m_userRole != ROLE_DOCTOR) {
        QMessageBox::warning(this, "权限不足", "只有管理员和医生可以导出生理参数数据");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
            "导出生理参数数据", QDir::homePath() + "/physiological_data.csv",
            "CSV文件 (*.csv);;所有文件 (*.*)");

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开文件进行写入");
        return;
    }

    // 添加BOM头解决中文乱码
    file.write("\xEF\xBB\xBF");

    QTextStream stream(&file);

    // 写入表头
    stream << "ID,姓名,年龄,体温,血压,心电信号,血氧,呼吸率,检查时间,检查项目编号\n";

    // 获取总记录数
    quint32 totalRecords = m_ptruserSql->getDataCnt();

    // 批量处理数据，避免一次性加载过多数据
    const int batchSize = 500;
    int totalBatches = (totalRecords + batchSize - 1) / batchSize;
    int totalExported = 0;

    QProgressDialog progress("导出数据中...", "取消", 0, totalBatches, this);
    progress.setWindowModality(Qt::WindowModal);

    for (int batch = 1; batch <= totalBatches; batch++) {
        // 每次获取一批数据
        QList<BaseInfo> batchData = m_ptruserSql->getPageData(batch, batchSize);

        for (const BaseInfo& data : batchData) {
            // 写入每一行数据
            stream << data.id << ","
                  << data.name << ","
                  << data.age << ","
                  << data.temperature << ","
                  << data.bloodpressure << ","
                  << data.ECGsignal << ","
                  << data.bloodoxygen << ","
                  << data.respiratoryrate << ","
                  << data.time << ","
                  << data.checkProjectNumber << "\n";
            totalExported++;
        }

        // 更新进度
        progress.setValue(batch);
        if (progress.wasCanceled())
            break;
    }

    file.close();

    // 记录日志
    m_ptruserSql->addLogRecord(m_ptruserSql->m_currentUser, "导出生理参数数据",
                             QString("导出了%1条生理参数数据").arg(totalExported));

    QMessageBox::information(this, "成功", QString("成功导出 %1 条生理参数数据").arg(totalExported));
}


void MainWindow::on_btn_importData_clicked()
{
    // 权限检查
    if (m_dlgLogin.m_userRole != ROLE_ADMIN && m_dlgLogin.m_userRole != ROLE_DOCTOR) {
        QMessageBox::warning(this, "权限不足", "只有管理员和医生可以导入生理参数数据");
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this,
            "导入生理参数数据", QDir::homePath(),
            "CSV文件 (*.csv);;所有文件 (*.*)");

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开文件进行读取");
        return;
    }

    QByteArray data = file.readAll();
    int startPos = 0;

    // 检查BOM
    if (data.startsWith("\xEF\xBB\xBF"))
        startPos = 3;

    QString content = QString::fromUtf8(data.mid(startPos));
    QTextStream in(&content);

    // 读取表头
    QString header = in.readLine();

    QList<BaseInfo> importData;
    int lineNumber = 1;

    // 逐行读取数据
    while (!in.atEnd()) {
        lineNumber++;
        QString line = in.readLine();
        QStringList fields = line.split(",");

        if (fields.size() < 10) {
            qDebug() << "跳过无效行: " << lineNumber;
            continue;
        }

        BaseInfo info;
        // ID不导入，让数据库自动生成
        info.name = fields[1];
        info.age = fields[2].toInt();
        info.temperature = fields[3].toDouble();
        info.bloodpressure = fields[4].toInt();
        info.ECGsignal = fields[5].toInt();
        info.bloodoxygen = fields[6].toInt();
        info.respiratoryrate = fields[7].toInt();
        info.time = fields[8];
        info.checkProjectNumber = fields[9].toInt();

        importData.append(info);
    }

    file.close();

    // 添加确认对话框
    QMessageBox::StandardButton reply = QMessageBox::question(this, "导入确认",
        QString("确定要导入 %1 条生理参数数据吗？").arg(importData.size()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 执行批量导入
        bool success = m_ptruserSql->addData(importData);

        if (success) {
            // 记录日志
            m_ptruserSql->addLogRecord(m_ptruserSql->m_currentUser, "导入生理参数数据",
                                     QString("导入了%1条生理参数数据").arg(importData.size()));

            QMessageBox::information(this, "成功",
                                    QString("成功导入 %1 条生理参数数据").arg(importData.size()));
            updateTable();
        } else {
            QMessageBox::critical(this, "失败", "导入数据时发生错误");
        }
    }
}


void MainWindow::on_btn_exportUser_clicked()
{
// 检查权限 - 只有管理员可以导出用户数据
    if (m_dlgLogin.m_userRole != ROLE_ADMIN) {
        QMessageBox::warning(this, "权限不足", "只有管理员可以导出用户数据");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
            "导出用户数据", QDir::homePath() + "/user_data.csv",
            "CSV文件 (*.csv);;所有文件 (*.*)");

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开文件进行写入");
        return;
    }

    // 添加BOM头解决中文乱码
    file.write("\xEF\xBB\xBF");

    QTextStream stream(&file);

    // 写入表头
    stream << "用户名,密码,角色,用户ID,关联姓名\n";

    // 获取所有用户
    QList<UserInfo> users = m_ptruserSql->getAllUser();

    for (const UserInfo& user : users) {
        // 获取关联的生理参数姓名
        QString relatedName = "";
        if (user.userid > 0) {
            relatedName = m_ptruserSql->getNameById(user.userid);
        }

        // 根据角色获取角色文本
        QString roleText;
        switch (user.role) {
            case ROLE_ADMIN: roleText = "管理员"; break;
            case ROLE_DOCTOR: roleText = "医生"; break;
            case ROLE_NURSE: roleText = "护士"; break;
            case ROLE_PATIENT: roleText = "患者"; break;
            default: roleText = "未知"; break;
        }

        stream << user.username << ","
              << user.password << ","
              << roleText << ","
              << QString::number(user.userid) << ","
              << relatedName << "\n";
    }

    file.close();

    // 记录日志
    m_ptruserSql->addLogRecord(m_ptruserSql->m_currentUser, "导出用户数据",
                              QString("导出了%1条用户数据").arg(users.size()));

    QMessageBox::information(this, "成功", QString("成功导出 %1 条用户数据").arg(users.size()));
}


void MainWindow::on_btn_importUser_clicked()
{
// 检查权限 - 只有管理员可以导入用户数据
    if (m_dlgLogin.m_userRole != ROLE_ADMIN) {
        QMessageBox::warning(this, "权限不足", "只有管理员可以导入用户数据");
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this,
            "导入用户数据", QDir::homePath(),
            "CSV文件 (*.csv);;所有文件 (*.*)");

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开文件进行读取");
        return;
    }

    QByteArray data = file.readAll();
    int startPos = 0;

    // 检查BOM
    if (data.startsWith("\xEF\xBB\xBF"))
        startPos = 3;

    QString content = QString::fromUtf8(data.mid(startPos));
    QTextStream in(&content);

    // 读取表头
    QString header = in.readLine();

    QList<UserInfo> importUsers;
    int lineNumber = 1;

    // 逐行读取数据
    while (!in.atEnd()) {
        lineNumber++;
        QString line = in.readLine();
        QStringList fields = line.split(",");

        if (fields.size() < 4) {
            qDebug() << "跳过无效行: " << lineNumber;
            continue;
        }

        UserInfo user;
        user.username = fields[0];
        user.password = fields[1]; // 注意：这将是已哈希的密码

        // 根据角色文本设置角色
        QString roleText = fields[2];
        if (roleText == "管理员")
            user.role = ROLE_ADMIN;
        else if (roleText == "医生")
            user.role = ROLE_DOCTOR;
        else if (roleText == "护士")
            user.role = ROLE_NURSE;
        else
            user.role = ROLE_PATIENT;

        user.userid = fields[3].toInt();

        importUsers.append(user);
    }

    file.close();

    // 添加确认对话框
    QMessageBox::StandardButton reply = QMessageBox::question(this, "导入确认",
        QString("确定要导入 %1 条用户数据吗？").arg(importUsers.size()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 执行批量导入
        int successCount = 0;
        for (const UserInfo& user : importUsers) {
            // 检查用户名是否已存在
            if (m_ptruserSql->isExist(user.username)) {
                // 更新现有用户
                if (m_ptruserSql->updateUser(user)) {
                    successCount++;
                }
            } else {
                // 添加新用户
                if (m_ptruserSql->AddUser(user)) {
                    successCount++;
                }
            }
        }

        // 记录日志
        m_ptruserSql->addLogRecord(m_ptruserSql->m_currentUser, "导入用户数据",
                                  QString("导入了%1条用户数据").arg(successCount));

        QMessageBox::information(this, "导入结果",
                                QString("共导入 %1 条用户数据，成功 %2 条")
                                .arg(importUsers.size())
                                .arg(successCount));
        updateTable();
    }
}

// 转到首页
void MainWindow::on_btn_FirstPage_clicked()
{
    if (m_currentPage != 1) {
        m_currentPage = 1;
        if (m_isSearchMode) {
            updateTableWithSearch();
        } else {
            updateTable();
        }
    }
}

// 转到上一页
void MainWindow::on_btn_PrevPage_clicked()
{
    if (m_currentPage > 1) {
        m_currentPage--;
        if (m_isSearchMode) {
            updateTableWithSearch();
        } else {
            updateTable();
        }
    }
}

// 转到下一页
void MainWindow::on_btn_NextPage_clicked()
{
    int totalPages;

    if (m_isSearchMode) {
        // 计算搜索结果的总页数
        // 这里需要重新计算，因为没有存储搜索结果的总记录数
        auto cnt = m_ptruserSql->getDataCnt();
        QList<BaseInfo> allData = m_ptruserSql->getPageData(1, cnt);

        int filteredCount = 0;
        for(const auto& info : allData) {
            if(info.name.contains(m_currentSearchText)) {
                filteredCount++;
            }
        }

        totalPages = (filteredCount + m_pageSize - 1) / m_pageSize;
    } else {
        // 正常模式的总页数
        quint32 totalRecords = m_ptruserSql->getDataCnt();
        totalPages = (totalRecords + m_pageSize - 1) / m_pageSize;
    }

    if (m_currentPage < totalPages) {
        m_currentPage++;
        if (m_isSearchMode) {
            updateTableWithSearch();
        } else {
            updateTable();
        }
    }
}

// 转到末页
void MainWindow::on_btn_LastPage_clicked()
{
    int totalPages;

    if (m_isSearchMode) {
        // 计算搜索结果的总页数
        auto cnt = m_ptruserSql->getDataCnt();
        QList<BaseInfo> allData = m_ptruserSql->getPageData(1, cnt);

        int filteredCount = 0;
        for(const auto& info : allData) {
            if(info.name.contains(m_currentSearchText)) {
                filteredCount++;
            }
        }

        totalPages = (filteredCount + m_pageSize - 1) / m_pageSize;
    } else {
        // 正常模式的总页数
        quint32 totalRecords = m_ptruserSql->getDataCnt();
        totalPages = (totalRecords + m_pageSize - 1) / m_pageSize;
    }

    if (m_currentPage != totalPages) {
        m_currentPage = totalPages;
        if (m_isSearchMode) {
            updateTableWithSearch();
        } else {
            updateTable();
        }
    }
}

// 更新分页控件状态
void MainWindow::updatePageControls()
{
    quint32 totalRecords = m_ptruserSql->getDataCnt();
    int totalPages = (totalRecords + m_pageSize - 1) / m_pageSize;

    // 确保页码合法
    if (m_currentPage < 1)
        m_currentPage = 1;
    if (totalPages > 0 && m_currentPage > totalPages)
        m_currentPage = totalPages;

    // 更新页码信息
    if (ui->lbl_pageInfo) {
        ui->lbl_pageInfo->setText(QString("第 %1 页 / 共 %2 页").arg(m_currentPage).arg(totalPages));
    }

    // 启用/禁用分页按钮
    if (ui->btn_FirstPage) ui->btn_FirstPage->setEnabled(m_currentPage > 1);
    if (ui->btn_PrevPage) ui->btn_PrevPage->setEnabled(m_currentPage > 1);
    if (ui->btn_NextPage) ui->btn_NextPage->setEnabled(m_currentPage < totalPages);
    if (ui->btn_LastPage) ui->btn_LastPage->setEnabled(m_currentPage < totalPages);
}

void MainWindow::on_btn_changePassword_clicked()
{
    // 创建密码修改对话框
    ChangePasswordDialog dialog(this);

    // 显示对话框
    if (dialog.exec() == QDialog::Accepted) {
        QString oldPassword = dialog.getOldPassword();
        QString newPassword = dialog.getNewPassword();
        QString confirmPassword = dialog.getConfirmPassword();

        // 验证输入
        if (newPassword.isEmpty()) {
            QMessageBox::warning(this, "错误", "新密码不能为空！");
            return;
        }

        if (newPassword != confirmPassword) {
            QMessageBox::warning(this, "错误", "两次输入的新密码不一致！");
            return;
        }

        // 检查当前密码是否正确
        QString currentUsername = m_ptruserSql->m_currentUser;
        QList<UserInfo> users = m_ptruserSql->getAllUser();
        UserInfo currentUser;
        bool userFound = false;

        for (const UserInfo &user : users) {
            if (user.username == currentUsername) {
                currentUser = user;
                userFound = true;
                break;
            }
        }

        if (!userFound) {
            QMessageBox::warning(this, "错误", "无法获取当前用户信息！");
            return;
        }

        // 验证旧密码
        if (!m_ptruserSql->verifyPassword(oldPassword, currentUser.password)) {
            QMessageBox::warning(this, "错误", "当前密码输入错误！");
            return;
        }

        // 更新密码 - 修改为使用哈希密码
        currentUser.password = m_ptruserSql->hashPassword(newPassword);
        if (m_ptruserSql->updateUser(currentUser)) {
            // 记录日志
            m_ptruserSql->addLogRecord(currentUsername, "修改密码",
                                    QString("用户 %1 修改了自己的密码").arg(currentUsername));

            QMessageBox::information(this, "成功", "密码修改成功！");
        } else {
            QMessageBox::critical(this, "错误", "密码修改失败，请稍后重试！");
        }
    }
}

// 调整表格行高，使20条数据平均铺满整个表格区域
void MainWindow::adjustTableRowHeightNoEmptyRows()
{
    // 计算表格可见区域的高度
    int tableHeight = ui->tableWidget->height();

    // 表头高度
    int headerHeight = ui->tableWidget->horizontalHeader()->height();

    // 可用于显示行的高度 = 表格高度 - 表头高度
    int availableHeight = tableHeight - headerHeight;

    // 每行高度 = 可用高度 / 20（固定显示20行）
    int rowHeight = availableHeight / 20;

    // 设置最小行高
    if (rowHeight < 25) rowHeight = 25;

    // 应用到所有行
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(rowHeight);

    // 如果当前行数少于20行，预先创建空行填充
    // int currentRows = ui->tableWidget->rowCount();
    // if (currentRows < 20) {
    //     for (int i = currentRows; i < 20; i++) {
    //         ui->tableWidget->insertRow(i);
    //     }
    // }

    qDebug() << "表格高度:" << tableHeight << "表头高度:" << headerHeight
             << "可用高度:" << availableHeight << "行高:" << rowHeight;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->tableWidget && event->type() == QEvent::Resize) {
        adjustTableRowHeightNoEmptyRows();
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::updateTableWithSearch()
{
    if (m_currentSearchText.isEmpty()) {
        updateTable();
        return;
    }

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    // 设置列
    ui->tableWidget->setColumnCount(10);
    QStringList headers = {"ID", "姓名", "年龄", "体温(℃)", "血压(mmHg)",
                         "心电信号(BPM)", "血氧(%)", "呼吸率(BPM)", "检查时间", "检查项目编号"};
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    // 获取全部符合条件的数据
    auto cnt = m_ptruserSql->getDataCnt();
    QList<BaseInfo> allData = m_ptruserSql->getPageData(1, cnt);

    // 筛选符合条件的数据
    QList<BaseInfo> filteredData;
    for(const auto& info : allData) {
        if(info.name.contains(m_currentSearchText)) {
            filteredData.append(info);
        }
    }

    // 计算总页数并更新分页信息
    int totalFilteredRecords = filteredData.size();
    int totalPages = (totalFilteredRecords + m_pageSize - 1) / m_pageSize;

    // 确保当前页码有效
    if (m_currentPage < 1) m_currentPage = 1;
    if (totalPages > 0 && m_currentPage > totalPages) m_currentPage = totalPages;

    // 计算本页数据的起始索引和结束索引
    int startIndex = (m_currentPage - 1) * m_pageSize;
    int endIndex = qMin(startIndex + m_pageSize, totalFilteredRecords);

    // 添加本页数据到表格
    for (int i = startIndex; i < endIndex; ++i) {
        const BaseInfo& info = filteredData[i];
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(info.id)));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(info.name));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(info.age)));
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(info.temperature)));
        ui->tableWidget->setItem(row, 4, new QTableWidgetItem(QString::number(info.bloodpressure)));
        ui->tableWidget->setItem(row, 5, new QTableWidgetItem(QString::number(info.ECGsignal)));
        ui->tableWidget->setItem(row, 6, new QTableWidgetItem(QString::number(info.bloodoxygen)));
        ui->tableWidget->setItem(row, 7, new QTableWidgetItem(QString::number(info.respiratoryrate)));
        ui->tableWidget->setItem(row, 8, new QTableWidgetItem(info.time));
        ui->tableWidget->setItem(row, 9, new QTableWidgetItem(QString::number(info.checkProjectNumber)));
    }

    // 更新显示的记录数
    ui->lb_cnt->setText(QString("数量:%1").arg(totalFilteredRecords));

    // 更新分页控件
    ui->lbl_pageInfo->setText(QString("第 %1 页 / 共 %2 页").arg(m_currentPage).arg(totalPages));
    ui->btn_FirstPage->setEnabled(m_currentPage > 1);
    ui->btn_PrevPage->setEnabled(m_currentPage > 1);
    ui->btn_NextPage->setEnabled(m_currentPage < totalPages);
    ui->btn_LastPage->setEnabled(m_currentPage < totalPages);

    // 调整行高但不添加空白行
    adjustTableRowHeightNoEmptyRows();
}
