DROP TABLE if EXISTS osm_boundaries;
CREATE TABLE osm_boundaries(
    osm_id BIGINT ,
    boundary_name TEXT,
    boundary_level BIGINT,
    boundary_type TEXT,
    boundary_geom TEXT
    );

COPY osm_boundary_vertices (
    osm_id, boundary_name,
    boundary_level, boundary_type, boundary_geom) FROM STDIN; 