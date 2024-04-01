#include <iostream>
#include <string>

#include <osmium/handler.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/visitor.hpp>
#include <osmium/geom/factory.hpp>
#include <osmium/geom/wkt.hpp>

#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>

class MyHandler : public osmium::handler::Handler {
public:
  void way(const osmium::Way& way) {

    if (!way.tags().has_key("highway")) return;

    osmium::geom::WKTFactory<> factory;
    auto wkt = factory.create_linestring(way);

    std::cout << "way " << way.id() << "wkt" << wkt << '\n';

    for (const osmium::Tag& t : way.tags()) {
      std::cout << t.key() << "=" << t.value() << '\n';
    }
    for (const auto& n : way.nodes()) {
      osmium::geom::WKTFactory<> factory;
      auto wkt = factory.create_point(n);
      std::cout << n.ref() << ": " << n.lon() << ", " << n.lat() << " wkt "<< wkt << '\n';
    }
  }

#if 0
  void node(const osmium::Node& node) {
    std::cout << "node " << node.id() << ": " << node.location().lat() << "," <<node.location().lon() << '\t';
    for (const osmium::Tag& t : node.tags()) {
      std::cout << t.key() << "=" << t.value() << '\t';
    }
    std::cout << "\n";
  }
#endif
};

int main(int argc, char *argv[]) {
  /* get the arguments */
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

  MyHandler handler;
  osmium::apply(reader, location_handler, handler);
  reader.close();
}

