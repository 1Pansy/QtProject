#include "dlg_user.h"
#include "ui_dlg_user.h"

#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>

dlg_user::dlg_user(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::dlg_user)
{
    ui->setupUi(this);

    // 设置窗口固定大小和标题
    setFixedSize(350, 250);
    setWindowTitle("用户信息");

    // 创建新的表单布局，替换现有复杂布局
    QWidget* formWidget = new QWidget(this);
    QFormLayout* formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(12);
    formLayout->setContentsMargins(20, 20, 20, 20);

    // 创建和设置字段
    // 用户名
    QLabel* lblUsername = new QLabel("用户名:", this);
    ui->le_username->setMinimumHeight(30);
    ui->le_username->setMaximumHeight(30);
    ui->le_username->setMinimumWidth(200);
    formLayout->addRow(lblUsername, ui->le_username);

    // 密码
    QLabel* lblPassword = new QLabel("密码:", this);
    ui->le_password->setMinimumHeight(30);
    ui->le_password->setMaximumHeight(30);
    ui->le_password->setMinimumWidth(200);
    formLayout->addRow(lblPassword, ui->le_password);

    // 角色
    QLabel* lblRole = new QLabel("角色:", this);
    m_cbRole = new QComboBox(this);
    m_cbRole->setMinimumHeight(30);
    m_cbRole->setMaximumHeight(30);
    m_cbRole->setMinimumWidth(200);
    setupRoleComboBox();
    formLayout->addRow(lblRole, m_cbRole);

    // 删除多余的权限控件
    ui->label_3->hide();
    ui->le_auth->hide();

    // 统一字体
    QFont labelFont("Microsoft YaHei", 10);
    lblUsername->setFont(labelFont);
    lblPassword->setFont(labelFont);
    lblRole->setFont(labelFont);

    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(formWidget);

    // 添加按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    ui->btn_save->setMinimumHeight(32);
    ui->btn_cancel->setMinimumHeight(32);
    buttonLayout->addWidget(ui->btn_save);
    buttonLayout->addWidget(ui->btn_cancel);
    mainLayout->addLayout(buttonLayout);

    // 应用布局
    setLayout(mainLayout);
}

dlg_user::~dlg_user()
{
    delete ui;
}

void dlg_user::setupRoleComboBox()
{
    m_cbRole->clear();
    m_cbRole->addItem("管理员", ROLE_ADMIN);
    m_cbRole->addItem("医生", ROLE_DOCTOR);
    m_cbRole->addItem("护士", ROLE_NURSE);
    // 移除患者选项，患者只能通过生理参数数据自动创建
    //m_cbRole->addItem("患者", ROLE_PATIENT);
}

bool dlg_user::validateInputs()
{
    // 检查用户名是否为空
    if (ui->le_username->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "用户名不能为空！");
        ui->le_username->setFocus();
        return false;
    }

    // 检查密码是否为空
    if (ui->le_password->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "密码不能为空！");
        ui->le_password->setFocus();
        return false;
    }

    // 如果是添加新用户，检查用户名是否已存在
    if (m_isAdd) {
        auto ptr = userSql::getinstance();
        if (ptr->isExist(ui->le_username->text())) {
            QMessageBox::warning(this, "输入错误", "用户名已存在！");
            ui->le_username->setFocus();
            return false;
        }
    }

    return true;
}

void dlg_user::setType(bool isAdd, UserInfo info)
{
    m_isAdd = isAdd;
    m_info = info;

    // 添加userid字段的UI元素但默认隐藏
    if (!m_useridField) {
        QLabel* lblUserid = new QLabel("用户ID:", this);
        lblUserid->setObjectName("lblUserid");
        m_useridField = new QLineEdit(this);
        m_useridField->setMinimumHeight(30);
        m_useridField->setMaximumHeight(30);
        m_useridField->setObjectName("m_useridField");

        // 将字段添加到表单布局中
        QFormLayout* formLayout = qobject_cast<QFormLayout*>(this->layout()->itemAt(0)->widget()->layout());
        if (formLayout) {
            formLayout->addRow(lblUserid, m_useridField);
        }
    }

    // 隐藏userid字段，非患者用户不需要关联ID
    QLabel* lblUserid = findChild<QLabel*>("lblUserid");
    if (lblUserid) lblUserid->setVisible(false);
    m_useridField->setVisible(false);

    if (!isAdd) {
        // 修改模式：预填充所有字段
        ui->le_username->setText(info.username);
        ui->le_username->setReadOnly(true);
        ui->le_username->setStyleSheet("background-color: #F0F0F0;");
        ui->le_password->setText(info.password);
        m_useridField->setText(QString::number(info.userid));

        // 只有在修改患者账号时才显示ID字段，但设为只读
        if (info.role == ROLE_PATIENT) {
            QLabel* lblUserid = findChild<QLabel*>("lblUserid");
            if (lblUserid) lblUserid->setVisible(true);
            m_useridField->setVisible(true);
            m_useridField->setReadOnly(true);
            m_useridField->setEnabled(false);
        }

        this->setWindowTitle("修改用户");

        // 设置角色下拉框
        int roleIndex = 0; // 默认为管理员
        switch (info.role) {
        case ROLE_ADMIN: roleIndex = 0; break;
        case ROLE_DOCTOR: roleIndex = 1; break;
        case ROLE_NURSE: roleIndex = 2; break;
        case ROLE_PATIENT:
            // 添加临时的患者选项供显示使用
            m_cbRole->addItem("患者", ROLE_PATIENT);
            roleIndex = 3;
            break;
        }

        m_cbRole->setCurrentIndex(roleIndex);

        // 如果是患者账号，禁用角色选择
        if (info.role == ROLE_PATIENT) {
            m_cbRole->setEnabled(false);
        } else {
            m_cbRole->setEnabled(true);
        }
    } else {
        // 添加模式：清空所有字段
        ui->le_username->clear();
        ui->le_username->setReadOnly(false);
        ui->le_password->clear();
        m_useridField->clear();
        m_cbRole->setEnabled(true);
        this->setWindowTitle("添加用户");
    }
}

void dlg_user::on_btn_save_clicked()
{
    // 验证输入
    if (!validateInputs()) {
        return;
    }

    // 确认保存
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认",
        "确认保存用户信息吗？", QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {
        return;
    }

    UserInfo info;
    auto ptr = userSql::getinstance();

    // 获取基本信息
    info.username = ui->le_username->text();
    info.password = ui->le_password->text();

    // 直接根据ComboBox的索引设置角色
    switch (m_cbRole->currentIndex()) {
        case 0: info.role = ROLE_ADMIN; break;
        case 1: info.role = ROLE_DOCTOR; break;
        case 2: info.role = ROLE_NURSE; break;
        case 3: info.role = ROLE_PATIENT; break;
        default: info.role = ROLE_PATIENT; break;
    }

    // 根据角色设置权限文本 (为了兼容性)
    switch (info.role) {
        case ROLE_ADMIN: info.aut = "admin"; break;
        case ROLE_DOCTOR: info.aut = "doctor"; break;
        case ROLE_NURSE: info.aut = "nurse"; break;
        case ROLE_PATIENT: info.aut = "user"; break;
    }

    // 只在修改患者账号时保留原userid值，新增用户或修改非患者账号时userid=0
    if (!m_isAdd && m_info.role == ROLE_PATIENT) {
        info.userid = m_info.userid; // 保持原患者ID不变
    } else {
        info.userid = 0; // 非患者用户不关联ID
    }

    bool success = false;

    if (m_isAdd) {
        success = ptr->AddUser(info);
        ptr->addLogRecord(ptr->m_currentUser, "添加用户", "添加了用户 " + info.username);
    } else {
        success = ptr->updateUser(info);
        ptr->addLogRecord(ptr->m_currentUser, "修改用户", "修改了用户 " + info.username + " 的信息");
    }

    if (success) {
        QMessageBox::information(this, "成功", "用户信息保存成功");
        this->accept();
    } else {
        QMessageBox::critical(this, "错误", "用户信息保存失败");
    }
}

void dlg_user::on_btn_cancel_clicked()
{
    this->reject();
}
