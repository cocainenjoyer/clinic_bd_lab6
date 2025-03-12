-- Создание базы данных doctor_appointments, если она не существует
-- и создание таблиц appointments и users

-- Процедура для создания базы данных
DELIMITER //
CREATE PROCEDURE create_database(IN db_name VARCHAR(64))
BEGIN
    SET @sql = CONCAT('CREATE DATABASE IF NOT EXISTS ', db_name);
    PREPARE stmt FROM @sql;
    EXECUTE stmt;
    DEALLOCATE PREPARE stmt;
END //
DELIMITER ;

-- Процедура для удаления базы данных
DELIMITER //
CREATE PROCEDURE drop_database(IN db_name VARCHAR(64))
BEGIN
    SET @sql = CONCAT('DROP DATABASE IF EXISTS ', db_name);
    PREPARE stmt FROM @sql;
    EXECUTE stmt;
    DEALLOCATE PREPARE stmt;
END //
DELIMITER ;

-- Процедура для создания таблицы appointments
DELIMITER //
CREATE PROCEDURE create_appointments_table()
BEGIN
    CREATE TABLE IF NOT EXISTS appointments (
        id INT AUTO_INCREMENT PRIMARY KEY,
        patient_name VARCHAR(100) NOT NULL,
        doctor_name VARCHAR(100) NOT NULL,
        appointment_date DATE NOT NULL,
        diagnosis VARCHAR(255),
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    );
END //
DELIMITER ;

-- Процедура для создания таблицы users
DELIMITER //
CREATE PROCEDURE create_users_table()
BEGIN
    CREATE TABLE IF NOT EXISTS users (
        id INT AUTO_INCREMENT PRIMARY KEY,
        username VARCHAR(50) NOT NULL UNIQUE,
        password VARCHAR(100) NOT NULL,
        role ENUM('admin', 'guest') NOT NULL DEFAULT 'guest',
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    );
    
    -- Добавление администратора по умолчанию, если таблица пуста
    IF (SELECT COUNT(*) FROM users) = 0 THEN
        INSERT INTO users (username, password, role) VALUES ('admin', 'admin123', 'admin');
    END IF;
END //
DELIMITER ;

-- Процедура для очистки таблицы
DELIMITER //
CREATE PROCEDURE truncate_table(IN table_name VARCHAR(64))
BEGIN
    SET @sql = CONCAT('TRUNCATE TABLE ', table_name);
    PREPARE stmt FROM @sql;
    EXECUTE stmt;
    DEALLOCATE PREPARE stmt;
END //
DELIMITER ;

-- Процедура для добавления новой записи
DELIMITER //
CREATE PROCEDURE add_appointment(
    IN p_patient_name VARCHAR(100),
    IN p_doctor_name VARCHAR(100),
    IN p_appointment_date DATE,
    IN p_diagnosis VARCHAR(255)
)
BEGIN
    INSERT INTO appointments (patient_name, doctor_name, appointment_date, diagnosis)
    VALUES (p_patient_name, p_doctor_name, p_appointment_date, p_diagnosis);
END //
DELIMITER ;

-- Процедура для поиска записей по текстовому полю
DELIMITER //
CREATE PROCEDURE search_appointments(
    IN field_name VARCHAR(64),
    IN search_value VARCHAR(255)
)
BEGIN
    SET @sql = CONCAT('SELECT * FROM appointments WHERE ', field_name, ' LIKE CONCAT(''%'', ''', search_value, ''', ''%'')');
    PREPARE stmt FROM @sql;
    EXECUTE stmt;
    DEALLOCATE PREPARE stmt;
END //
DELIMITER ;

-- Процедура для обновления записи
DELIMITER //
CREATE PROCEDURE update_appointment(
    IN p_id INT,
    IN field_name VARCHAR(64),
    IN new_value VARCHAR(255)
)
BEGIN
    SET @sql = CONCAT('UPDATE appointments SET ', field_name, ' = ''', new_value, ''' WHERE id = ', p_id);
    PREPARE stmt FROM @sql;
    EXECUTE stmt;
    DEALLOCATE PREPARE stmt;
END //
DELIMITER ;

-- Процедура для удаления записей по текстовому полю
DELIMITER //
CREATE PROCEDURE delete_appointment_by_field(
    IN field_name VARCHAR(64),
    IN field_value VARCHAR(255)
)
BEGIN
    SET @sql = CONCAT('DELETE FROM appointments WHERE ', field_name, ' LIKE CONCAT(''%'', ''', field_value, ''', ''%'')');
    PREPARE stmt FROM @sql;
    EXECUTE stmt;
    DEALLOCATE PREPARE stmt;
END //
DELIMITER ;

-- Процедура для отображения всех записей
DELIMITER //
CREATE PROCEDURE show_all_appointments()
BEGIN
    SELECT * FROM appointments ORDER BY appointment_date;
END //
DELIMITER ;

-- Процедура для создания нового пользователя
DELIMITER //
CREATE PROCEDURE create_user(
    IN p_username VARCHAR(50),
    IN p_password VARCHAR(100),
    IN p_role ENUM('admin', 'guest')
)
BEGIN
    INSERT INTO users (username, password, role)
    VALUES (p_username, p_password, p_role);
    
    -- Создаем MySQL пользователя с соответствующими правами
    IF p_role = 'admin' THEN
        SET @sql1 = CONCAT('CREATE USER IF NOT EXISTS ''', p_username, '''@''localhost'' IDENTIFIED BY ''', p_password, '''');
        SET @sql2 = CONCAT('GRANT ALL PRIVILEGES ON *.* TO ''', p_username, '''@''localhost''');
        PREPARE stmt1 FROM @sql1;
        PREPARE stmt2 FROM @sql2;
        EXECUTE stmt1;
        EXECUTE stmt2;
        DEALLOCATE PREPARE stmt1;
        DEALLOCATE PREPARE stmt2;
    ELSE
        SET @sql1 = CONCAT('CREATE USER IF NOT EXISTS ''', p_username, '''@''localhost'' IDENTIFIED BY ''', p_password, '''');
        SET @sql2 = CONCAT('GRANT SELECT ON *.* TO ''', p_username, '''@''localhost''');
        PREPARE stmt1 FROM @sql1;
        PREPARE stmt2 FROM @sql2;
        EXECUTE stmt1;
        EXECUTE stmt2;
        DEALLOCATE PREPARE stmt1;
        DEALLOCATE PREPARE stmt2;
    END IF;
    
    FLUSH PRIVILEGES;
END //
DELIMITER ;
