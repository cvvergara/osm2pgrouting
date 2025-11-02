#include <iostream>
#include <iomanip>
#include <map>
#include <set>

#include <osmium/handler.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/visitor.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>


#include "options.hpp"

namespace {

const std::string copy_end = "\\.\n";

}

class NodeCount : public osmium::handler::Handler {

  public:
    explicit NodeCount(std::map<int64_t, size_t> &node_count) :
        m_node_count(node_count) {};
    NodeCount() = delete;

    void node(const osmium::Node& node) {
      const osmium::TagList& tags = node.tags();
      const char* highway = tags["highway"];
      const char* nameptr = tags["name"];

      if (nfirst) {
        std::cout << "COPY osm_nodes (osm_id, name, geom) FROM stdin WITH DELIMITER '\t' NULL 'NULL' CSV;\n";
        nfirst = false;
      }
      if (true /* --addnodes */) {

        /* hint on how to add the tags of the nodes
           UPDATE tab SET h = h || hstore(array['q', 'w'], array['11', '12']);
           */

        const osmium::Location& location = node.location();
        if (location.valid()) {

          std::string name = nameptr? std::string("'") + nameptr + std::string("'") : "NULL";
          std::cout <<  std::setprecision (15)
            << node.id() << "\t" << name
            << "\t\"POINT(" << location.lon() << " " << location.lat() << ")\"\n";
        } else {
          std::cout << "Found invalid location at " << node.id() << "\n";
        }
      }

      if (!highway) return;
      if (!node.tags().empty()) m_node_count[node.id()]++;
    }


    void way(const osmium::Way& way) {
      const osmium::TagList& tags = way.tags();
      const char* nameptr = tags["name"];
      std::string name = nameptr? std::string("'") + nameptr + std::string("'") : "NULL";
      const char* highway = way.tags()["highway"];

      if (wfirst) {
        /*
         * end of osm_nodes COPY
         */
        std::cout << copy_end;
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
          points += std::to_string(n.lon()) + " " + std::to_string(n.lat());
        }

        /*
         * Count nodes to detect where to split
         */
        if (highway) {
          m_node_count[n.ref()]++;
        }
      }

      if (true /* --addnodes */) {
        std::cout << way.id() << "\t" << name
          << "\t\"LINESTRING(" << points << ")\"\n";
      }
    }

    std::map<int64_t, size_t>& node_count() {return m_node_count;}

  private:
    bool nfirst = true;
    bool wfirst = true;
    std::map<int64_t, size_t> &m_node_count;
};

class Wayid_NodeLocationsofWays : public osmium::handler::Handler {
  std::string get_point(const osmium::NodeRef& n) {
          std::ostringstream oss;
          oss  << std::setprecision(15) << "\"POINT(" << n.lon() << " " << n.lat() << ")\"";
          return oss.str();
  }

  public:
    explicit Wayid_NodeLocationsofWays(
            std::map<int64_t, size_t> &node_count,
            std::map<int64_t, std::string> &vertices) :
        m_node_count(node_count), m_vertices(vertices) {};
    Wayid_NodeLocationsofWays() = delete;

    void way(const osmium::Way& way) {
      const osmium::TagList& tags = way.tags();
      const char* nameptr = tags["name"];
      std::string name = nameptr? nameptr : "NULL";
      const char* highway = way.tags()["highway"];
      if (!highway) return;

      if (wfirst) {
        std::cout << "COPY ways (osm_id, name, geom) FROM stdin WITH DELIMITER '\t' NULL 'NULL' CSV;\n";
        wfirst = false;
      }

#if 0
      std::cout << "\nall way's nodes " << way.id() << '\n';
      for (const auto& n : way.nodes()) {
        std::cout << n.ref() << ", ";
      }
      std::cout << "\n";
#endif

      bool newBroken = true;
      std::string points = "";
      std::string point = "";
      size_t psize = 0;
      for (const auto& n : way.nodes()) {
        /*
         * the point's coordinates
         */
        std::ostringstream oss;
        oss  << std::setprecision(15) << n.lon() << " " << n.lat();
        point = oss.str();

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
          m_vertices[n.ref()] = get_point(n);
          continue;
        }

        if (!(m_node_count.find(n.ref()) == m_node_count.end())) {
          /*
           * found a node in the node_count list
           * - INSERT because it's the last node of the edge
           * - Add to the vertices table
           */
          m_vertices[n.ref()] = get_point(n);

          /*
           * It's the beginning of the next edge
           */
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
        std::cout << way.id() << "\t" << name << "\t\"LINESTRING(" << points << ")\"\n";
        auto n = way.nodes().back();
        m_vertices[n.ref()] = get_point(n);
      }
    }

  private:
    bool wfirst = true;
    std::map<int64_t, size_t> &m_node_count;
    std::map<int64_t, std::string> &m_vertices;
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
        std::string connection_str(
                "host=" + vm["host"].as<std::string>()
                + " user=" +  vm["username"].as<std::string>()
                + " dbname=" + vm["dbname"].as<std::string>()
                + " port=" + vm["port"].as<std::string>()
                + " password=" + vm["password"].as<std::string>());

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
        NodeCount node_count_handler(node_count);

        osmium::apply(reader, location_handler, node_count_handler);
        reader.close();

        /*
         * This is the end of the osm_ways COPY
         */
        if (true /* --addnodes */) {
            std::cout << copy_end;
        }

        /* nodes that are in more than one way */
        for (auto it = node_count.cbegin(); it != node_count.cend(); ) {
            if (it->second == 1)  {
                node_count.erase(it++);
            } else {
                ++it;
            }
        }

        /*
         * storage for the vertices (start and end vertices of a linestring)
         */
        std::map<int64_t, std::string> vertices;

        Wayid_NodeLocationsofWays way_location_handler(node_count, vertices);
        osmium::io::Reader reader2{in_file_name, otypes};
        osmium::apply(reader2, location_handler, way_location_handler);
        reader.close();

        /*
         * end of ways COPY
         */
        std::cout << copy_end;

        /* print the vertices table */
        std::cout << "COPY ways_vertices_pgr (osm_id, geom) FROM stdin WITH DELIMITER '\t' NULL 'NULL' CSV;\n";
        for (const auto v : vertices) {
            std::cout <<  std::setprecision (15)
                << v.first << "\t" << v.second << "\n";
        }

        /*
         * end of ways_vertices_pgr COPY
         */
        std::cout << copy_end;
    }

    catch (std::exception &e) {
        std::cout << e.what() << std::endl;
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
