#ifndef LOG_VIEWER_H
#define LOG_VIEWER_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDateEdit>
#include <QComboBox>
#include "usersql.h"

class LogViewer : public QDialog
{
    Q_OBJECT

public:
    explicit LogViewer(QWidget *parent = nullptr);
    ~LogViewer();

    // 刷新日志显示
    void refreshLogs();

private slots:
    // 过滤日志
    void onFilterButtonClicked();
    // 导出日志
    void onExportButtonClicked();
    // 清除日志
    void onClearButtonClicked();
    // 导入日志
    void onImportButtonClicked();
    // 用户筛选变化
    void onUserFilterChanged(int index);
    // 操作类型筛选变化
    void onOperationFilterChanged(int index);
    // 日期选择变化时刷新筛选数据
    void onDateChanged(const QDate &date);

private:
    // 设置筛选器
    void setupFilters();

    // 筛选日志
    void filterLogs(const QString& specificUser = "", const QStringList& userList = QStringList());

    // 设置用户下拉框
    void setupUserComboBox();
    void populateUserComboBox();
    QStringList getRoleBasedUsers(UserRole role);

    // 应用当前所有筛选条件
    void applyCurrentFilters();

    // UI组件
    QTableWidget *m_tableWidget;
    QDateEdit *m_startDateEdit;
    QDateEdit *m_endDateEdit;
    QComboBox *m_operationFilter;
    QComboBox *m_userFilter;
    QPushButton *m_filterButton;
    QPushButton *m_exportButton;
    QPushButton *m_importButton;
    QPushButton *m_clearButton;
    QPushButton *m_closeButton;
    QLabel *m_labelCount;

    // 初始化UI
    void setupUI();

    // 加载日志数据
    void loadLogData(const QList<LogRecord> &logs);

    // 添加变量跟踪当前筛选状态
    bool m_isRoleFiltering = false;
    UserRole m_currentRoleFilter = ROLE_PATIENT;
    QStringList m_currentRoleUsers;
};

#endif // LOG_VIEWER_H
