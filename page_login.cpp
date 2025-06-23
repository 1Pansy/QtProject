#include "page_login.h"
#include "ui_page_login.h"
#include "usersql.h"
#include <QMessageBox>
#include <QPixmap>
#include <QDebug>

page_login::page_login(QWidget *parent)
    : QWidget(parent)
    , m_userRole(ROLE_PATIENT)
    , ui(new Ui::page_login)

{
    ui->setupUi(this);
    this->setWindowTitle("人体生理参数检测仪管理系统 - 登录");

// 为测试方便，在这里设置默认用户名和密码
#ifdef QT_DEBUG
    ui->le_username->setText("admin");
    ui->le_password->setText("666666");
#endif
}

page_login::~page_login()
{
    delete ui;
}

void page_login::on_btn_login_clicked()
{
    // 获取输入的用户名和密码
    QString strname = ui->le_username->text();
    QString strpassword = ui->le_password->text();

    // 输入验证
    if (strname.isEmpty() || strpassword.isEmpty()) {
        QMessageBox::warning(this, "警告", "用户名和密码不能为空！");
        return;
    }

    // 数据库查找用户名和密码
    userSql *sql = userSql::getinstance();
    QList<UserInfo> l = sql->getAllUser();

    int flag = 0;
    for (int i = 0; i < l.size(); ++i) {
        if (l[i].username == strname && sql->verifyPassword(strpassword, l[i].password)) {
            // 设置用户角色
            m_userRole = l[i].role;

            // 兼容性设置
            if (m_userRole == ROLE_ADMIN) {
                m_checkIdentity = true;
            } else {
                m_checkIdentity = false;
            }

            // 保存用户名
            m_username = l[i].username;

            // 添加切换用户日志（如果不是首次登录）
            QString previousUser = sql->m_currentUser;
            if (!previousUser.isEmpty() && previousUser != m_username) {
                sql->addLogRecord(m_username, "切换用户",
                        QString("从用户 %1 切换到 %2").arg(previousUser).arg(m_username));
            }

            // 设置当前登录用户名
            sql->setCurrentUser(m_username);

            // 添加登录日志
            sql->addLogRecord(m_username, "登录", "用户登录系统");

            // 成功进入主界面
            flag = 1;
            emit sendLoginSuccess();
            this->hide();
            break;
        }
    }

    if (flag == 0) {
        // 失败提示
        QMessageBox::warning(this, "登录失败", "用户名或密码错误！");

        // 清空密码框，方便重新输入
        ui->le_password->clear();
        ui->le_password->setFocus();
    }
}

void page_login::on_btn_exit_clicked()
{
    exit(0);
}

void page_login::on_le_username_textChanged(const QString &arg1)
{
    // 清空密码输入框
    ui->le_password->clear();
}

// 添加到现有函数之后
void page_login::clearInputs()
{
    if (ui) {
        ui->le_username->clear();
        ui->le_password->clear();
        ui->le_username->setFocus();
    }
}
