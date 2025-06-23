#ifndef DLG_USER_H
#define DLG_USER_H

#include <QDialog>
#include <QComboBox>
#include "usersql.h"

namespace Ui {
class dlg_user;
}

class dlg_user : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_user(QWidget *parent = nullptr);

    ~dlg_user();

    void setType(bool isAdd, UserInfo info = {});

private slots:
    void on_btn_save_clicked();

    void on_btn_cancel_clicked();

private:
    bool validateInputs();

    void setupRoleComboBox();

private:
    Ui::dlg_user *ui;
    bool m_isAdd;
    UserInfo m_info;
    QComboBox *m_cbRole;
    QLineEdit *m_useridField = nullptr;
};

#endif // DLG_USER_H
