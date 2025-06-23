#ifndef PAGE_LOGIN_H
#define PAGE_LOGIN_H

#include <QWidget>
#include "usersql.h"


namespace Ui {
class page_login;
}

class page_login : public QWidget
{
    Q_OBJECT

public:
    explicit page_login(QWidget *parent = nullptr);
    ~page_login();
    bool m_checkIdentity;   // 保留兼容性
    QString m_username;
    UserRole m_userRole;       // 用户角色
    QString m_patientId;       // 患者ID（对于普通用户）

    // 清除输入框
    void clearInputs();

private slots:
    void on_btn_login_clicked();

    void on_btn_exit_clicked();

    void on_le_username_textChanged(const QString &arg1);

signals:
    void sendLoginSuccess();

private:
    Ui::page_login *ui;
};

#endif // PAGE_LOGIN_H
