#include "database_manager.h"

DatabaseManager::DatabaseManager() = default;

bool DatabaseManager::Open() {
    return db_.open();
}

void DatabaseManager::Close() {
    db_.close();
}

void DatabaseManager::UpdateConnection(const QString& host, int port, const QString& db_name, const QString& username, const QString& password) {
    db_ = QSqlDatabase::addDatabase("QPSQL");
    db_.setHostName(host);
    db_.setPort(port);
    db_.setDatabaseName(db_name);
    db_.setUserName(username);
    db_.setPassword(password);
}

QString DatabaseManager::GetLastError() const{
    return db_.lastError().text();
}

QString DatabaseManager::GetTableDescription(const QStringView table_name){
    QSqlQuery query;
    query.prepare("SELECT obj_description(oid) AS description FROM pg_class WHERE relname = :table_name;");
    query.bindValue(":table_name", table_name.toString());
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return QString();
}

QVariant DatabaseManager::ExecuteQuery(const QStringView string_query) {
    QSqlQuery query;
    if (!query.exec(string_query.toString())) {
        return query.lastError().text();
    }
    return QString();
}

QVariant DatabaseManager::ExecuteSelectQuery(const QStringView string_query) {
    QSqlQuery query;
    if (!query.exec(string_query.toString())) {
        return query.lastError().text();
    }
    return QVariant::fromValue(query);
}

int DatabaseManager::GetRowsCount(QStringView table_name) const {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM information_schema.columns WHERE table_name = :table_name;");
    query.bindValue(":table_name", table_name.toString());
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return false;
}

int DatabaseManager::GetMaxOrMinValueFromTable(const QString& max_or_min, const QString& column_name, const QString& table_name) {
    QSqlQuery query;
    query.prepare(QString("SELECT %1(%2) FROM %3").arg(max_or_min.toUpper(), column_name, table_name));

    if (!query.exec()) {
        return -1;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return -1;
}

const QStringList DatabaseManager::GetForeignKeysForColumn(const QString& table_name, const QString& column_name) {
    QSqlQuery query;
    query.prepare(R"(
        SELECT
            tc.table_name AS referencing_table,
            kcu.column_name AS referencing_column,
            ccu.table_name AS referenced_table,
            ccu.column_name AS referenced_column
        FROM
            information_schema.table_constraints AS tc
        JOIN
            information_schema.key_column_usage AS kcu
        ON
            tc.constraint_name = kcu.constraint_name
        AND
            tc.table_schema = kcu.table_schema
        JOIN
            information_schema.constraint_column_usage AS ccu
        ON
            ccu.constraint_name = tc.constraint_name
        AND
            ccu.table_schema = tc.table_schema
        WHERE
            ccu.table_name = :table_name AND
            ccu.column_name = :column_name AND
            tc.constraint_type = 'FOREIGN KEY';
    )");

    query.bindValue(":table_name", table_name);
    query.bindValue(":column_name", column_name);

    QStringList foreign_keys;

    if (!query.exec()) {
        qDebug() << "Query execution failed:" << query.lastError().text();
        return foreign_keys;
    }

    while (query.next()) {
        QString referencing_table = query.value("referencing_table").toString();
        QString referencing_column = query.value("referencing_column").toString();
        QString referenced_table = query.value("referenced_table").toString();
        QString referenced_column = query.value("referenced_column").toString();
        foreign_keys.append(QString("%1(%2) -> %3(%4)")
                                .arg(referencing_table, referencing_column, referenced_table, referenced_column));
    }

    return foreign_keys;
}

