#include "change_password_dialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

ChangePasswordDialog::ChangePasswordDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("修改密码");
    setFixedSize(320, 200);

    // 创建表单布局
    QFormLayout *formLayout = new QFormLayout;

    // 创建输入框和标签
    m_oldPasswordEdit = new QLineEdit(this);
    m_newPasswordEdit = new QLineEdit(this);
    m_confirmPasswordEdit = new QLineEdit(this);

    // 设置密码模式
    m_oldPasswordEdit->setEchoMode(QLineEdit::Password);
    m_newPasswordEdit->setEchoMode(QLineEdit::Password);
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);

    // 添加到表单
    formLayout->addRow("当前密码:", m_oldPasswordEdit);
    formLayout->addRow("新密码:", m_newPasswordEdit);
    formLayout->addRow("确认密码:", m_confirmPasswordEdit);

    // 创建按钮
    m_okButton = new QPushButton("确定", this);
    m_cancelButton = new QPushButton("取消", this);

    // 创建按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    // 连接信号和槽
    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

ChangePasswordDialog::~ChangePasswordDialog()
{
}

QString ChangePasswordDialog::getOldPassword() const
{
    return m_oldPasswordEdit->text();
}

QString ChangePasswordDialog::getNewPassword() const
{
    return m_newPasswordEdit->text();
}

QString ChangePasswordDialog::getConfirmPassword() const
{
    return m_confirmPasswordEdit->text();
}
