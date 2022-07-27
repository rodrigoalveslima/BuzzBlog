-- Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
-- Systems

CREATE TABLE Posts(
  id SERIAL PRIMARY KEY, 
  created_at INTEGER NOT NULL,
  active BOOLEAN DEFAULT true,
  text VARCHAR(256) NOT NULL,
  author_id INTEGER NOT NULL
);

CREATE INDEX idx_created_at ON Posts(created_at);
CREATE INDEX idx_author_id ON Posts(author_id);
