CREATE TABLE data (stationid integer, obstime timestamp, tbtime timestamp DEFAULT CURRENT_TIMESTAMP, original text, paramid integer, typeid integer, sensor integer, level integer, controlinfo text, useinfo text, 
                   UNIQUE(stationid, obstime, paramid, typeid, level, sensor));

CREATE TABLE bufr(wmono integer, id integer, callsign text, code text, obstime timestamp, createtime timestamp, crc integer, ccx integer, data text, bufrBase64 text, 
             UNIQUE(wmono, id, callsign, code, obstime));

CREATE TABLE waiting( wmono integer, id integer, callsign text, code text, obstime timestamp, delaytime timestamp, UNIQUE(wmono, id, callsign, code, obstime) );

CREATE TABLE keyval(key text, val text, UNIQUE(key));

CREATE INDEX data_stationid_obstime_index on data (stationid,obstime);
CREATE INDEX bufr_obstime_index on bufr (obstime);

