CREATE TABLE if not exists  devices
(
  id_device integer,
  name character varying,
  CONSTRAINT readings_pkey PRIMARY KEY (id_device)
);

GRANT ALL ON TABLE devices TO postgres;
GRANT ALL ON TABLE devices TO public;

delete from devices where id_device in (1, 2, 3);
insert into devices values (1, 'Fridge');
insert into devices values (2, 'Dungeon');
insert into devices select 3, 'Boiler';

CREATE TABLE readings
(
  idr serial NOT NULL,
  raw_dt bigint NOT NULL,
  dt timestamp without time zone,
  lamp_state integer,
  lamp_mode integer,
  watering_state integer,
  temperature double precision,
  humidity double precision,
  soil_humidity double precision,
  soil_humidity_raw double precision,
  dt_server timestamp without time zone,
  humidifier_state integer,
  id_device integer,
  humidity_target double precision,
  co2 double precision,
  fan_state smallint,
  co2_state smallint,
  CONSTRAINT readings_pkey PRIMARY KEY (idr)
);

GRANT ALL ON TABLE readings TO postgres;
GRANT ALL ON TABLE readings TO public;