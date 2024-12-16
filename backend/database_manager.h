#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QVariant>
#include <QDebug>
#include <utility>

class DatabaseManager {
public:
    DatabaseManager();

    template <typename Container, typename Port>
    DatabaseManager(Container&& host, Port&& port, Container&& db_name, Container&& username, Container&& password)
    {
        db_ = QSqlDatabase::addDatabase("QPSQL");
        db_.setHostName(std::forward<Container>(host));
        db_.setPort(std::forward<Port>(port));
        db_.setDatabaseName(std::forward<Container>(db_name));
        db_.setUserName(std::forward<Container>(username));
        db_.setPassword(std::forward<Container>(password));
    }

    bool Open();
    void Close();
    void UpdateConnection(const QString& host, int port, const QString& db_name, const QString& username, const QString& password);

    QString GetLastError() const;

    QString GetTableDescription(const QStringView table_name);

    QVariant ExecuteQuery(const QStringView string_query);
    QVariant ExecuteSelectQuery(const QStringView string_query);

    int GetRowsCount(QStringView table_name) const;

    int GetMaxOrMinValueFromTable(const QString& max_or_min, const QString& column_name, const QString& table_name);

    const QStringList GetForeignKeysForColumn(const QString& table_name, const QString& column);

private:
    QSqlDatabase db_;
};

#endif // DATABASE_MANAGER_H
