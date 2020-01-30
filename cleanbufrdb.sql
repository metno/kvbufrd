-- 
-- This sql script cleans up the sqlite database that kvbufrd use as
-- a data cache and backing store. 
-- 
-- There is four tables in the database: data, kvbufr, waiting and keyval.
-- It is only the data and kvbufr tables that need routine clean up. 
--
-- We only need one week of data and one month of kvbufr. 
--
-- This clean up script is run once a day by cron.
-- It is run at night in a low activity period, kl 3:30
--

--
-- Deletes data that is too old, ie obstime is older than 7 days.
-- It is deleted from both data and text_data tables.
--

.headers on
.mode column

select date('now', '-7 day') AS "Deleting data before!";
delete from data where obstime<date('now', '-7  day');


--
-- Deletes from bufr all records that is more than 1 month old.
--
select date('now', '-31 day') AS "Deleting BUFR before!";
delete from bufr where obstime<date('now', '-31  day');

