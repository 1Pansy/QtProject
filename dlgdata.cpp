#include "dlgdata.h"
#include "ui_dlgdata.h"
#include "usersql.h"
#include <QMessageBox>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QDateTime>

Dlg_Add::Dlg_Add(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dlg_Add)
    , m_patientId("")
{
    ui->setupUi(this);

    // 设置验证器
    setupValidators();

    ui->sb_age->setRange(1, 120);
    ui->sb_age->setValue(18); // 默认值

    // 添加日期选择控件
    QDateEdit *dateEdit = new QDateEdit(this);
    dateEdit->setCalendarPopup(true);
    dateEdit->setDate(QDate::currentDate());
    dateEdit->setDisplayFormat("yyyy-MM-dd");
    ui->gridLayout_2->addWidget(dateEdit, 8, 2);
    dateEdit->setObjectName("dateEdit");

    // 连接日期变更信号
    connect(dateEdit, &QDateEdit::dateChanged, this, &Dlg_Add::on_dateEdit_dateChanged);

    // 设置当前日期
    ui->le_time->setText(QDate::currentDate().toString("yyyy-MM-dd"));
    ui->le_time->setReadOnly(true);
}

Dlg_Add::~Dlg_Add()
{
    delete ui;
}

void Dlg_Add::setupValidators()
{
    // 体温验证器 (35.0-42.0)
    QDoubleValidator *tempValidator = new QDoubleValidator(35.0, 42.0, 1, this);
    tempValidator->setNotation(QDoubleValidator::StandardNotation);
    ui->le_temperature->setValidator(tempValidator);
    ui->le_temperature->setPlaceholderText("35.0-42.0");

    // 血压验证器 (40-211)
    QIntValidator *bpValidator = new QIntValidator(40, 211, this);
    ui->le_bloodpressure->setValidator(bpValidator);
    ui->le_bloodpressure->setPlaceholderText("40-211");

    // 心电信号验证器 (30-281)
    QIntValidator *ecgValidator = new QIntValidator(30, 281, this);
    ui->le_ECGSignal->setValidator(ecgValidator);
    ui->le_ECGSignal->setPlaceholderText("30-281");

    // 血氧验证器 (80-100)
    QIntValidator *oxygenValidator = new QIntValidator(80, 100, this);
    ui->le_bloodoxygen->setValidator(oxygenValidator);
    ui->le_bloodoxygen->setPlaceholderText("80-100");

    // 呼吸率验证器 (20-60)
    QIntValidator *respValidator = new QIntValidator(20, 60, this);
    ui->le_respiratoryRate->setValidator(respValidator);
    ui->le_respiratoryRate->setPlaceholderText("20-60");

    // 检查项目编号验证器 (1-1000)
    QIntValidator *projValidator = new QIntValidator(1, 1000, this);
    ui->le_checkProjectNumber->setValidator(projValidator);
    ui->le_checkProjectNumber->setPlaceholderText("1-1000");

    // 设置年龄范围 (1-120)
    ui->sb_age->setRange(1, 120);
}

void Dlg_Add::on_dateEdit_dateChanged(const QDate &date)
{
    ui->le_time->setText(date.toString("yyyy-MM-dd"));
}

void Dlg_Add::setPatientId(const QString& patientId)
{
    m_patientId = patientId;
}

bool Dlg_Add::validateInputs()
{
    // 检查姓名是否为空
    if (ui->le_name->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "姓名不能为空！");
        ui->le_name->setFocus();
        return false;
    }

    // 检查必填字段是否为空及数值范围
    if (ui->le_temperature->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "体温不能为空！");
        ui->le_temperature->setFocus();
        return false;
    } else {
        // 检查体温范围 (35.0-42.0)
        double temp = ui->le_temperature->text().toDouble();
        if (temp < 35.0 || temp > 42.0) {
            QMessageBox::warning(this, "输入错误", "体温必须在35.0-42.0℃范围内！");
            ui->le_temperature->setFocus();
            return false;
        }
    }

    if (ui->le_bloodpressure->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "血压不能为空！");
        ui->le_bloodpressure->setFocus();
        return false;
    } else {
        // 检查血压范围 (40-211)
        int bp = ui->le_bloodpressure->text().toInt();
        if (bp < 40 || bp > 211) {
            QMessageBox::warning(this, "输入错误", "血压必须在40-211mmHg范围内！");
            ui->le_bloodpressure->setFocus();
            return false;
        }
    }

    if (ui->le_ECGSignal->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "心电信号不能为空！");
        ui->le_ECGSignal->setFocus();
        return false;
    } else {
        // 检查心电信号范围 (30-281)
        int ecg = ui->le_ECGSignal->text().toInt();
        if (ecg < 30 || ecg > 281) {
            QMessageBox::warning(this, "输入错误", "心电信号必须在30-281BPM范围内！");
            ui->le_ECGSignal->setFocus();
            return false;
        }
    }

    if (ui->le_bloodoxygen->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "血氧不能为空！");
        ui->le_bloodoxygen->setFocus();
        return false;
    } else {
        // 检查血氧范围 (80-100)
        int oxygen = ui->le_bloodoxygen->text().toInt();
        if (oxygen < 80 || oxygen > 100) {
            QMessageBox::warning(this, "输入错误", "血氧必须在80-100%范围内！");
            ui->le_bloodoxygen->setFocus();
            return false;
        }
    }

    if (ui->le_respiratoryRate->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "呼吸率不能为空！");
        ui->le_respiratoryRate->setFocus();
        return false;
    } else {
        // 检查呼吸率范围 (20-60)
        int resp = ui->le_respiratoryRate->text().toInt();
        if (resp < 20 || resp > 60) {
            QMessageBox::warning(this, "输入错误", "呼吸率必须在20-60BPM范围内！");
            ui->le_respiratoryRate->setFocus();
            return false;
        }
    }

    if (ui->le_time->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "检查时间不能为空！");
        ui->le_time->setFocus();
        return false;
    }

    if (ui->le_checkProjectNumber->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "检查项目编号不能为空！");
        ui->le_checkProjectNumber->setFocus();
        return false;
    } else {
        // 检查检查项目编号范围 (1-1000)
        int proj = ui->le_checkProjectNumber->text().toInt();
        if (proj < 1 || proj > 1000) {
            QMessageBox::warning(this, "输入错误", "检查项目编号必须在1-1000范围内！");
            ui->le_checkProjectNumber->setFocus();
            return false;
        }
    }

    // 检查年龄范围 (1-120)
    int age = ui->sb_age->value();
    if (age < 1 || age > 120) {
        QMessageBox::warning(this, "输入错误", "年龄必须在1-120岁范围内！");
        ui->sb_age->setFocus();
        return false;
    }

    return true;
}

void Dlg_Add::setType(bool isAdd, BaseInfo info)
{
    m_isAdd = isAdd;
    m_info = info;
    m_id = info.id;

    if (isAdd) {
        this->setWindowTitle("添加数据");
        ui->sb_age->setValue(18);
        ui->le_name->clear();
        ui->le_name->setReadOnly(false); // 添加模式下姓名可编辑
        ui->le_temperature->clear();
        ui->le_ECGSignal->clear();
        ui->le_bloodpressure->clear();
        ui->le_bloodoxygen->clear();
        ui->le_respiratoryRate->clear();
        ui->le_time->setText(QDate::currentDate().toString("yyyy-MM-dd"));
        ui->le_checkProjectNumber->clear();
    } else {
        this->setWindowTitle("修改数据");
        ui->le_name->setText(info.name);
        ui->le_name->setReadOnly(true); // 修改模式下姓名只读
        ui->sb_age->setValue(info.age);
        ui->le_temperature->setText(QString::number(info.temperature, 'f', 1));
        ui->le_ECGSignal->setText(QString::number(info.ECGsignal));
        ui->le_bloodpressure->setText(QString::number(info.bloodpressure));
        ui->le_bloodoxygen->setText(QString::number(info.bloodoxygen));
        ui->le_respiratoryRate->setText(QString::number(info.respiratoryrate));
        ui->le_time->setText(info.time);
        ui->le_checkProjectNumber->setText(QString::number(info.checkProjectNumber));

        // 设置日期控件
        QDateEdit *dateEdit = findChild<QDateEdit*>("dateEdit");
        if (dateEdit) {
            dateEdit->setDate(QDate::fromString(info.time, "yyyy-MM-dd"));
        }
    }
}

void Dlg_Add::on_btn_save_clicked()
{
    // 验证输入
    if (!validateInputs()) {
        return;
    }

    // 确认保存
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认",
        "确认保存数据吗？", QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {
        return;
    }

    BaseInfo info;
    auto ptr = userSql::getinstance();

    // 修改数据时保留原姓名
    if (!m_isAdd) {
        info.name = m_info.name; // 使用原姓名，而不是从输入框获取
    } else {
        info.name = ui->le_name->text(); // 新增时才从输入框获取姓名
    }

    info.age = ui->sb_age->text().toInt();
    info.temperature = ui->le_temperature->text().toDouble();
    info.ECGsignal = ui->le_ECGSignal->text().toInt();
    info.bloodpressure = ui->le_bloodpressure->text().toInt();
    info.bloodoxygen = ui->le_bloodoxygen->text().toInt();
    info.respiratoryrate = ui->le_respiratoryRate->text().toInt();
    info.time = ui->le_time->text();
    info.checkProjectNumber = ui->le_checkProjectNumber->text().toInt();
    bool success = false;

    if (m_isAdd) {
        success = ptr->addData(info);
        if (success) {
            ptr->addLogRecord(ptr->m_currentUser, "添加数据", "添加了患者 " + info.name + " 的数据");
        }
    } else {
        info.id = m_id;
        success = ptr->UpdateDataInfo(info);
        if (success) {
            ptr->addLogRecord(ptr->m_currentUser, "修改数据", "修改了 ID:" + QString::number(m_id) + " 的数据");
        }
    }

    if (success) {
        QMessageBox::information(this, "成功", "数据保存成功");
        this->accept();
    } else {
        QMessageBox::critical(this, "错误", "数据保存失败");
    }
}


void Dlg_Add::on_btn_cancel_clicked()
{
    this->reject();
}
