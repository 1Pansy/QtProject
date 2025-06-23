#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include "page_login.h"
#include "usersql.h"
#include "dlgdata.h"
#include "dlg_user.h"
#include "charts.h"
#include "log_viewer.h"
#include "change_password_dialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    virtual void keyPressEvent(QKeyEvent *event);
    int m_page = 0;

protected:
    // 添加事件过滤器声明
    bool eventFilter(QObject *watched, QEvent *event) override;


private slots:
    void on_btn_exit_clicked();

    void on_btn_simulation_clicked();

    void on_btn_add_clicked();

    void on_btn_clear_clicked();

    void on_btn_del_clicked();

    void on_btn_update_clicked();

    void on_btn_search_clicked();

    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

    void on_btn_add_2_clicked();

    void on_btn_update_2_clicked();

    void on_btn_del_2_clicked();

    void on_btn_search_2_clicked();

    void on_btn_chart_clicked();

    void on_pushButton_clicked();

    void on_btn_viewLogs_clicked();

    void on_btn_batchGenerate_clicked();  // 批量生成数据

    void on_actionSwitchUser_triggered();

    void on_btn_swuser_clicked();

    void on_btn_exportData_clicked();

    void on_btn_importData_clicked();

    void on_btn_exportUser_clicked();

    void on_btn_importUser_clicked();

    // 数据统计对话框
    void showStatisticsDialog();

    // 分页控制槽函数
    void on_btn_FirstPage_clicked();
    void on_btn_PrevPage_clicked();
    void on_btn_NextPage_clicked();
    void on_btn_LastPage_clicked();

    void on_btn_changePassword_clicked();

private:
    void updateTable();

    void setupUserAccess(); // 角色权限

    void adjustTableRowHeightNoEmptyRows();

    void updateTableWithSearch();

private:
    Ui::MainWindow *ui;
    page_login m_dlgLogin;
    userSql *m_ptruserSql;
    QStringList m_lNames;
    Dlg_Add m_dlgAddData;
    dlg_user m_dlgAddUser;
    charts m_viewCharts;
    LogViewer m_logViewer;

    // 分页相关
    void updatePageControls();
    // 当前页码
    int m_currentPage = 1;
    // 每页显示记录数
    int m_pageSize = 20;

    // 是否处于搜索模式
    bool m_isSearchMode = false;
    // 当前搜索文本
    QString m_currentSearchText;

};
#endif // MAINWINDOW_H
