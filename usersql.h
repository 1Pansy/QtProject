#ifndef USERSQL_H
#define USERSQL_H

#include <QObject>
#include <QSqlDatabase>
#include <QCoreApplication>
#include <QDateTime>

// 定义用户角色枚举
enum UserRole {
    ROLE_ADMIN = 0,      // 管理员
    ROLE_DOCTOR = 1,     // 医生
    ROLE_NURSE = 2,      // 护士
    ROLE_PATIENT = 3     // 普通用户/患者
};

struct BaseInfo
{
    int id;
    QString name;
    quint8 age;
    double temperature;
    quint16 bloodpressure;
    quint16 ECGsignal;
    quint8 bloodoxygen;
    quint16 respiratoryrate;
    QString time;
    quint16 checkProjectNumber;
};

struct UserInfo
{
    QString username;   // 作为用户的唯一标识符
    QString password;
    QString aut;        // 保持兼容性
    UserRole role;      // 使用枚举类型
    int userid;         // 添加userid字段，对应生理参数表的id
};

// 日志记录结构
struct LogRecord {
    int id;
    QString username;
    QString operation;
    QString details;
    QString timestamp;
};

class userSql : public QObject
{
    Q_OBJECT
public:
    static userSql *ptruserSql;
    static userSql *getinstance()
    {
        if(nullptr == ptruserSql)
        {
            ptruserSql = new userSql;
        }
        return ptruserSql;
    }

    explicit userSql(QObject *parent = nullptr);
    // 初始化
    void init();

    // 查询所有数据数量
    quint32 getDataCnt();

    // 按条件查询数据数量
    quint32 getDataCnt(const QString& patientId);

    BaseInfo getDataById(int id);

    // 查询第几页用户数据
    QList<BaseInfo> getPageData(quint32 page,quint32 uiCnt);

    // 按用户ID查询数据
    QList<BaseInfo> getUserData(const QString& userId, quint32 page, quint32 uiCnt);

    // 增加数据
    bool addData(BaseInfo info);
    bool addData(QList<BaseInfo> l);

    // 删除数据
    bool delData(int id);

    // 清空数据表
    void clearDataTable();

    // 修改数据信息
    bool UpdateDataInfo(BaseInfo info);

    // 查询所有用户
    QList<UserInfo> getAllUser();

    // 查询用户名是否存在
    bool isExist(QString strUser);

    // 更新用户信息
    bool updateUser(UserInfo info);

    // 添加单个用户
    bool AddUser(UserInfo info);

    // 删除单个用户
    bool delUser(const QString &username);

    // 获取用户角色
    UserRole getUserRole(const QString& username);

    // 根据ID获取生理参数姓名
    QString getNameById(int id);

    // 密码加密函数
    QString hashPassword(const QString& password);

    // 验证密码
    bool verifyPassword(const QString& inputPassword, const QString& storedHash);

    // 添加日志记录
    bool addLogRecord(const QString& username, const QString& operation, const QString& details);

    // 获取日志记录
    QList<LogRecord> getLogRecords(int limit = 100);

    // 添加数据筛选和导入方法
    QList<LogRecord> getFilteredLogs(const QDate &startDate, const QDate &endDate,
            const QString &operation, const QString &username,
            int limit = 1000);

    QList<LogRecord> getFilteredLogsByUsers(const QDate &startDate, const QDate &endDate,
            const QString &operation, const QStringList &usernames,
            int limit = 1000);

    bool importLogs(const QList<LogRecord>& logs);

    // 清除日志
    bool clearLogs();

    // 数据库结构版本更新
    void upgradeDatabase();

    // 统计数据功能
    bool getStatisticsData(double& avgTemp, double& minTemp, double& maxTemp,
                           int& avgBP, int& minBP, int& maxBP,
                           int& avgECG, int& minECG, int& maxECG,
                           int& avgO2, int& minO2, int& maxO2,
                           int& avgRR, int& minRR, int& maxRR,
                           int& totalCount);

    // 根据患者ID获取生理参数数据
    QList<BaseInfo> getDataByPatientId(int patientId);

public:
    QString m_currentUser; // 当前登录用户名

    // 设置当前登录用户
    void setCurrentUser(const QString &username) {
        m_currentUser = username;
    }

signals:

private:
    QSqlDatabase m_db;

    // 创建必要的数据表
    void createTables();

    // 添加默认用户
    void addDefaultUsers();
};

#endif // USERSQL_H
