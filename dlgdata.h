#ifndef DLGDATA_H
#define DLGDATA_H

#include <QDialog>
#include <QValidator>
#include <QDateEdit>
#include "usersql.h"

namespace Ui {
class Dlg_Add;
}

class Dlg_Add : public QDialog
{
    Q_OBJECT

public:
    explicit Dlg_Add(QWidget *parent = nullptr);
    ~Dlg_Add();

    void setType(bool isAdd, BaseInfo info = {});

    void setPatientId(const QString& patientId);

private slots:
    void on_btn_save_clicked();

    void on_btn_cancel_clicked();

    void on_dateEdit_dateChanged(const QDate &date);

private:
    void setupValidators();

    bool validateInputs();

private:
    Ui::Dlg_Add *ui;
    bool m_isAdd;
    int m_id;
    BaseInfo m_info;
    QString m_patientId;
};

#endif // DLGDATA_H
