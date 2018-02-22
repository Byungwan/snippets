CREATE USER 'mariadb_benchmark'@'localhost' identified by 'mariadb_benchmark';
CREATE USER 'mariadb_benchmark'@'%' identified by 'mariadb_benchmark';
GRANT ALL PRIVILEGES ON mariadb_benchmark.* to 'mariadb_benchmark'@'%';
FLUSH PRIVILEGES;

CREATE DATABASE mariadb_benchmark;
USE mariadb_benchmark;

CREATE TABLE segment_timeline (
    media_start BIGINT UNSIGNED PRIMARY KEY NOT NULL,
    media_duration BIGINT UNSIGNED DEFAULT 0,
    media_sequence INT UNSIGNED DEFAULT 0,

    file_path VARCHAR(512) NOT NULL,
    file_offset BIGINT UNSIGNED DEFAULT 0,
    file_data_size INT UNSIGNED DEFAULT 0,

    key_id BINARY(16)
);
