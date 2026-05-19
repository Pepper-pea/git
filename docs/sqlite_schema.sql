PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS persons (
    id TEXT PRIMARY KEY,
    name TEXT,
    team TEXT,
    role TEXT,
    card_no TEXT,
    list_type TEXT,
    access_level INTEGER,
    enabled INTEGER,
    feature BLOB
);

CREATE TABLE IF NOT EXISTS attendance_records (
    id TEXT PRIMARY KEY,
    person_id TEXT,
    person_name TEXT,
    team TEXT,
    direction TEXT,
    result TEXT,
    score REAL,
    device_id TEXT,
    snapshot_path TEXT,
    uploaded INTEGER,
    created_at TEXT
);

CREATE TABLE IF NOT EXISTS recognized_faces (
    id TEXT PRIMARY KEY,
    frame_id INTEGER,
    person_id TEXT,
    person_name TEXT,
    team TEXT,
    status TEXT,
    access_allowed INTEGER,
    decision TEXT,
    message TEXT,
    score REAL,
    cosine REAL,
    euclidean REAL,
    quality REAL,
    blink_detected INTEGER,
    face_x INTEGER,
    face_y INTEGER,
    face_width INTEGER,
    face_height INTEGER,
    snapshot_path TEXT,
    device_id TEXT,
    created_at TEXT
);

CREATE INDEX IF NOT EXISTS idx_recognized_faces_created_at
    ON recognized_faces(created_at);

CREATE INDEX IF NOT EXISTS idx_recognized_faces_person_id
    ON recognized_faces(person_id);

CREATE TABLE IF NOT EXISTS device_logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    level TEXT,
    message TEXT,
    created_at TEXT
);
