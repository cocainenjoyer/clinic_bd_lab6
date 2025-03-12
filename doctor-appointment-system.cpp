#include <iostream>
#include <string>
#include <mysql/mysql.h>
#include <vector>
#include <limits>

class DatabaseManager {
private:
    MYSQL* conn;
    std::string currentUser;
    std::string currentRole;

    // Проверка прав доступа
    bool checkAdminAccess() {
        if (currentRole != "admin") {
            std::cout << "Отказано в доступе: требуются права администратора." << std::endl;
            return false;
        }
        return true;
    }

public:
    DatabaseManager() : conn(nullptr) {}

    ~DatabaseManager() {
        if (conn) {
            mysql_close(conn);
        }
    }

    // Подключение к серверу MySQL
    bool connect(const std::string& host, const std::string& user, 
                 const std::string& password, const std::string& db = "") {
        conn = mysql_init(nullptr);
        if (!conn) {
            std::cout << "Ошибка инициализации MySQL: " << mysql_error(conn) << std::endl;
            return false;
        }

        if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(), 
                               db.empty() ? nullptr : db.c_str(), 0, nullptr, 0)) {
            std::cout << "Ошибка подключения к MySQL: " << mysql_error(conn) << std::endl;
            mysql_close(conn);
            conn = nullptr;
            return false;
        }

        currentUser = user;
        
        // Определение роли пользователя
        if (!db.empty()) {
            std::string query = "SELECT role FROM users WHERE username = '" + user + "'";
            if (mysql_query(conn, query.c_str()) == 0) {
                MYSQL_RES* result = mysql_store_result(conn);
                if (result) {
                    MYSQL_ROW row = mysql_fetch_row(result);
                    if (row) {
                        currentRole = row[0];
                    } else {
                        currentRole = "guest"; // По умолчанию гость
                    }
                    mysql_free_result(result);
                }
            }
        }
        
        return true;
    }

    // Создание базы данных
    bool createDatabase(const std::string& dbName) {
        if (!checkAdminAccess()) return false;
        
        std::string query = "CALL create_database('" + dbName + "')";
        if (mysql_query(conn, query.c_str()) != 0) {
            std::cout << "Ошибка создания базы данных: " << mysql_error(conn) << std::endl;
            return false;
        }
        
        std::cout << "База данных " << dbName << " успешно создана." << std::endl;
        return true;
    }

    // Удаление базы данных
    bool dropDatabase(const std::string& dbName) {
        if (!checkAdminAccess()) return false;
        
        std::string query = "CALL drop_database('" + dbName + "')";
        if (mysql_query(conn, query.c_str()) != 0) {
            std::cout << "Ошибка удаления базы данных: " << mysql_error(conn) << std::endl;
            return false;
        }
        
        std::cout << "База данных " << dbName << " успешно удалена." << std::endl;
        return true;
    }

    // Создание таблицы appointments
    bool createAppointmentsTable() {
        if (!checkAdminAccess()) return false;
        
        std::string query = "CALL create_appointments_table()";
        if (mysql_query(conn, query.c_str()) != 0) {
            std::cout << "Ошибка создания таблицы appointments: " << mysql_error(conn) << std::endl;
            return false;
        }
        
        std::cout << "Таблица appointments успешно создана." << std::endl;
        return true;
    }

    // Создание таблицы пользователей
    bool createUsersTable() {
        if (!checkAdminAccess()) return false;
        
        std::string query = "CALL create_users_table()";
        if (mysql_query(conn, query.c_str()) != 0) {
            std::cout << "Ошибка создания таблицы users: " << mysql_error(conn) << std::endl;
            return false;
        }
        
        std::cout << "Таблица users успешно создана." << std::endl;
        return true;
    }

    // Очистка таблицы
    bool truncateTable(const std::string& tableName) {
        if (!checkAdminAccess()) return false;
        
        std::string query = "CALL truncate_table('" + tableName + "')";
        if (mysql_query(conn, query.c_str()) != 0) {
            std::cout << "Ошибка очистки таблицы: " << mysql_error(conn) << std::endl;
            return false;
        }
        
        std::cout << "Таблица " << tableName << " успешно очищена." << std::endl;
        return true;
    }

    // Добавление новой записи к врачу
    bool addAppointment(const std::string& patientName, const std::string& doctorName, 
                        const std::string& appointmentDate, const std::string& diagnosis) {
        if (!checkAdminAccess()) return false;
        
        std::string query = "CALL add_appointment('" + patientName + "', '" + 
                            doctorName + "', '" + appointmentDate + "', '" + 
                            diagnosis + "')";
        
        if (mysql_query(conn, query.c_str()) != 0) {
            std::cout << "Ошибка добавления записи: " << mysql_error(conn) << std::endl;
            return false;
        }
        
        std::cout << "Запись к врачу успешно добавлена." << std::endl;
        return true;
    }

    // Поиск записей по текстовому неключевому полю
    bool searchAppointments(const std::string& fieldName, const std::string& searchValue) {
        std::string query = "CALL search_appointments('" + fieldName + "', '" + searchValue + "')";
        
        if (mysql_query(conn, query.c_str()) != 0) {
            std::cout << "Ошибка поиска записей: " << mysql_error(conn) << std::endl;
            return false;
        }
        
        MYSQL_RES* result = mysql_store_result(conn);
        if (!result) {
            std::cout << "Ошибка получения результатов: " << mysql_error(conn) << std::endl;
            return false;
        }
        
        int numFields = mysql_num_fields(result);
        MYSQL_FIELD* fields = mysql_fetch_fields(result);
        
        // Вывод заголовков
        for (int i = 0; i < numFields; i++) {
            std::cout << fields[i].name << "\t";
        }
        std::cout << std::endl;
        
        // Вывод данных
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            for (int i = 0; i < numFields; i++) {
                std::cout << (row[i] ? row[i] : "NULL") << "\t";
            }
            std::cout << std::endl;
        }
        
        mysql_free_result(result);
        return true;
    }

    // Обновление записи
    bool updateAppointment(int appointmentId, const std::string& fieldName, const std::string& newValue) {
        if (!checkAdminAccess()) return false;
        
        std::string query = "CALL update_appointment(" + std::to_string(appointmentId) + 
                           ", '" + fieldName + "', '" + newValue + "')";
        
        if (mysql_query(conn, query.c_str()) != 0) {
            std::cout << "Ошибка обновления записи: " << mysql_error(conn) << std::endl;
            return false;
        }
        
        std::cout << "Запись успешно обновлена." << std::endl;
        return true;
    }

    // Удаление записи по текстовому неключевому полю
    bool deleteAppointmentByField(const std::string& fieldName, const std::string& fieldValue) {
        if (!checkAdminAccess()) return false;
        
        std::string query = "CALL delete_appointment_by_field('" + fieldName + "', '" + fieldValue + "')";
        
        if (mysql_query(conn, query.c_str()) != 0) {
            std::cout << "Ошибка удаления записи: " << mysql_error(conn) << std::endl;
            return false;
        }
        
        std::cout << "Записи успешно удалены." << std::endl;
        return true;
    }

    // Вывод всех записей
    bool showAllAppointments() {
        std::string query = "CALL show_all_appointments()";
        
        if (mysql_query(conn, query.c_str()) != 0) {
            std::cout << "Ошибка получения всех записей: " << mysql_error(conn) << std::endl;
            return false;
        }
        
        MYSQL_RES* result = mysql_store_result(conn);
        if (!result) {
            std::cout << "Ошибка получения результатов: " << mysql_error(conn) << std::endl;
            return false;
        }
        
        int numFields = mysql_num_fields(result);
        MYSQL_FIELD* fields = mysql_fetch_fields(result);
        
        // Вывод заголовков
        for (int i = 0; i < numFields; i++) {
            std::cout << fields[i].name << "\t";
        }
        std::cout << std::endl;
        
        // Вывод данных
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            for (int i = 0; i < numFields; i++) {
                std::cout << (row[i] ? row[i] : "NULL") << "\t";
            }
            std::cout << std::endl;
        }
        
        mysql_free_result(result);
        return true;
    }

    // Создание нового пользователя
    bool createUser(const std::string& username, const std::string& password, const std::string& role) {
        if (!checkAdminAccess()) return false;
        
        std::string query = "CALL create_user('" + username + "', '" + password + "', '" + role + "')";
        
        if (mysql_query(conn, query.c_str()) != 0) {
            std::cout << "Ошибка создания пользователя: " << mysql_error(conn) << std::endl;
            return false;
        }
        
        std::cout << "Пользователь " << username << " с ролью " << role << " успешно создан." << std::endl;
        return true;
    }

    // Получение текущей роли пользователя
    std::string getCurrentRole() const {
        return currentRole;
    }
};

// Функция для отображения меню
void showMenu(const std::string& role) {
    std::cout << "\n=== Система записи к врачу ===\n";
    std::cout << "Текущая роль: " << role << "\n\n";
    
    std::cout << "1. Просмотреть все записи\n";
    std::cout << "2. Поиск записей\n";
    
    if (role == "admin") {
        std::cout << "3. Создать базу данных\n";
        std::cout << "4. Удалить базу данных\n";
        std::cout << "5. Создать таблицу записей\n";
        std::cout << "6. Создать таблицу пользователей\n";
        std::cout << "7. Очистить таблицу\n";
        std::cout << "8. Добавить новую запись к врачу\n";
        std::cout << "9. Обновить запись\n";
        std::cout << "10. Удалить записи\n";
        std::cout << "11. Создать нового пользователя\n";
    }
    
    std::cout << "0. Выход\n";
    std::cout << "Выберите действие: ";
}

int main() {
    // Установка кодировки для корректного отображения русских символов
    setlocale(LC_ALL, "Russian");
    
    std::string host, username, password, dbName;
    
    std::cout << "=== Система записи к врачу ===\n";
    std::cout << "Введите хост MySQL: ";
    std::getline(std::cin, host);
    std::cout << "Введите имя пользователя: ";
    std::getline(std::cin, username);
    std::cout << "Введите пароль: ";
    std::getline(std::cin, password);
    std::cout << "Введите имя базы данных (оставьте пустым для создания новой): ";
    std::getline(std::cin, dbName);
    
    DatabaseManager dbManager;
    if (!dbManager.connect(host, username, password, dbName)) {
        std::cout << "Не удалось подключиться к серверу MySQL." << std::endl;
        return 1;
    }
    
    int choice;
    std::string currentRole = dbManager.getCurrentRole();
    
    while (true) {
        showMenu(currentRole);
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        if (choice == 0) {
            break;
        }
        
        switch (choice) {
            case 1: // Просмотр всех записей
                dbManager.showAllAppointments();
                break;
                
            case 2: // Поиск записей
                {
                    std::string field, value;
                    std::cout << "Введите поле для поиска (patient_name, doctor_name, diagnosis): ";
                    std::getline(std::cin, field);
                    std::cout << "Введите значение для поиска: ";
                    std::getline(std::cin, value);
                    dbManager.searchAppointments(field, value);
                }
                break;
                
            case 3: // Создать базу данных
                if (currentRole == "admin") {
                    std::string newDbName;
                    std::cout << "Введите имя новой базы данных: ";
                    std::getline(std::cin, newDbName);
                    dbManager.createDatabase(newDbName);
                } else {
                    std::cout << "Недостаточно прав для этой операции." << std::endl;
                }
                break;
                
            case 4: // Удалить базу данных
                if (currentRole == "admin") {
                    std::string dropDbName;
                    std::cout << "Введите имя базы данных для удаления: ";
                    std::getline(std::cin, dropDbName);
                    dbManager.dropDatabase(dropDbName);
                } else {
                    std::cout << "Недостаточно прав для этой операции." << std::endl;
                }
                break;
                
            case 5: // Создать таблицу записей
                if (currentRole == "admin") {
                    dbManager.createAppointmentsTable();
                } else {
                    std::cout << "Недостаточно прав для этой операции." << std::endl;
                }
                break;
                
            case 6: // Создать таблицу пользователей
                if (currentRole == "admin") {
                    dbManager.createUsersTable();
                } else {
                    std::cout << "Недостаточно прав для этой операции." << std::endl;
                }
                break;
                
            case 7: // Очистить таблицу
                if (currentRole == "admin") {
                    std::string tableName;
                    std::cout << "Введите имя таблицы для очистки: ";
                    std::getline(std::cin, tableName);
                    dbManager.truncateTable(tableName);
                } else {
                    std::cout << "Недостаточно прав для этой операции." << std::endl;
                }
                break;
                
            case 8: // Добавить новую запись
                if (currentRole == "admin") {
                    std::string patientName, doctorName, appointmentDate, diagnosis;
                    std::cout << "Введите имя пациента: ";
                    std::getline(std::cin, patientName);
                    std::cout << "Введите имя врача: ";
                    std::getline(std::cin, doctorName);
                    std::cout << "Введите дату приема (ГГГГ-ММ-ДД): ";
                    std::getline(std::cin, appointmentDate);
                    std::cout << "Введите диагноз: ";
                    std::getline(std::cin, diagnosis);
                    dbManager.addAppointment(patientName, doctorName, appointmentDate, diagnosis);
                } else {
                    std::cout << "Недостаточно прав для этой операции." << std::endl;
                }
                break;
                
            case 9: // Обновить запись
                if (currentRole == "admin") {
                    int id;
                    std::string field, value;
                    std::cout << "Введите ID записи для обновления: ";
                    std::cin >> id;
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Введите поле для обновления (patient_name, doctor_name, appointment_date, diagnosis): ";
                    std::getline(std::cin, field);
                    std::cout << "Введите новое значение: ";
                    std::getline(std::cin, value);
                    dbManager.updateAppointment(id, field, value);
                } else {
                    std::cout << "Недостаточно прав для этой операции." << std::endl;
                }
                break;
                
            case 10: // Удалить записи
                if (currentRole == "admin") {
                    std::string field, value;
                    std::cout << "Введите поле для удаления (patient_name, doctor_name, diagnosis): ";
                    std::getline(std::cin, field);
                    std::cout << "Введите значение для удаления: ";
                    std::getline(std::cin, value);
                    dbManager.deleteAppointmentByField(field, value);
                } else {
                    std::cout << "Недостаточно прав для этой операции." << std::endl;
                }
                break;
                
            case 11: // Создать нового пользователя
                if (currentRole == "admin") {
                    std::string newUsername, newPassword, newRole;
                    std::cout << "Введите имя нового пользователя: ";
                    std::getline(std::cin, newUsername);
                    std::cout << "Введите пароль: ";
                    std::getline(std::cin, newPassword);
                    std::cout << "Введите роль (admin/guest): ";
                    std::getline(std::cin, newRole);
                    dbManager.createUser(newUsername, newPassword, newRole);
                } else {
                    std::cout << "Недостаточно прав для этой операции." << std::endl;
                }
                break;
                
            default:
                std::cout << "Неверный выбор. Пожалуйста, попробуйте снова." << std::endl;
        }
    }
    
    return 0;
}
