#include <iostream>
#include <iomanip>
#include <map>
#include <set>
#include <string>
#include <stdexcept>

#include <osmium/handler.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/visitor.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>

#include <pqxx/pqxx>

#include "options.hpp"
#include "handle_pgpass.hpp"

namespace {

const std::string copy_end = "\\.\n";
const int PRECISION = 15;

}  // namespace

std::string get_name(const char* nameptr) {
    return nameptr? nameptr : "NULL";
};

std::string get_point(const osmium::NodeRef &n) {
    std::ostringstream oss;
    oss  << std::setprecision(PRECISION) << n.lon() << " " << n.lat();
    return oss.str();
};

std::string get_point(const osmium::Node &n) {
    std::ostringstream oss;
    oss  << std::setprecision(PRECISION) << n.location().lon() << " " << n.location().lat();
    return "SRID=4326;POINT(" + oss.str() +  ")";
};


class NodeCount : public osmium::handler::Handler {
  public:
    NodeCount() = delete;
    NodeCount(std::map<int64_t, size_t> &node_count, std::string connInfo) :
        m_node_count(node_count),
        m_node_conn(connInfo),
        m_node_action(m_node_conn),
        m_node_stream(pqxx::stream_to::table(m_node_action, {"new_osm_nodes"}, {"osm_id", "name", "geom"})),
        m_way_conn(connInfo),
        m_way_action(m_way_conn),
        m_way_stream(pqxx::stream_to::table(m_way_action, {"new_osm_ways"}, {"osm_id", "name", "geom"}))
    {
    }


    void node(const osmium::Node& node) {
        /*
         * Check for valid location
         */
        if (!node.location().valid()) {
            std::cerr << "Found invalid location at " << node.id() << "\n";
            return;
        }

        const osmium::TagList& tags = node.tags();
        const char* nameptr = tags["name"];

        if (nfirst and true /* --addnodes */) {

            /*
             * Use this code for saving into a file
             */
            std::string sql = "COPY new_osm_nodes (osm_id, name, geom) FROM stdin WITH DELIMITER '\t' NULL 'NULL' CSV;";
            std::cout << sql << "\n";
            nfirst = false;
        }
        if (true /* --addnodes */) {

            /* hint on how to add the tags of the nodes
               UPDATE tab SET h = h || hstore(array['q', 'w'], array['11', '12']);
               */

            auto data = std::make_tuple(node.id(), nameptr, get_point(node));

            m_node_stream.write_values(data);

            /*
             * Use this code for saving into a file
             */
            std::cout << node.id() << "\t" << get_name(nameptr) << "\t" << get_point(node) << "\n";
        }

        /*
         * This is representing the filtering
         */
        const char* highway = tags["highway"];
        if (!highway) return;

        /*
         * Count the nodes that match the filter
         */
        m_node_count[node.id()]++;
    }

    void after_nodes() {
        /*
         * Finalize the COPY operation
         */
        m_node_stream.complete();
        m_node_action.commit();
        std::clog << "Completing the node stream\n";
        std::cout << copy_end;
    }

    void after_ways() {
        /*
         * Finalize the COPY operation
         */
        m_way_stream.complete();
        m_way_action.commit();
        std::clog << "Completing the way stream\n";
        std::cout << copy_end;
    }

    void way(const osmium::Way& way) {
        const osmium::TagList& tags = way.tags();
        const char* nameptr = tags["name"];
        const char* highway = way.tags()["highway"];

        if (wfirst) {
            /*
             * end of osm_nodes COPY
             */
            after_nodes();

            /*
             * Use this code for saving into a file
             */
            std::cout << "COPY osm_ways (osm_id, name, geom) FROM stdin WITH DELIMITER '\t' NULL 'NULL' CSV;\n";
            wfirst = false;
        }

        std::string points = "";
        for (const auto &n : way.nodes()) {
            /*
             * get the points of the way
             */
            if (true /* --addnodes */) {
                points += points.empty()? "" : ",";
                points += get_point(n);
            }

            /*
             * Count nodes to detect where to split
             */
            if (highway) {
                m_node_count[n.ref()]++;
            }
        }

        if (true /* --addnodes */) {
            std::string geom = "SRID=4326;LINESTRING(" + points + ")";
            auto data = std::make_tuple(way.id(), nameptr, geom);

            m_way_stream.write_values(data);

            /*
             * Use this code for saving into a file
             */
            std::cout << way.id() << "\t" << get_name(nameptr) << "\t" << geom << "\n";
        }
    }

  private:
    bool nfirst = true;
    bool wfirst = true;
    std::map<int64_t, size_t> &m_node_count;

    /** The postgres connection */
    pqxx::connection m_node_conn;
    pqxx::work m_node_action;
    pqxx::stream_to m_node_stream;
    pqxx::connection m_way_conn;
    pqxx::work m_way_action;
    pqxx::stream_to m_way_stream;

};

class SplitWays : public osmium::handler::Handler {

  public:
    SplitWays() = delete;
    SplitWays(
            std::map<int64_t, size_t> &node_count,
            std::map<int64_t, std::string> &vertices,
            std::string connInfo) :
        m_node_count(node_count),
        m_vertices(vertices),
        m_edge_conn(connInfo),
        m_edge_action(m_edge_conn),
        m_edge_stream(pqxx::stream_to::table(m_edge_action, {"edges"}, {"osm_id", "osm_source", "osm_target", "name", "geom"})){
        };

    void after_split() {
        /*
         * Finalize the COPY operation
         */
        m_edge_stream.complete();
        m_edge_action.commit();
        std::clog << "Completing the split stream\n";
        std::cout << copy_end;
    }

    void way(const osmium::Way& way) {
        const osmium::TagList& tags = way.tags();
        const char* nameptr = tags["name"];
        const char* highway = way.tags()["highway"];
        if (!highway) return;

        if (wfirst) {
            std::cout << "COPY edges (osm_id, osm_source, osm_target, name, geom) FROM stdin WITH DELIMITER '\t' NULL 'NULL' CSV;\n";
            wfirst = false;
        }


        bool newBroken = true;
        std::string points = "";
        std::string point = "";
        size_t psize = 0;
        auto source = way.nodes().front().ref();
        osmium::object_id_type target(0);


        for (const auto& n : way.nodes()) {
            /*
             * the point's coordinates
             */
            point = get_point(n);

            /*
             * add to the list of points
             */
            points += points.empty()? "" : ",";
            points += point;
            ++psize;

            /*
             * When its the first node of the way:
             * - newBroken == true
             * - psize == 1
             * Regardless if it is in node_count or not:
             * - continue
             * postcond:
             * - newBroken == false
             */
            if (newBroken && psize == 1 ) {
                newBroken = false;
                /* vertices table contains the first node of the segment */
                m_vertices[n.ref()] = "SRID=4326;POINT(" + get_point(n) +")";
                continue;
            }

            if (!(m_node_count.find(n.ref()) == m_node_count.end())) {
                /*
                 * found a node in the node_count list
                 * - INSERT because it's the last node of the edge
                 * - Add to the vertices table
                 */
                target = n.ref();
                m_vertices[target] = "SRID=4326;POINT(" + get_point(n) +")";

                std::string geom = "SRID=4326;LINESTRING(" + points + ")";
                auto data = std::make_tuple(way.id(), source, target, nameptr, geom);
                m_edge_stream.write_values(data);

                /*
                 * Use this code for saving into a file
                 */
                std::cout << way.id() << "\t" << source << "\t" << target << "\t"
                 << get_name(nameptr) << "\t" << geom << "\n";

                /*
                 * It's the beginning of the next edge
                 */
                source = target;
                points = point;
                psize = 1;
            }
        }

        /*
         * When its the last node of the way:
         * - INSERT WHEN psize != 1: because it has more than one node
         * - Add to the vertices table
         */
        if (psize != 1) {
            std::string geom = "SRID=4326;LINESTRING(" + points + ")";
            auto n = way.nodes().back();
            target = n.ref();

            auto data = std::make_tuple(way.id(), source, target, nameptr, geom);

            m_edge_stream.write_values(data);

            /*
             * Use this code for saving into a file
             */
            std::cout << way.id() << "\t" << source << "\t" << target << "\t"
                << get_name(nameptr) << "\t" << geom << "\n";

            /*
             * The last node of the way goes to the vertices table
             */
            m_vertices[target] = "SRID=4326;POINT(" + get_point(n) +")";
        }
    }

  private:
    bool wfirst = true;
    std::map<int64_t, size_t> &m_node_count;
    std::map<int64_t, std::string> &m_vertices;

    /** The postgres connection */
    pqxx::connection m_edge_conn;
    pqxx::work m_edge_action;
    pqxx::stream_to m_edge_stream;
};

int main(int argc, char *argv[]) {
    try {
        /*
         * get the arguments
         */
        po::options_description od_desc("Allowed options");
        get_option_description(od_desc);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).
                options(od_desc).run(), vm);

        if (vm.count("help")) {
            std::cout << od_desc << "\n";
            return 0;
        }

        if (vm.count("version")) {
            std::cout << "This is osm2pgrouting Version 3.1.0\n";
            return 0;
        }

        try {
            notify(vm);
        }

        catch(std::exception &ex) {
            std::cout << ex.what() << "\n";
            std::cout << od_desc << "\n";
            return 0;
        }


        /*
         *  the input file
         */
        auto in_file_name(vm["file"].as<std::string>());

        /*
         * Connection to database
         */
        handle_pgpass(vm);
        std::string connection_str(
                "host=" + vm["host"].as<std::string>()
                + " user=" +  vm["username"].as<std::string>()
                + " dbname=" + vm["dbname"].as<std::string>()
                + " port=" + vm["port"].as<std::string>()
                + " password=" + vm["password"].as<std::string>());

        std::clog << "Conecting to datatabase: " << vm["dbname"].as<std::string>() << std::endl;
        pqxx::connection dbconn(connection_str);
        if (dbconn.is_open()) {
            std::clog << "database connection successful: " << dbconn.dbname() << std::endl;
        } else {
            std::cerr << "Can't open database" << std::endl;
            return 1;
        }


        /*
         * Drop & Create the necessary tables
         */

        /*
         * start a transaction
         */
        pqxx::work create_tables(dbconn);
        std::string sql = "DROP TABLE IF EXISTS new_osm_nodes";
        create_tables.exec(sql);
        sql = "DROP TABLE IF EXISTS new_osm_ways";
        create_tables.exec(sql);
        sql = "DROP TABLE IF EXISTS edges";
        create_tables.exec(sql);
        sql = "DROP TABLE IF EXISTS vertices";
        create_tables.exec(sql);
        sql = "CREATE TABLE IF NOT EXISTS new_osm_nodes("
            "id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,"
            "osm_id BIGINT,"
            "name TEXT,"
            "geom GEOMETRY(POINT, 4326));";
        create_tables.exec(sql);
        sql = "CREATE TABLE IF NOT EXISTS new_osm_ways("
            "id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,"
            "osm_id BIGINT,"
            "name TEXT,"
            "geom GEOMETRY(LINESTRING, 4326));";
        create_tables.exec(sql);
        sql = "CREATE TABLE IF NOT EXISTS edges("
            "id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,"
            "source BIGINT,"
            "target BIGINT,"
            "length DOUBLE PRECISION GENERATED ALWAYS AS (ST_length(geom::geography)) STORED,"
            "x1 numeric(11,8) GENERATED ALWAYS AS (ST_X(ST_StartPoint(geom))) STORED,"
            "y1 numeric(11,8) GENERATED ALWAYS AS (ST_Y(ST_StartPoint(geom))) STORED,"
            "x2 numeric(11,8) GENERATED ALWAYS AS (ST_X(ST_EndPoint(geom))) STORED,"
            "y2 numeric(11,8) GENERATED ALWAYS AS (ST_Y(ST_EndPoint(geom))) STORED,"
            "osm_id BIGINT,"
            "osm_source BIGINT,"
            "osm_target BIGINT,"
            "name TEXT,"
            "geom GEOMETRY(LINESTRING, 4326));";
        create_tables.exec(sql);
        sql = "CREATE TABLE IF NOT EXISTS vertices("
            "id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,"
            "in_edges BIGINT[],"
            "out_edges BIGINT[],"
            "x numeric(11,8) GENERATED ALWAYS AS (ST_X(geom)) STORED,"
            "y numeric(11,8) GENERATED ALWAYS AS (ST_Y(geom)) STORED,"
            "osm_id BIGINT,"
            "geom GEOMETRY(POINT, 4326) NOT NULL);";
        create_tables.exec(sql);
        create_tables.commit();


        /*
         * Create the osmium reader
         */
        auto otypes = osmium::osm_entity_bits::node | osmium::osm_entity_bits::way;
        osmium::io::Reader reader{in_file_name, otypes};

        using index_type = osmium::index::map::SparseMemArray<osmium::unsigned_object_id_type, osmium::Location>;
        using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

        index_type index;
        location_handler_type location_handler{index};

        /*
         * storage for the node counting
         */
        std::map<int64_t, size_t> node_count;

        /*
         * Create the handler
         */
        NodeCount node_count_handler(node_count, connection_str);

        // Apply the handler to the reader
        std::clog << "Starting OSM node count processing..." << std::endl;
        osmium::apply(reader, location_handler, node_count_handler);
        reader.close();
        std::clog << "Finished OSM node count processing." << std::endl;
        node_count_handler.after_ways();


        /* nodes that are in more than one way */
        std::clog << "node count: " << node_count.size() << "\n";
        for (auto it = node_count.cbegin(); it != node_count.cend(); ) {
            if (it->second == 1)  {
                node_count.erase(it++);
            } else {
                ++it;
            }
        }
        std::clog << "new node count: " << node_count.size() << "\n";

        /*
         * storage for the vertices (start and end vertices of a linestring)
         */
        std::map<int64_t, std::string> vertices;

        SplitWays split_handler(node_count, vertices, connection_str);
        osmium::io::Reader reader2{in_file_name, otypes};
        osmium::apply(reader2, location_handler, split_handler);
        reader.close();
        split_handler.after_split();

        /*
         * print the vertices table
         */
        pqxx::work vertices_action(dbconn);
        pqxx::stream_to vertices_stream(pqxx::stream_to::table(vertices_action, {"vertices"}, {"osm_id", "geom"}));

        std::cout << "COPY vertices (osm_id, geom) FROM stdin WITH DELIMITER '\t' NULL 'NULL' CSV;\n";
        for (const auto v : vertices) {
            auto data = std::make_tuple(v.first, v.second);

            vertices_stream.write_values(data);
            std::cout << v.first << "\t" << v.second << "\n";
        }

        /*
         * end of ways_vertices_pgr COPY
         */
        vertices_stream.complete();
        vertices_action.commit();

        std::cout << copy_end;

        pqxx::work updates(dbconn);
        sql = "UPDATE edges set source = v.id FROM (SELECT osm_id, id FROM vertices) AS v WHERE osm_source = v.osm_id;";
        sql += "UPDATE edges set target = v.id FROM (SELECT osm_id, id FROM vertices) AS v WHERE osm_target = v.osm_id;";
        sql += "UPDATE vertices set out_edges = e.arr FROM (SELECT source, array_agg(id) AS arr FROM edges group by source) AS e WHERE id = source;";
        sql += "UPDATE vertices set in_edges = e.arr FROM (SELECT target, array_agg(id) AS arr FROM edges group by target) AS e WHERE id = target;";
        updates.exec(sql);
        updates.commit();
    }

    catch (const std::exception &e){
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (std::string &e) {
        std::cout << e << std::endl;
        return 1;
    }
    catch (...) {
        std::cout << "Terminating" << std::endl;
        return 1;
    }
}
