-- Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
-- Systems

CREATE TABLE Accounts(
  id SERIAL PRIMARY KEY,
  created_at INTEGER NOT NULL,
  active BOOLEAN DEFAULT true,
  username VARCHAR(64) UNIQUE NOT NULL,
  password VARCHAR(64) NOT NULL,
  first_name VARCHAR(64) NOT NULL,
  last_name VARCHAR(64) NOT NULL
);

CREATE INDEX idx_created_at ON Accounts(created_at);
CREATE INDEX idx_username ON Accounts(username);
