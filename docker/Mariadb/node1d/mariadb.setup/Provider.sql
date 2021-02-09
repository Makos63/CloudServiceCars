create database if not exists vsTest;
use vsTest;


create table if not exists centralData 
(
id int auto_increment,
sensorType varchar(100), 
sensorValue varchar(100), 
sensorTimestamp varchar(100),
centralId varchar (100),
PRIMARY KEY (id)
);
--insert into centralData value (null,'traffic','maessig',"2020-01-05 14:50:05");