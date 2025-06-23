#include "usersql.h"
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QtDebug>
#include <QDateTime>
#include <QCryptographicHash>

userSql *userSql::ptruserSql = nullptr;

userSql::userSql(QObject *parent)
    : QObject{parent}
    , m_currentUser("")  // 初始化为空字符串
{

    // BaseInfo b;
    // b.name = "小明";
    // for (int i = 0; i < 10; i++) {
    //     addData(b);
    // }
    // getDataCnt();

    //getPageData(2,3);

    //delData(20);

    //clearDataTable();

    // BaseInfo b;
    // b.name = "小王";
    // b.id = 10;
    // UpdateDataInfo(b);
    // UserInfo info;
    // info.username = "xiaoyu";
    // info.password = "123456";
    // info.aut = "admin";
    // AddUser(info);
    // AddUser(info);
    // auto l = getAllUser();
    // qDebug()<<isExist("xiaoyu");
    // info.password = "666";
    // updateUser(info);
    // delUser("xiaoyu");
}

void userSql::init()
{
    if (QSqlDatabase::drivers().isEmpty()){
        qDebug()<<"No database drivers found";
    }


    m_db = QSqlDatabase::addDatabase("QSQLITE");


    auto str = QCoreApplication::applicationDirPath() + "/data.db";
    qDebug()<<str;


    m_db.setDatabaseName(str);
    if (!m_db.open()) {
        qDebug() << "Database not opened: " << m_db.lastError().text();
        return;
    }

    // 创建数据表并升级数据库结构
    createTables();
    upgradeDatabase();

    // 添加默认用户（如果不存在）
    addDefaultUsers();
}

void userSql::createTables()
{
    QSqlQuery query(m_db);

    // 创建用户表
    query.exec("CREATE TABLE IF NOT EXISTS user ("
               "username VARCHAR(50) PRIMARY KEY, "
               "password VARCHAR(100) NOT NULL, "
               "auth VARCHAR(20) NOT NULL, "
               "role INTEGER DEFAULT 3, "
               "userid INTEGER)");

    // 创建生理参数数据表（移除user_id字段）
    query.exec("CREATE TABLE IF NOT EXISTS 'Human physiological parameter data' ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "name VARCHAR(50) NOT NULL, "
               "age INTEGER CHECK(age BETWEEN 1 AND 120), "
               "temperature DECIMAL(3,1) CHECK(temperature BETWEEN 35.0 AND 42.0), "
               "bloodpressure INTEGER CHECK(bloodpressure BETWEEN 40 AND 211), "
               "ECGsignal INTEGER CHECK(ECGsignal BETWEEN 30 AND 281), "
               "bloodoxygen INTEGER CHECK(bloodoxygen BETWEEN 80 AND 100), "
               "respiratoryrate INTEGER CHECK(respiratoryrate BETWEEN 20 AND 60), "
               "time VARCHAR(20) NOT NULL, "
               "checkProjectNumber INTEGER CHECK(checkProjectNumber BETWEEN 1 AND 1000))");

    // 创建操作日志表（如果不存在）
    query.exec("CREATE TABLE IF NOT EXISTS 'LogRecords' ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "username VARCHAR(50) NOT NULL, "
               "operation VARCHAR(50) NOT NULL, "
               "details TEXT, "
               "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP)");

    // 添加索引以提高查询性能
    query.exec("CREATE INDEX IF NOT EXISTS idx_name ON 'Human physiological parameter data' (name)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_username ON 'LogRecords' (username)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_timestamp ON 'LogRecords' (timestamp)");
}

void userSql::upgradeDatabase()
{
    QSqlQuery query(m_db);

    // 检查用户表中是否有role和userid列
    query.exec("PRAGMA table_info(user)");
    bool hasRoleColumn = false;
    bool hasUserIdColumn = false;

    while (query.next()) {
        QString columnName = query.value(1).toString();
        if (columnName == "role")
            hasRoleColumn = true;
        if (columnName == "userid")
            hasUserIdColumn = true;
    }

    // 添加缺失的列
    m_db.transaction();
    if (!hasRoleColumn) {
        query.exec("ALTER TABLE user ADD COLUMN role INTEGER DEFAULT 3");
        qDebug() << "添加role列:" << (!query.lastError().isValid());
    }

    if (!hasUserIdColumn) {
        query.exec("ALTER TABLE user ADD COLUMN userid INTEGER");
        qDebug() << "添加userid列:" << (!query.lastError().isValid());
    }

    // 提交事务
    m_db.commit();

    // 根据auth字段更新role字段
    query.exec("UPDATE user SET role = 0 WHERE auth = 'admin'");
    query.exec("UPDATE user SET role = 1 WHERE auth = 'doctor'");
    query.exec("UPDATE user SET role = 2 WHERE auth = 'nurse'");
    query.exec("UPDATE user SET role = 3 WHERE auth NOT IN ('admin', 'doctor', 'nurse')");

    // 优化: 添加索引提升查询性能
    query.exec("CREATE INDEX IF NOT EXISTS idx_name ON 'Human physiological parameter data' (name)");

    qDebug() << "数据库结构升级完成";
}

void userSql::addDefaultUsers()
{
    QSqlQuery query(m_db);

    // 检查是否存在admin用户
    query.prepare("SELECT COUNT(*) FROM user WHERE username = ?");
    query.addBindValue("admin");
    query.exec();

    if (query.next() && query.value(0).toInt() == 0) {
        // 添加默认管理员
        UserInfo admin;
        admin.username = "admin";
        admin.password = "admin";
        admin.aut = "admin";
        admin.role = ROLE_ADMIN;
        AddUser(admin);

        // 添加默认医生
        UserInfo doctor;
        doctor.username = "doctor";
        doctor.password = "doctor";
        doctor.aut = "doctor";
        doctor.role = ROLE_DOCTOR;
        AddUser(doctor);

        // 添加默认护士
        UserInfo nurse;
        nurse.username = "nurse";
        nurse.password = "nurse";
        nurse.aut = "nurse";
        nurse.role = ROLE_NURSE;
        AddUser(nurse);

        // 添加默认患者
        UserInfo patient;
        patient.username = "patient";
        patient.password = "patient";
        patient.aut = "user";
        patient.role = ROLE_PATIENT;
        AddUser(patient);
    }
}

QString userSql::hashPassword(const QString &password)
{
    QByteArray passwordBytes = password.toUtf8();
    QByteArray hashedPassword = QCryptographicHash::hash(
                                    passwordBytes, QCryptographicHash::Sha256).toHex();
    return QString(hashedPassword);
}

bool userSql::verifyPassword(const QString &inputPassword, const QString &storedHash)
{
    // 检查是否是明文密码（兼容旧数据）
    if (inputPassword == storedHash) {
        return true;
    }

    // 检查哈希值
    QByteArray inputBytes = inputPassword.toUtf8();
    QByteArray inputHashed = QCryptographicHash::hash(
            inputBytes, QCryptographicHash::Sha256).toHex();

    return (QString(inputHashed) == storedHash);
}

quint32 userSql::getDataCnt()
{
    QSqlQuery query(m_db);
    query.exec("SELECT COUNT(*) FROM 'Human physiological parameter data'");

    if (query.next()) {
        return query.value(0).toUInt();
    }

    return 0;
}

quint32 userSql::getDataCnt(const QString& patientId)
{
    QSqlQuery sql(m_db);
    sql.prepare("SELECT COUNT(id) FROM 'Human physiological parameter data' WHERE patientId = ?");
    sql.addBindValue(patientId);
    sql.exec();

    quint32 uiCnt = 0;
    while (sql.next()) {
        uiCnt = sql.value(0).toUInt();
    }
    return uiCnt;
}

BaseInfo userSql::getDataById(int id)
{
    BaseInfo info;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM BaseInfo WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        info.id = query.value("id").toInt();
        info.name = query.value("name").toString();
        info.age = query.value("age").toInt();
        info.temperature = query.value("temperature").toDouble();
        info.bloodpressure = query.value("bloodpressure").toInt();
        info.ECGsignal = query.value("ECGsignal").toInt();
        info.bloodoxygen = query.value("bloodoxygen").toInt();
        info.respiratoryrate = query.value("respiratoryrate").toInt();
        info.time = query.value("time").toString();
        info.checkProjectNumber = query.value("checkProjectNumber").toInt();
    } else {
        qDebug() << "Error getting data by id:" << query.lastError().text();
    }

    return info;
}

QList<BaseInfo> userSql::getPageData(quint32 page, quint32 uiCnt)
{
    QList<BaseInfo> l;
    QSqlQuery sql(m_db);
    QString strsql = QString("SELECT * FROM 'Human physiological parameter data' ORDER BY id LIMIT %1 OFFSET %2").
                     arg(uiCnt).arg((page-1)*uiCnt);
    sql.exec(strsql);

    BaseInfo info;
    while (sql.next()) {
        info.id = sql.value(0).toUInt();
        info.name = sql.value(1).toString();
        info.age = sql.value(2).toUInt();
        info.temperature = sql.value(3).toDouble();
        info.bloodpressure = sql.value(4).toUInt();
        info.ECGsignal = sql.value(5).toUInt();
        info.bloodoxygen = sql.value(6).toUInt();
        info.respiratoryrate = sql.value(7).toUInt();
        info.time = sql.value(8).toString();
        info.checkProjectNumber = sql.value(9).toUInt();

        l.push_back(info);
    }
    return l;
}

//  单条数据添加方法
bool userSql::addData(BaseInfo info)
{
    QSqlQuery sql(m_db);

    // 使用事务提高性能
    m_db.transaction();

    // 首先添加生理参数数据
    QString strSql = "INSERT INTO 'Human physiological parameter data' ("
                     "name, age, temperature, bloodpressure, ECGsignal, "
                     "bloodoxygen, respiratoryrate, time, checkProjectNumber) "
                     "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)";

    sql.prepare(strSql);
    sql.bindValue(0, info.name);
    sql.bindValue(1, info.age);
    sql.bindValue(2, info.temperature);
    sql.bindValue(3, info.bloodpressure);
    sql.bindValue(4, info.ECGsignal);
    sql.bindValue(5, info.bloodoxygen);
    sql.bindValue(6, info.respiratoryrate);
    sql.bindValue(7, info.time);
    sql.bindValue(8, info.checkProjectNumber);

    bool success = sql.exec();

    if (!success) {
        qDebug() << "添加生理参数数据失败:" << sql.lastError().text();
        m_db.rollback();
        return false;
    }

    // 获取新插入数据的ID
    int newId = sql.lastInsertId().toInt();
    if (newId <= 0) {
        qDebug() << "获取新插入ID失败";
        m_db.rollback();
        return false;
    }

    // 使用姓名+ID作为用户名
    QString uniqueUsername = info.name + QString::number(newId);

    // 创建对应的用户条目
    UserInfo newUser;
    newUser.username = uniqueUsername;
    newUser.password = "123456"; // 使用明文密码，让 AddUser 方法处理哈希
    newUser.aut = "user"; // 默认权限
    newUser.role = ROLE_PATIENT; // 默认角色为患者
    newUser.userid = newId; // 关联到生理参数数据ID

    // 添加用户
    if (!AddUser(newUser)) {
        qDebug() << "创建用户失败:" << sql.lastError().text();
                m_db.rollback();
        return false;
    }

    // 添加日志记录
    if (!addLogRecord(m_currentUser, "添加数据与用户",
            QString("添加了生理参数数据(ID:%1)和对应用户(%2)")
            .arg(newId).arg(uniqueUsername))) {
        qDebug() << "添加日志记录失败";
        // 不因日志记录失败回滚事务
    }

    m_db.commit();
    qDebug() << "成功添加数据和用户账号: " << uniqueUsername;
    return true;
}

// 批量添加数据函数
bool userSql::addData(QList<BaseInfo> info_list)
{
    if (info_list.isEmpty())
        return true;

    QSqlQuery sql(m_db);

    // 使用事务处理
    m_db.transaction();

    // 准备SQL语句
    QString strSql = "INSERT INTO 'Human physiological parameter data' ("
                     "name, age, temperature, bloodpressure, ECGsignal, "
                     "bloodoxygen, respiratoryrate, time, checkProjectNumber) "
                     "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)";

    sql.prepare(strSql);

    QList<int> newIds;
    QList<QString> names;

    // 添加所有生理参数数据
    for (const auto &info : info_list) {
        sql.bindValue(0, info.name);
        sql.bindValue(1, info.age);
        sql.bindValue(2, info.temperature);
        sql.bindValue(3, info.bloodpressure);
        sql.bindValue(4, info.ECGsignal);
        sql.bindValue(5, info.bloodoxygen);
        sql.bindValue(6, info.respiratoryrate);
        sql.bindValue(7, info.time);
        sql.bindValue(8, info.checkProjectNumber);

        if (!sql.exec()) {
            qDebug() << "添加生理参数数据失败:" << sql.lastError().text();
            m_db.rollback();
            return false;
        }

        // 保存新插入数据的ID和姓名
        newIds.append(sql.lastInsertId().toInt());
        names.append(info.name);
    }

    // 创建批量用户记录
    int successCount = 0;

    // 为每条数据创建用户账号
    for (int i = 0; i < newIds.size(); i++) {
        // 使用姓名+ID作为用户名
        QString uniqueUsername = names[i] + QString::number(newIds[i]);

        // 创建用户
        UserInfo newUser;
        newUser.username = uniqueUsername;
        newUser.password = "123456";  // 使用明文密码
        newUser.aut = "user"; // 默认权限
        newUser.role = ROLE_PATIENT; // 默认角色
        newUser.userid = newIds[i]; // 关联ID

        if (AddUser(newUser)) {
            successCount++;
        } else {
            qDebug() << "创建用户失败(index" << i << "):" << sql.lastError().text();
        }
    }

    // 记录批量操作
    addLogRecord(m_currentUser, "批量添加数据与用户",
            QString("成功添加%1条生理参数数据和%2个用户账号")
            .arg(newIds.size()).arg(successCount));

    m_db.commit();
    return true;
}

bool userSql::delData(int id)
{
    // 使用事务确保原子性
    m_db.transaction();

    // 首先查找与该ID关联的用户
    QSqlQuery findUserQuery(m_db);
    findUserQuery.prepare("SELECT username FROM user WHERE userid = ?");
    findUserQuery.addBindValue(id);
    findUserQuery.exec();

    QString username;
    if (findUserQuery.next()) {
        username = findUserQuery.value(0).toString();

        // 如果找到了关联用户，则删除该用户
        if (!username.isEmpty()) {
            QSqlQuery deleteUserQuery(m_db);
            deleteUserQuery.prepare("DELETE FROM user WHERE username = ?");
            deleteUserQuery.addBindValue(username);

            if (!deleteUserQuery.exec()) {
                qDebug() << "删除关联用户失败:" << deleteUserQuery.lastError().text();
                                                               m_db.rollback();
                return false;
            }

            qDebug() << "已删除与ID" << id << "关联的用户:" << username;
        }
    }

    // 然后删除生理参数数据
    QSqlQuery deleteDataQuery(m_db);
    deleteDataQuery.prepare("DELETE FROM 'Human physiological parameter data' WHERE id = ?");
    deleteDataQuery.addBindValue(id);

    bool success = deleteDataQuery.exec();

    if (success) {
        m_db.commit();

        // 记录日志
        if (!username.isEmpty()) {
            addLogRecord(m_currentUser, "删除数据和用户",
                        QString("删除了ID为%1的生理参数数据及关联用户%2").arg(id).arg(username));
        } else {
            addLogRecord(m_currentUser, "删除数据",
                        QString("删除了ID为%1的生理参数数据").arg(id));
        }
    } else {
        m_db.rollback();
        qDebug() << "删除生理参数数据失败:" << deleteDataQuery.lastError().text();
    }

    return success;
}

void userSql::clearDataTable()
{
    // 使用事务确保操作的原子性
    m_db.transaction();

    QSqlQuery sql(m_db);

    // 首先删除所有患者角色的用户（保留系统用户如admin、doctor等）
    bool userDelSuccess = sql.exec("DELETE FROM user WHERE role = 3");
    if (!userDelSuccess) {
        qDebug() << "清空用户表中的患者数据失败:" << sql.lastError().text();
                    m_db.rollback();
        return;
    }

    // 然后清空生理参数数据表
    bool dataDelSuccess = sql.exec("DELETE FROM 'Human physiological parameter data'");
    if (!dataDelSuccess) {
        qDebug() << "清空生理参数数据表失败:" << sql.lastError().text();
                                                                m_db.rollback();
        return;
    }

    // 重置自增ID
    bool resetSuccess = sql.exec("UPDATE sqlite_sequence SET seq=0 WHERE name='Human physiological parameter data'");
    if (!resetSuccess) {
        qDebug() << "重置生理参数数据表ID失败:" << sql.lastError().text();
                                                                  m_db.rollback();
        return;
    }

    // 如果所有操作成功，提交事务
    m_db.commit();

    // 添加日志记录
    addLogRecord(m_currentUser, "清空数据",
                "清空了所有生理参数数据和关联的用户账号");

    qDebug() << "成功清空生理参数数据表和关联的用户数据";
}

bool userSql::UpdateDataInfo(BaseInfo info)
{
    QSqlQuery sql(m_db);

    m_db.transaction();

    sql.prepare("UPDATE 'Human physiological parameter data' SET "
                "name=?, age=?, temperature=?, bloodPressure=?, "
                "ECGSignal=?, bloodOxygen=?, respiratoryRate=?, time=?, "
                "checkProjectNumber=? WHERE id=?");

    sql.addBindValue(info.name);
    sql.addBindValue(info.age);
    sql.addBindValue(info.temperature);
    sql.addBindValue(info.bloodpressure);
    sql.addBindValue(info.ECGsignal);
    sql.addBindValue(info.bloodoxygen);
    sql.addBindValue(info.respiratoryrate);
    sql.addBindValue(info.time);
    sql.addBindValue(info.checkProjectNumber);
    // 删除user_id绑定
    sql.addBindValue(info.id);

    bool ret = sql.exec();

    if(ret) {
        m_db.commit();
    } else {
        m_db.rollback();
        qDebug() << "Error updating data: " << sql.lastError().text();
    }

    return ret;
}

QList<UserInfo> userSql::getAllUser()
{
    QList<UserInfo> l;
    QSqlQuery query(m_db);
    QString sql = "select * from user";
    query.exec(sql);

    while (query.next()) {
        UserInfo info;
        info.username = query.value("username").toString();
        info.password = query.value("password").toString();
        info.aut = query.value("auth").toString();
        info.role = static_cast<UserRole>(query.value("role").toInt());
        info.userid = query.value("userid").toInt();

        l.append(info);
    }
    return l;
}

bool userSql::isExist(QString strUser)
{
    QSqlQuery sql(m_db);
    sql.prepare("SELECT * FROM user WHERE username = ?");
    sql.addBindValue(strUser);
    sql.exec();
    return sql.next();
}

bool userSql::updateUser(UserInfo info)
{
    QSqlQuery sql(m_db);
    sql.prepare("UPDATE user SET password=?, auth=?, role=?, userid=? WHERE username=?");

    sql.addBindValue(info.password);
    sql.addBindValue(info.aut);
    sql.addBindValue(static_cast<int>(info.role));
    sql.addBindValue(info.userid);
    sql.addBindValue(info.username);

    bool ret = sql.exec();
    if(!ret) {
        qDebug() << "Error updating user: " << sql.lastError().text();
    }

    return ret;
}

bool userSql::AddUser(UserInfo info)
{
    // 添加调试输出
    qDebug() << "尝试添加用户:" << info.username << "角色:" << info.role << "userid:" << info.userid;

    // 确保非患者账号的userid为0
    if (info.role != ROLE_PATIENT) {
        info.userid = 0;
    }

    QSqlQuery sql(m_db);

    // 明确指定列名，解决列顺序问题
    sql.prepare("INSERT INTO user (username, password, auth, role, userid) VALUES(?, ?, ?, ?, ?)");

    // 使用哈希密码
    QString hashedPassword = hashPassword(info.password);

    sql.addBindValue(info.username);
    sql.addBindValue(hashedPassword);
    sql.addBindValue(info.aut);
    sql.addBindValue(static_cast<int>(info.role));
    sql.addBindValue(info.userid);

    bool result = sql.exec();
    if (!result) {
        qDebug() << "添加用户失败: " << sql.lastError().text();
    } else {
        qDebug() << "成功添加用户: " << info.username;
    }

    return result;
}

bool userSql::delUser(const QString &username)
{
    // 阻止删除当前登录用户
    if (username == m_currentUser) {
        QMessageBox::warning(nullptr, "警告", "不能删除当前登录的用户");
        return false;
    }

    // 使用事务确保原子性
    m_db.transaction();

    // 首先查找与该用户关联的生理参数数据ID
    QSqlQuery findDataQuery(m_db);
    findDataQuery.prepare("SELECT userid FROM user WHERE username = ?");
    findDataQuery.addBindValue(username);
    findDataQuery.exec();

    int dataId = -1;
    if (findDataQuery.next()) {
        dataId = findDataQuery.value(0).toInt();
    }

    // 删除用户
    QSqlQuery deleteUserQuery(m_db);
    deleteUserQuery.prepare("DELETE FROM user WHERE username = ?");
    deleteUserQuery.addBindValue(username);
    bool userDeleted = deleteUserQuery.exec();

    if (!userDeleted) {
        qDebug() << "删除用户失败:" << deleteUserQuery.lastError().text();
        m_db.rollback();
        return false;
    }

    // 如果用户有关联的生理参数数据，则删除这些数据
    if (dataId > 0) {
        QSqlQuery deleteDataQuery(m_db);
        deleteDataQuery.prepare("DELETE FROM 'Human physiological parameter data' WHERE id = ?");
        deleteDataQuery.addBindValue(dataId);
        bool dataDeleted = deleteDataQuery.exec();

        if (!dataDeleted) {
            qDebug() << "删除关联的生理参数数据失败:" << deleteDataQuery.lastError().text();
            m_db.rollback();
            return false;
        }

        qDebug() << "已删除与用户" << username << "关联的生理参数数据ID:" << dataId;
    }

    // 提交事务
    m_db.commit();

    // 添加日志记录
    if (dataId > 0) {
        addLogRecord(m_currentUser, "删除用户和数据",
                   QString("删除了用户%1及其关联的生理参数数据(ID:%2)").arg(username).arg(dataId));
    } else {
        addLogRecord(m_currentUser, "删除用户",
                   QString("删除了用户%1").arg(username));
    }

    return true;
}

UserRole userSql::getUserRole(const QString& username)
{
    QSqlQuery sql(m_db);
    sql.prepare("SELECT role FROM user WHERE username = ?");
    sql.addBindValue(username);
    sql.exec();

    if (sql.next()) {
        return static_cast<UserRole>(sql.value(0).toInt());
    }

    // 默认为普通用户
    return ROLE_PATIENT;
}

// 根据ID获取生理参数姓名
QString userSql::getNameById(int id) {
    QSqlQuery query(m_db);
    query.prepare("SELECT name FROM 'Human physiological parameter data' WHERE id = ?");
    query.addBindValue(id);
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return "";
}

bool userSql::addLogRecord(const QString& username, const QString& operation, const QString& details)
{
    QSqlQuery sql(m_db);
    sql.prepare("INSERT INTO LogRecords (username, operation, details, timestamp) VALUES (?, ?, ?, ?)");

    QDateTime currentTime = QDateTime::currentDateTime();
    sql.addBindValue(username);
    sql.addBindValue(operation);
    sql.addBindValue(details);
    sql.addBindValue(currentTime.toString("yyyy-MM-dd HH:mm:ss"));

    return sql.exec();
}

QList<LogRecord> userSql::getLogRecords(int limit)
{
    QList<LogRecord> logs;
    QSqlQuery query(m_db);

    // 修改排序方式为 ASC（升序）
    query.prepare("SELECT id, username, operation, details, timestamp FROM LogRecords ORDER BY id ASC LIMIT ?");
    query.addBindValue(limit);

    if (query.exec()) {
        while (query.next()) {
            LogRecord log;
            log.id = query.value(0).toInt();
            log.username = query.value(1).toString();
            log.operation = query.value(2).toString();
            log.details = query.value(3).toString();
            log.timestamp = query.value(4).toString();
            logs.append(log);
        }
    }

    return logs;
}

// 基于条件筛选日志
QList<LogRecord> userSql::getFilteredLogs(const QDate &startDate, const QDate &endDate,
        const QString &operation, const QString &username, int limit)
{
    QList<LogRecord> logs;
    QSqlQuery query(m_db);

    // 构建SQL查询
    QString sql = "SELECT id, username, operation, details, timestamp FROM LogRecords WHERE 1=1";
    QStringList conditions;
    QList<QVariant> bindValues;

    // 添加日期范围条件
    if(startDate.isValid()) {
        conditions << "timestamp >= ?";
        bindValues << startDate.toString("yyyy-MM-dd");
    }

    if(endDate.isValid()) {
        conditions << "timestamp <= ?";
        bindValues << endDate.toString("yyyy-MM-dd") + " 23:59:59";
    }

    // 添加操作类型条件
    if(!operation.isEmpty()) {
        conditions << "operation = ?";
        bindValues << operation;
    }

    // 添加用户名条件
    if(!username.isEmpty()) {
        conditions << "username = ?";
        bindValues << username;
    }

    // 构建完整SQL
    if(!conditions.isEmpty()) {
        sql += " AND " + conditions.join(" AND ");
    }

    // 修改排序方式为 ASC（升序）
    sql += " ORDER BY id ASC";

    if(limit > 0) {
        sql += " LIMIT ?";
        bindValues << limit;
    }

    query.prepare(sql);

    // 绑定值
    for(const QVariant &value : bindValues) {
        query.addBindValue(value);
    }

    if(query.exec()) {
        while(query.next()) {
            LogRecord log;
            log.id = query.value(0).toInt();
            log.username = query.value(1).toString();
            log.operation = query.value(2).toString();
            log.details = query.value(3).toString();
            log.timestamp = query.value(4).toString();
            logs.append(log);
        }
    } else {
        qDebug() << "Error querying logs: " << query.lastError().text();
    }

    return logs;
}

// 实现多用户筛选方法
QList<LogRecord> userSql::getFilteredLogsByUsers(const QDate &startDate, const QDate &endDate,
        const QString &operation, const QStringList &usernames, int limit)
{
    QList<LogRecord> logs;
    QSqlQuery query(m_db);

    // 构建SQL
    QString sql = "SELECT id, username, operation, details, timestamp FROM LogRecords WHERE 1=1";
    QStringList conditions;
    QList<QVariant> bindValues;

    // 添加日期范围条件
    if(startDate.isValid()) {
        conditions << "timestamp >= ?";
        bindValues << startDate.toString("yyyy-MM-dd");
    }

    if(endDate.isValid()) {
        conditions << "timestamp <= ?";
        bindValues << endDate.toString("yyyy-MM-dd") + " 23:59:59";
    }

    // 添加操作类型条件
    if(!operation.isEmpty()) {
        conditions << "operation = ?";
        bindValues << operation;
    }

    // 添加用户名条件 - 使用IN子句
    if(!usernames.isEmpty()) {
        QStringList placeholders;
        for(int i = 0; i < usernames.size(); ++i) {
            placeholders << "?";
            bindValues << usernames[i];
        }
        conditions << QString("username IN (%1)").arg(placeholders.join(","));
    }

    // 构建完整SQL
    if(!conditions.isEmpty()) {
        sql += " AND " + conditions.join(" AND ");
    }

    // 修改排序方式为 ASC（升序）
    sql += " ORDER BY id ASC";

    if(limit > 0) {
        sql += " LIMIT ?";
        bindValues << limit;
    }

    query.prepare(sql);

    // 绑定值
    for(const QVariant &value : bindValues) {
        query.addBindValue(value);
    }

    if(query.exec()) {
        while(query.next()) {
            LogRecord log;
            log.id = query.value(0).toInt();
            log.username = query.value(1).toString();
            log.operation = query.value(2).toString();
            log.details = query.value(3).toString();
            log.timestamp = query.value(4).toString();
            logs.append(log);
        }
    } else {
        qDebug() << "Error querying logs by users: " << query.lastError().text();
    }

    return logs;
}

// 清除日志
bool userSql::clearLogs()
{
    QSqlQuery query(m_db);

    // 开始事务处理
    m_db.transaction();

    // 执行删除操作
    bool success = query.exec("DELETE FROM LogRecords");

    if (!success) {
        m_db.rollback();
        qDebug() << "清除日志失败:" << query.lastError().text();
                return false;
    }

    // 重置自增ID
    success = query.exec("UPDATE sqlite_sequence SET seq=0 WHERE name='LogRecords'");

    if (!success) {
        m_db.rollback();
        qDebug() << "重置日志ID失败:" << query.lastError().text();
                return false;
    }

    // 提交事务
    m_db.commit();
    return true;
}

bool userSql::importLogs(const QList<LogRecord>& logs)
{
    if (logs.isEmpty())
        return true;

    // 使用事务处理批量导入
    m_db.transaction();

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO LogRecords (username, operation, details, timestamp) VALUES (?, ?, ?, ?)");

    bool success = true;

    for (const LogRecord& log : logs) {
        query.bindValue(0, log.username);
        query.bindValue(1, log.operation);
        query.bindValue(2, log.details);
        query.bindValue(3, log.timestamp);

        if (!query.exec()) {
            qDebug() << "导入日志记录失败:" << query.lastError().text();
                    success = false;
            break;
        }
    }

    if (success) {
        m_db.commit();
        qDebug() << "成功导入" << logs.size() << "条日志记录";
    } else {
        m_db.rollback();
        qDebug() << "日志导入失败，已回滚事务";
    }

    return success;
}

bool userSql::getStatisticsData(double& avgTemp, double& minTemp, double& maxTemp,
                                int& avgBP, int& minBP, int& maxBP,
                                int& avgECG, int& minECG, int& maxECG,
                                int& avgO2, int& minO2, int& maxO2,
                                int& avgRR, int& minRR, int& maxRR,
                                int& totalCount)
{
    QSqlQuery query(m_db);
    query.exec("SELECT "
               "AVG(temperature) as avg_temp, "
               "MIN(temperature) as min_temp, "
               "MAX(temperature) as max_temp, "
               "AVG(bloodpressure) as avg_bp, "
               "MIN(bloodpressure) as min_bp, "
               "MAX(bloodpressure) as max_bp, "
               "AVG(ECGsignal) as avg_ecg, "
               "MIN(ECGsignal) as min_ecg, "
               "MAX(ECGsignal) as max_ecg, "
               "AVG(bloodoxygen) as avg_o2, "
               "MIN(bloodoxygen) as min_o2, "
               "MAX(bloodoxygen) as max_o2, "
               "AVG(respiratoryrate) as avg_rr, "
               "MIN(respiratoryrate) as min_rr, "
               "MAX(respiratoryrate) as max_rr, "
               "COUNT(*) as total_count "
               "FROM 'Human physiological parameter data'");

    if (query.next()) {
        avgTemp = query.value("avg_temp").toDouble();
        minTemp = query.value("min_temp").toDouble();
        maxTemp = query.value("max_temp").toDouble();

        avgBP = query.value("avg_bp").toInt();
        minBP = query.value("min_bp").toInt();
        maxBP = query.value("max_bp").toInt();

        avgECG = query.value("avg_ecg").toInt();
        minECG = query.value("min_ecg").toInt();
        maxECG = query.value("max_ecg").toInt();

        avgO2 = query.value("avg_o2").toInt();
        minO2 = query.value("min_o2").toInt();
        maxO2 = query.value("max_o2").toInt();

        avgRR = query.value("avg_rr").toInt();
        minRR = query.value("min_rr").toInt();
        maxRR = query.value("max_rr").toInt();

        totalCount = query.value("total_count").toInt();

        return true;
    }

    return false;
}

QList<BaseInfo> userSql::getDataByPatientId(int patientId)
{
    QList<BaseInfo> results;
    QSqlQuery query(m_db);

    // 查询与指定ID匹配的所有生理参数数据
    // 查询id列，因为用户表中的userid与生理参数表中的id关联
    query.prepare("SELECT * FROM 'Human physiological parameter data' WHERE id = ?");
    query.addBindValue(patientId);

    if (query.exec()) {
        while (query.next()) {
            BaseInfo info;
            info.id = query.value(0).toUInt();
            info.name = query.value(1).toString();
            info.age = query.value(2).toUInt();
            info.temperature = query.value(3).toDouble();
            info.bloodpressure = query.value(4).toUInt();
            info.ECGsignal = query.value(5).toUInt();
            info.bloodoxygen = query.value(6).toUInt();
            info.respiratoryrate = query.value(7).toUInt();
            info.time = query.value(8).toString();
            info.checkProjectNumber = query.value(9).toUInt();
            results.append(info);
        }
    } else {
        qDebug() << "查询患者数据失败:" << query.lastError().text();
    }

    return results;
}
