#include <iostream>
#include <iomanip>

#include <osmium/handler.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/visitor.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>


class Wayid_NodeLocationsofWays : public osmium::handler::Handler {
public:
    void way(const osmium::Way& way) {
        std::cout << "way " << way.id() << '\n';
        for (const auto& n : way.nodes()) {
            std::cout << std::setprecision (15) << n.ref() << ": " << n.lon() << ", " << n.lat() << '\n';
            node_count[n.ref()]++;
        }
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



    namespace map = osmium::index::map;
    using index_type = map::SparseMemArray<osmium::unsigned_object_id_type, osmium::Location>;
    using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

    index_type index;
    location_handler_type location_handler{index};

    Wayid_NodeLocationsofWays handler;
    osmium::apply(reader, location_handler, handler);
    reader.close();
}
