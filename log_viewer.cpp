#include "log_viewer.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QHeaderView>
#include <QDateTime>

LogViewer::LogViewer(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("系统操作日志");
    setMinimumSize(800, 500);

    setupUI();
    setupFilters();
    setupUserComboBox();
    refreshLogs();
}

LogViewer::~LogViewer()
{
    //delete ui;
}

void LogViewer::setupUI()
{
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 创建筛选区域
    QHBoxLayout *filterLayout = new QHBoxLayout();

    // 日期范围
    QLabel *lblStartDate = new QLabel("开始日期:", this);
    m_startDateEdit = new QDateEdit(this);
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setDate(QDate::currentDate().addDays(-30));

    QLabel *lblEndDate = new QLabel("结束日期:", this);
    m_endDateEdit = new QDateEdit(this);
    m_endDateEdit->setCalendarPopup(true);
    m_endDateEdit->setDate(QDate::currentDate());

    // 操作类型筛选
    QLabel *lblOperation = new QLabel("操作类型:", this);
    m_operationFilter = new QComboBox(this);

    // 用户筛选
    QLabel *lblUser = new QLabel("用户:", this);
    m_userFilter = new QComboBox(this);

    // 筛选按钮
    m_filterButton = new QPushButton("筛选", this);

    // 添加到筛选布局
    filterLayout->addWidget(lblStartDate);
    filterLayout->addWidget(m_startDateEdit);
    filterLayout->addWidget(lblEndDate);
    filterLayout->addWidget(m_endDateEdit);
    filterLayout->addWidget(lblOperation);
    filterLayout->addWidget(m_operationFilter);
    filterLayout->addWidget(lblUser);
    filterLayout->addWidget(m_userFilter);
    filterLayout->addWidget(m_filterButton);

    // 创建表格
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(5);
    QStringList headers;
    headers << "ID" << "用户" << "操作" << "详情" << "时间";
    m_tableWidget->setHorizontalHeaderLabels(headers);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->verticalHeader()->setVisible(false);

    // 创建按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_exportButton = new QPushButton("导出日志", this);
    m_importButton = new QPushButton("导入日志", this);
    m_clearButton = new QPushButton("清除日志", this);
    m_closeButton = new QPushButton("关闭", this);

    buttonLayout->addWidget(m_exportButton);
    buttonLayout->addWidget(m_importButton);
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeButton);

    // 添加到主布局
    mainLayout->addLayout(filterLayout);
    mainLayout->addWidget(m_tableWidget);

    // 添加记录计数标签
    m_labelCount = new QLabel("共 0 条记录", this);
    m_labelCount->setAlignment(Qt::AlignRight);
    mainLayout->addWidget(m_labelCount);

    mainLayout->addLayout(buttonLayout);

    // 连接信号和槽
    connect(m_filterButton, &QPushButton::clicked, this, &LogViewer::onFilterButtonClicked);
    connect(m_exportButton, &QPushButton::clicked, this, &LogViewer::onExportButtonClicked);
    connect(m_importButton, &QPushButton::clicked, this, &LogViewer::onImportButtonClicked);
    connect(m_clearButton, &QPushButton::clicked, this, &LogViewer::onClearButtonClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    // 添加日期选择变化的连接
    connect(m_startDateEdit, &QDateEdit::dateChanged, this, &LogViewer::onDateChanged);
    connect(m_endDateEdit, &QDateEdit::dateChanged, this, &LogViewer::onDateChanged);
}

void LogViewer::setupFilters()
{
    // 操作类型筛选选项
    m_operationFilter->addItem("全部");
    m_operationFilter->addItem("登录");

    m_operationFilter->addItem("添加数据");
    m_operationFilter->addItem("修改数据");
    m_operationFilter->addItem("删除数据");
    m_operationFilter->addItem("清空数据");

    m_operationFilter->addItem("添加用户");
    m_operationFilter->addItem("修改用户");
    m_operationFilter->addItem("删除用户");
    m_operationFilter->addItem("切换用户");

    m_operationFilter->addItem("添加数据与用户");
    m_operationFilter->addItem("删除数据和用户");
    m_operationFilter->addItem("批量添加数据与用户");

    m_operationFilter->addItem("查看图表");

    m_operationFilter->addItem("导出日志");
    m_operationFilter->addItem("导入日志");
    m_operationFilter->addItem("清除日志");


    // 连接操作类型筛选信号
    connect(m_operationFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LogViewer::onOperationFilterChanged);

    // 设置用户筛选
    m_userFilter->clear();
    m_userFilter->addItem("全部", "");

    // 获取所有用户
    userSql *sql = userSql::getinstance();
    QList<UserInfo> users = sql->getAllUser();

    for(const UserInfo &user : users) {
        m_userFilter->addItem(user.username, user.username);
    }
}

void LogViewer::refreshLogs()
{
    userSql *sql = userSql::getinstance();
    QList<LogRecord> logs = sql->getLogRecords(1000); // 获取最近1000条日志
    loadLogData(logs);
}

void LogViewer::loadLogData(const QList<LogRecord> &logs)
{
    m_tableWidget->setRowCount(0);
    m_tableWidget->setRowCount(logs.size());

    for(int i = 0; i < logs.size(); ++i) {
        const LogRecord &log = logs[i];

        m_tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(log.id)));
        m_tableWidget->setItem(i, 1, new QTableWidgetItem(log.username));
        m_tableWidget->setItem(i, 2, new QTableWidgetItem(log.operation));
        m_tableWidget->setItem(i, 3, new QTableWidgetItem(log.details));
        m_tableWidget->setItem(i, 4, new QTableWidgetItem(log.timestamp));
    }

    // 自动调整列宽
    m_tableWidget->resizeColumnsToContents();

    //更新记录计数标签
    if (m_labelCount) {
        m_labelCount->setText(QString("共 %1 条记录").arg(logs.size()));
    }
}

void LogViewer::onFilterButtonClicked()
{
    // 直接调用通用筛选方法
    applyCurrentFilters();
}

void LogViewer::onDateChanged(const QDate &date)
{
    // 日期改变时，自动应用筛选
    applyCurrentFilters();
}

void LogViewer::onExportButtonClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("导出日志"), QDir::homePath() + "/logs.csv",
        tr("CSV文件 (*.csv);;文本文件 (*.txt)"));

    if(fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);

    // 使用UTF-8 with BOM编码打开文件，解决中文乱码问题
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开文件进行写入");
        return;
    }

    // 添加UTF-8 BOM
    file.write("\xEF\xBB\xBF");

    QTextStream stream(&file);

    // 写入表头
    stream << "ID,用户,操作,详情,时间\n";

    // 写入数据行
    int rowCount = m_tableWidget->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        QStringList fields;

        // 收集每一列的数据
        for (int col = 0; col < m_tableWidget->columnCount(); ++col) {
            QTableWidgetItem *item = m_tableWidget->item(row, col);
            QString text = item ? item->text() : "";

            // 如果字段中有逗号，将整个字段用引号括起来
            if (text.contains(',') || text.contains('"') || text.contains('\n')) {
                text.replace("\"", "\"\""); // 转义引号
                text = "\"" + text + "\"";
            }

            fields << text;
        }

        // 写入一行数据
        stream << fields.join(",") << "\n";
    }

    file.close();

    // 记录日志导出操作
    userSql *sql = userSql::getinstance();
    sql->addLogRecord(sql->m_currentUser, "导出日志",
            QString("导出了%1条日志记录到文件%2").arg(rowCount).arg(fileName));

    refreshLogs();

    QMessageBox::information(this, "成功", "日志导出成功");
}

// 实现日志导入功能
void LogViewer::onImportButtonClicked()
{
    // 获取导入文件名
    QString fileName = QFileDialog::getOpenFileName(this, "导入日志",
        QDir::homePath(), "CSV文件 (*.csv);;所有文件 (*.*)");

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开文件进行读取");
        return;
    }

    // 检测UTF-8 BOM
    QByteArray data = file.readAll();
    int startPos = 0;
    if (data.startsWith("\xEF\xBB\xBF")) {
        startPos = 3;
    }

    QString content = QString::fromUtf8(data.mid(startPos));
    QTextStream in(&content);

    // 读取表头行
    QString header = in.readLine();
    QStringList headerFields = header.split(",");

    // 验证CSV格式
    if (headerFields.size() < 5 ||
        !header.contains("用户") || !header.contains("操作") ||
        !header.contains("详情") || !header.contains("时间")) {
        QMessageBox::critical(this, "格式错误", "CSV文件格式不正确，请确保包含必要的列");
        return;
    }

    // 准备导入数据
    QList<LogRecord> logsToImport;
    int lineNumber = 1; // 从第2行开始（第1行是表头）
    userSql *sql = userSql::getinstance();

    // 逐行读取数据
    while (!in.atEnd()) {
        lineNumber++;
        QString line = in.readLine();

        // 解析一行，考虑引号内的逗号
        QStringList fields;
        bool inQuotes = false;
        QString field;

        for (int i = 0; i < line.length(); i++) {
            if (line[i] == '\"') {
                inQuotes = !inQuotes;
            } else if (line[i] == ',' && !inQuotes) {
                fields.append(field);
                field.clear();
            } else {
                field.append(line[i]);
            }
        }
        fields.append(field); // 添加最后一个字段

        // 验证字段数量
        if (fields.size() < 4) {
            qDebug() << "跳过不完整的行:" << lineNumber;
            continue;
        }

        // 创建日志记录
        LogRecord log;
        log.username = fields[1];
        log.operation = fields[2];
        log.details = fields[3];

        // 处理时间字段
        QString timestamp = fields.size() > 4 ? fields[4] : QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        log.timestamp = timestamp;

        logsToImport.append(log);
    }

    std::sort(logsToImport.begin(), logsToImport.end(),
        [](const LogRecord &a, const LogRecord &b) {
            // 将时间字符串转换为QDateTime对象进行比较
            QDateTime timeA = QDateTime::fromString(a.timestamp, "yyyy-MM-dd hh:mm:ss");
            QDateTime timeB = QDateTime::fromString(b.timestamp, "yyyy-MM-dd hh:mm:ss");
            return timeA < timeB; // 时间早的排在前面，获得较小的ID
        }
    );

    // 导入到数据库
    bool success = sql->importLogs(logsToImport);

    if (success) {
        // 记录日志导入操作
        sql->addLogRecord(sql->m_currentUser, "导入日志",
                QString("从文件%1导入了%2条日志记录").arg(fileName).arg(logsToImport.size()));

        refreshLogs();

        QMessageBox::information(this, "导入成功",
            QString("成功导入 %1 条日志记录").arg(logsToImport.size()));
    } else {
        QMessageBox::critical(this, "导入失败", "导入日志时发生错误");
    }
}

void LogViewer::onClearButtonClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认",
        "确定要清除所有日志吗？\n此操作不可撤销！",
                QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 获取数据库实例
        userSql *sql = userSql::getinstance();

        // 执行清除操作之前保存当前用户名
        QString currentUser = sql->m_currentUser;

        // 执行清除操作
        bool success = sql->clearLogs();

        if (success) {
            // 记录清除日志操作（需要在数据库清空后单独处理）
            // 因为清空操作会删除所有日志，所以需要特殊处理这个日志记录
            sql->addLogRecord(currentUser, "清除日志", "清除了所有日志记录");

            QMessageBox::information(this, "成功", "所有日志记录已清除");
            // 刷新日志显示
            refreshLogs();
        } else {
            QMessageBox::critical(this, "失败", "清除日志时发生错误");
        }
    }
}

// 日志筛选方法
void LogViewer::filterLogs(const QString& specificUser, const QStringList& userList)
{
    QDate startDate = m_startDateEdit->date();
    QDate endDate = m_endDateEdit->date();

    QString operation = m_operationFilter->currentText();
    if (operation == "全部") operation = "";

    QString username;
    if (!specificUser.isEmpty()) {
        username = specificUser;
    } else if (!userList.isEmpty()) {
        username = "";
    } else {
        username = m_userFilter->currentText();
        if (username == "全部" || username == "管理员" ||
            username == "医生" || username == "护士" ||
            username == "患者") {
            username = "";
        }
    }

    // 获取筛选日志
    userSql *sql = userSql::getinstance();
    QList<LogRecord> logs;

    if (!userList.isEmpty()) {
        logs = sql->getFilteredLogsByUsers(startDate, endDate, operation, userList, 1000);
    } else {
        logs = sql->getFilteredLogs(startDate, endDate, operation, username, 1000);
    }

    // 更新表格显示
    m_tableWidget->clearContents();
    m_tableWidget->setRowCount(0);

    for (int i = 0; i < logs.size(); ++i) {
        const LogRecord &log = logs[i];
        m_tableWidget->insertRow(i);

        m_tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(log.id)));
        m_tableWidget->setItem(i, 1, new QTableWidgetItem(log.username));
        m_tableWidget->setItem(i, 2, new QTableWidgetItem(log.operation));
        m_tableWidget->setItem(i, 3, new QTableWidgetItem(log.details));
        m_tableWidget->setItem(i, 4, new QTableWidgetItem(log.timestamp));
    }

    m_labelCount->setText(QString("共 %1 条记录").arg(logs.size()));
}

// 实现角色筛选
void LogViewer::setupUserComboBox()
{
m_userFilter->clear();
    m_userFilter->addItem("全部", "");
    m_userFilter->insertSeparator(1);

    // 添加角色分类
    m_userFilter->addItem("管理员", "role:admin");
    m_userFilter->addItem("医生", "role:doctor");
    m_userFilter->addItem("护士", "role:nurse");
    m_userFilter->addItem("患者", "role:patient");
    m_userFilter->insertSeparator(6);

    // 添加具体用户
    userSql *sql = userSql::getinstance();
    QList<UserInfo> users = sql->getAllUser();

    for (const auto &user : users) {
        m_userFilter->addItem(user.username, user.username);
    }

    // 连接选择变化信号
    connect(m_userFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &LogViewer::onUserFilterChanged);
}

void LogViewer::populateUserComboBox()
{
    // 获取所有用户
    userSql *sql = userSql::getinstance();
    QList<UserInfo> users = sql->getAllUser();

    for (const auto &user : users) {
        m_userFilter->addItem(user.username);
    }
}

// 处理角色筛选变化
void LogViewer::onUserFilterChanged(int index)
{
    // 添加保护性检查
    if (!m_userFilter || index < 0 || index >= m_userFilter->count()) {
        return;
    }

    // 重置角色筛选状态
    m_isRoleFiltering = false;
    m_currentRoleUsers.clear();

    // 特别处理"全部"选项
    if (index == 0) {  // "全部"选项的索引为0
        applyCurrentFilters();
        return;
    }

    // 验证数据有效性
    if (!m_userFilter->currentData().isValid()) {
        return;
    }

    QString value = m_userFilter->currentData().toString();

    try {
        if (value.startsWith("role:")) {
            QString role = value.mid(5);

            // 保存角色筛选状态
            m_isRoleFiltering = true;

            // 获取该角色的所有用户
            if (role == "admin") {
                m_currentRoleFilter = ROLE_ADMIN;
                m_currentRoleUsers = getRoleBasedUsers(ROLE_ADMIN);
            } else if (role == "doctor") {
                m_currentRoleFilter = ROLE_DOCTOR;
                m_currentRoleUsers = getRoleBasedUsers(ROLE_DOCTOR);
            } else if (role == "nurse") {
                m_currentRoleFilter = ROLE_NURSE;
                m_currentRoleUsers = getRoleBasedUsers(ROLE_NURSE);
            } else if (role == "patient") {
                m_currentRoleFilter = ROLE_PATIENT;
                m_currentRoleUsers = getRoleBasedUsers(ROLE_PATIENT);
            }

            // 应用筛选，同时考虑当前的操作类型
            applyCurrentFilters();
        } else {
            // 普通用户选择，也自动刷新
            applyCurrentFilters();
        }
    } catch (const std::exception& e) {
        qDebug() << "用户筛选时发生异常:" << e.what();
    } catch (...) {
        qDebug() << "用户筛选时发生未知异常";
    }
}

void LogViewer::onOperationFilterChanged(int index)
{
    // 只有当选择了有效选项时才应用筛选
    if (index >= 0) {
        applyCurrentFilters();
    }
}

void LogViewer::applyCurrentFilters()
{
// 获取筛选条件
    QDate startDate = m_startDateEdit->date();
    QDate endDate = m_endDateEdit->date();

    QString operation;
    if (m_operationFilter && m_operationFilter->currentIndex() > 0) { // 不是"全部"选项
        operation = m_operationFilter->currentText();
    } else {
        operation = "";
    }

    // 添加空指针检查
    userSql *sql = userSql::getinstance();
    if (!sql) return;

    // 添加try-catch以防止程序崩溃
    try {
        QList<LogRecord> logs;

        // 根据筛选状态选择不同的筛选方法
        if (m_isRoleFiltering && !m_currentRoleUsers.isEmpty()) {
            // 角色筛选 + 操作类型筛选
            logs = sql->getFilteredLogsByUsers(startDate, endDate, operation, m_currentRoleUsers, 1000);
        } else {
            // 普通用户或全部用户筛选
            QString username;
            if (m_userFilter && m_userFilter->currentIndex() > 0 &&
                !m_userFilter->currentData().toString().startsWith("role:")) {
                username = m_userFilter->currentText();
            } else {
                username = "";
            }
            logs = sql->getFilteredLogs(startDate, endDate, operation, username, 1000);
        }

        loadLogData(logs);

        // 更新统计信息
        if (m_labelCount) {
            m_labelCount->setText(QString("共 %1 条记录").arg(logs.size()));
        }
    } catch (const std::exception& e) {
        qDebug() << "筛选日志时发生异常:" << e.what();
        QMessageBox::critical(this, "错误", "筛选日志失败: " + QString(e.what()));
    } catch (...) {
        qDebug() << "筛选日志时发生未知异常";
        QMessageBox::critical(this, "错误", "筛选日志失败: 未知错误");
    }
}

// 获取指定角色的所有用户名
QStringList LogViewer::getRoleBasedUsers(UserRole role)
{
    QStringList result;
    userSql *sql = userSql::getinstance();
    QList<UserInfo> users = sql->getAllUser();

    for (const auto &user : users) {
        if (user.role == role) {
            result.append(user.username);
        }
    }

    return result;
}
