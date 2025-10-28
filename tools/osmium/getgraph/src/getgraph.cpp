#include <iostream>
#include <iomanip>

#include <osmium/handler.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/visitor.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>

std::map<int64_t, size_t> node_count;

class NodeCount : public osmium::handler::Handler {
  public:
    void node(const osmium::Node& node) {

      const osmium::TagList& tags = node.tags();
      if (true /* --addnodes */) {

        /* hint on how to add the tags of the nodes
           UPDATE tab SET h = h || hstore(array['q', 'w'], array['11', '12']);
           */

        const osmium::Location& location = node.location();
        if (location.valid()) {

          const char* name = tags["name"];
          if (name) {
            std::cout <<  std::setprecision (15) << "INSERT INTO osm_nodes (osm_id, name, geom) VALUES ("
              << node.id() << ", '" << name << "'"
              << ", ST_SetSRID(ST_Point(" << location.lon() << "," << location.lat() << "), 4326));\n";
          } else {
            std::cout <<  std::setprecision (15) << "INSERT INTO osm_nodes (osm_id, geom) VALUES ("
              << node.id()
              << ", ST_SetSRID(ST_Point(" << location.lon() << "," << location.lat() << "), 4326));\n";
          }
        } else {
          std::cout << "Found invalid location at " << node.id() << "\n";
        }
      }

      const char* highway = tags["highway"];
      if (!highway) return;
      if (!node.tags().empty()) node_count[node.id()]++;
    }

    void way(const osmium::Way& way) {
      const osmium::TagList& tags = way.tags();

      for (const auto &n : way.nodes()) {

        /*
         * fill up osm_ways
         */
        if (true /* --addnodes */) {
          std::string points;
          for (const auto& n : way.nodes()) {
            points += points.empty()? "" : ",";
            points += "ST_Point(" + std::to_string(n.lon()) + "," + std::to_string(n.lat()) + ')';
          }

          const char* name = tags["name"];
          if (name) {
            std::cout << "INSERT INTO osm_ways (osm_id, name, geom) VALUES ("
              << way.id() << ", '" << name << "'"
              << ", ST_SetSRID(ST_MakeLine(ARRAY[" << points << "]), 4326));\n";
          } else {
            std::cout << "INSERT INTO osm_ways (osm_id, geom) VALUES ("
              << way.id()
              << ", ST_SetSRID(ST_MakeLine(" << points << "), 4326));\n";
          }
        }

        /*
         * Count nodes to detect where to split
         */
        const char* highway = way.tags()["highway"];
        if (highway) {
          node_count[n.ref()]++;
        }
      }
    }
};

class Wayid_NodeLocationsofWays : public osmium::handler::Handler {
  public:
    void way(const osmium::Way& way) {
      std::cout << "\nway " << way.id() << '\n';
      for (const auto& n : way.nodes()) {
        std::cout << n.ref() << ", ";
      }
      std::cout << "\n";

      for (const auto& n : way.nodes()) {
        const char* highway = way.tags()["highway"];
        if (!highway) continue;
        if (node_count.find(n.ref()) == node_count.end()) {
          /*
           * not found
           */
          std::cout << n.ref() << ", ";
        } else {
          /*
           * found
           */
          if (n.ref() != way.nodes().begin()->ref()) std::cout << n.ref() << "\n";
          std::cout << n.ref() << ", ";
        }
#if 0
        std::cout << std::setprecision (15) << n.ref() << ": " << n.lon() << ", " << n.lat() << '\n';
#endif
      }
      std::cout << "\n----\n";
    }
};

int main(int argc, char *argv[]) {
  /*
   * get the arguments
   */
  if (argc != 2) {
    std::cerr << "file to process missing\n";
    exit(1);
  }

  /*
   *  the input file
   */
  std::string in_file_name = argv[1];
  std::cout << "processing: " << in_file_name << "\n";

  auto otypes = osmium::osm_entity_bits::node | osmium::osm_entity_bits::way;
  osmium::io::Reader reader{in_file_name, otypes};


  using index_type = osmium::index::map::SparseMemArray<osmium::unsigned_object_id_type, osmium::Location>;
  using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

  index_type index;
  location_handler_type location_handler{index};
  NodeCount node_count_handler;
  Wayid_NodeLocationsofWays way_location_handler;

  osmium::apply(reader, location_handler, node_count_handler);
  reader.close();

  /* nodes that are in more than one way */
  std::cout << "total nodes: " << node_count.size();
  for (auto it = node_count.cbegin(); it != node_count.cend(); ) {
    if (it->second == 1)  {
      node_count.erase(it++);
    } else {
      ++it;
    }
  }
  std::cout << "nodes in more than one way: " << node_count.size();


  osmium::io::Reader reader2{in_file_name, otypes};
  osmium::apply(reader2, location_handler, way_location_handler);
  reader.close();
}
