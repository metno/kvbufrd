CREATE TABLE data (stationid integer, obstime timestamp, original text, paramid integer, typeid integer, sensor character(1), level integer, controlinfo text, useinfo text, UNIQUE(stationid, obstime, paramid, typeid, level, sensor));

CREATE TABLE bufr(wmono integer, obstime timestamp, createtime timestamp, crc integer, ccx integer, data text, bufrBase64 text, UNIQUE(wmono, obstime));

CREATE TABLE waiting(wmono integer, obstime timestamp, delaytime timestamp, UNIQUE(wmono, obstime));

CREATE TABLE keyval(key text, val text, UNIQUE(key));

CREATE INDEX data_stationid_obstime_index on data (stationid,obstime);
CREATE INDEX bufr_obstime_index on bufr (obstime);

