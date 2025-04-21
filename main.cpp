#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;

struct Columns {
    string name;
    string type;
};

using Record = map<string, string>;

class TableSchema {
    string tableName_;
    vector<Columns> columns_;
    map<int, Record> records_;
    int nextId_ = 1;

public:
    TableSchema() = default;

    TableSchema(const string& tableName, const vector<Columns>& columns)
        : tableName_(tableName), columns_(columns) {}

    const string& getName() const {
        return tableName_;
    }

    const vector<Columns>& getColumns() const {
        return columns_;
    }

    void addRecord(const Record& record) {
        records_[nextId_++] = record;
    }

    void deleteRecord(int id) {
        records_.erase(id);
    }

    void updateRecord(int id, const Record& newRecord) {
        if (records_.count(id)) {
            records_[id] = newRecord;
        }
    }

    void printRecords() const {
        cout << "Table: " << tableName_ << "\n";
        cout << left << setw(8) << "ID";
        for (const auto& col : columns_) {
            cout << setw(10) << col.name;
        }
        cout << "\n";
        for (const auto& [id, record] : records_) {
            cout << setw(8) << id;
            for (const auto& col : columns_) {
                auto it = record.find(col.name);
                cout << setw(10) << (it != record.end() ? it->second : "");
            }
            cout << "\n";
        }
    }

    bool hasRecord(int id) const {
        return records_.count(id);
    }

    void findRecord(const string& columnName, const string& value) const {
        cout << "Searching for " << columnName << " = " << value << "\n";
        bool found = false;
        cout << left << setw(8) << "ID";
        for (const auto& col : columns_) {
            cout << setw(10) << col.name;
        }
        cout << "\n";
        for (const auto& [id, record] : records_) {
            auto it = record.find(columnName);
            if (it != record.end() && it->second == value) {
                cout << setw(8) << id;
                for (const auto& col : columns_) {
                    auto valIt = record.find(col.name);
                    cout << setw(10) << (valIt != record.end() ? valIt->second : "");
                }
                cout << "\n";
                found = true;
            }
        }
        if (!found) {
            cout << "No matching records found.\n";
        }
    }
};

class DbInfo {
public:
    map<string, TableSchema> tables_;
};

class Database {
    unique_ptr<DbInfo> dbInfo_;

public:
    Database() : dbInfo_(make_unique<DbInfo>()) {}

    void createTable(const string& tableName, const vector<Columns>& columns) {
        if (dbInfo_->tables_.count(tableName)) {
            cout << "Table already exists.\n";
            return;
        }
        dbInfo_->tables_.emplace(tableName, TableSchema(tableName, columns));
        cout << "Table '" << tableName << "' created.\n";
    }

    void deleteTable(const string& tableName) {
        if (!dbInfo_->tables_.erase(tableName)) {
            cout << "No such table.\n";
        } else {
            cout << "Table '" << tableName << "' deleted.\n";
        }
    }

    void listTables() const {
        if (dbInfo_->tables_.empty()) {
            cout << "No tables defined yet.\n";
            return;
        }
        cout << "Tables:\n";
        for (const auto& pair : dbInfo_->tables_) {
            cout << " - " << pair.first << "\n";
        }
    }

    const vector<Columns>& getColumns(const string& tableName) const {
        return dbInfo_->tables_.at(tableName).getColumns();
    }

    bool hasTable(const string& name) const {
        return dbInfo_->tables_.count(name);
    }

    TableSchema& getTable(const string& name) {
        return dbInfo_->tables_.at(name);
    }
};

void printHelp() {
    cout << "Commands:\n"
         << "  help                           Show this help\n"
         << "  create <table> <col:type>...   Create a new table\n"
         << "  drop <table>                   Delete a table\n"
         << "  show                           List all tables\n"
         << "  describe <table>               Show table columns\n"
         << "  insert <table> <val>...        Insert a record\n"
         << "  delete <table> <id>            Delete a record\n"
         << "  update <table> <id> <val>...   Update a record\n"
         << "  print <table>                  Show all records in table\n"
         << "  find <table> <col> <val>       Search records by column value\n"
         << "  exit                           Exit the program\n";
}

void runInteractive() {
    Database db;
    string line;

    cout << "Welcome to the interactive DB shell!\nType 'help' for commands.\n";

    while (true) {
        cout << "> ";
        getline(cin, line);
        stringstream ss(line);

        string command;
        ss >> command;

        if (command == "exit") {
            break;
        } else if (command == "help") {
            printHelp();
        } else if (command == "create") {
            string tableName;
            ss >> tableName;
            if (tableName.empty()) {
                cout << "Table name required.\n";
                continue;
            }

            vector<Columns> cols;
            string colDef;
            while (ss >> colDef) {
                size_t pos = colDef.find(":");
                if (pos == string::npos) {
                    cout << "Invalid column format. Use name:type\n";
                    continue;
                }
                string colName = colDef.substr(0, pos);
                string colType = colDef.substr(pos + 1);
                cols.push_back({colName, colType});
            }

            db.createTable(tableName, cols);
        } else if (command == "drop") {
            string table;
            ss >> table;
            db.deleteTable(table);
        } else if (command == "show") {
            db.listTables();
        } else if (command == "describe") {
            string table;
            ss >> table;
            if (!db.hasTable(table)) {
                cout << "No such table: " << table << "\n";
                continue;
            }
            const auto& cols = db.getColumns(table);
            cout << "Columns in '" << table << "':\n";
            for (const auto& col : cols) {
                cout << " - " << col.name << " (" << col.type << ")\n";
            }
        } else if (command == "insert") {
            string table;
            ss >> table;
            if (!db.hasTable(table)) {
                cout << "Table does not exist.\n";
                continue;
            }
            auto& schema = db.getTable(table);
            Record record;
            const auto& cols = schema.getColumns();
            for (const auto& col : cols) {
                string val;
                if (!(ss >> val)) {
                    cout << "Not enough values.\n";
                    break;
                }
                record[col.name] = val;
            }
            schema.addRecord(record);
        } else if (command == "delete") {
            string table;
            int id;
            ss >> table >> id;
            if (!db.hasTable(table)) {
                cout << "Table does not exist.\n";
                continue;
            }
            db.getTable(table).deleteRecord(id);
        } else if (command == "update") {
            string table;
            int id;
            ss >> table >> id;
            if (!db.hasTable(table)) {
                cout << "Table does not exist.\n";
                continue;
            }
            auto& schema = db.getTable(table);
            if (!schema.hasRecord(id)) {
                cout << "Record not found.\n";
                continue;
            }
            Record record;
            const auto& cols = schema.getColumns();
            for (const auto& col : cols) {
                string val;
                if (!(ss >> val)) {
                    cout << "Not enough values.\n";
                    break;
                }
                record[col.name] = val;
            }
            schema.updateRecord(id, record);
        } else if (command == "print") {
            string table;
            ss >> table;
            if (!db.hasTable(table)) {
                cout << "Table does not exist.\n";
                continue;
            }
            db.getTable(table).printRecords();
        } else if (command == "find") {
            string table, column, value;
            ss >> table >> column >> value;
            if (!db.hasTable(table)) {
                cout << "Table does not exist.\n";
                continue;
            }
            db.getTable(table).findRecord(column, value);
        } else {
            cout << "Unknown command. Type 'help' for list.\n";
        }
    }
}

int main() {
    runInteractive();
    return 0;
}

