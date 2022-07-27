-- Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
-- Systems

CREATE TABLE Uniquepairs(
  id SERIAL PRIMARY KEY, 
  created_at INTEGER NOT NULL,
  domain VARCHAR(32) NOT NULL,
  first_elem INTEGER NOT NULL,
  second_elem INTEGER NOT NULL,
  UNIQUE(domain, first_elem, second_elem)
);

CREATE INDEX idx_created_at ON Uniquepairs(created_at);
CREATE INDEX idx_first_elem ON Uniquepairs(domain, first_elem);
CREATE INDEX idx_second_elem ON Uniquepairs(domain, second_elem);
CREATE INDEX idx_first_and_second_elem ON Uniquepairs(domain, first_elem, second_elem);
