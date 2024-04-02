#include <iostream>
#include <string>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <new>
#include <map>


#include <osmium/builder/builder.hpp>
#include <osmium/memory/item.hpp>
#include <osmium/osm/area.hpp>
#include <osmium/osm/box.hpp>
#include <osmium/osm/changeset.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/node_ref.hpp>
#include <osmium/osm/object.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/osm/tag.hpp>
#include <osmium/osm/timestamp.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/handler.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/visitor.hpp>
#include <osmium/osm/tag.hpp>
#include <osmium/geom/factory.hpp>
#include <osmium/geom/wkt.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>

std::map<osmium::unsigned_object_id_type, int> nodeSet;

struct NodeCounter : public osmium::handler::Handler
{

  void way(const osmium::Way& way)
  {
    for (const auto& node : way.nodes())
      nodeSet[node.positive_ref()]++;
  }
};

class BreakLines : public osmium::handler::Handler {
  osmium::memory::Buffer& m_buffer;

  // Copy attributes common to all OSM objects (nodes, ways, and relations).
  template <typename T>
    void copy_attributes(T& builder, const osmium::OSMObject& object) {
      // The setter functions on the builder object all return the same
      // builder object so they can be chained.
      builder.set_id(object.id())
        .set_version(object.version())
        .set_changeset(object.changeset())
        .set_timestamp(object.timestamp())
        .set_uid(object.uid())
        .set_user(object.user());
    }

  // Copy all tags with two changes:
  // * Do not copy "created_by" tags
  // * Change "landuse=forest" into "natural=wood"
  static void copy_tags(osmium::builder::Builder& parent, const osmium::TagList& tags) {

    // The TagListBuilder is used to create a list of tags. The parameter
    // to create it is a reference to the builder of the object that
    // should have those tags.
    osmium::builder::TagListBuilder builder{parent};

    // Iterate over all tags and build new tags using the new builder
    // based on the old ones.
    for (const auto& tag : tags) {
        builder.add_tag(tag);
    }
  }

  public:
  // Constructor. New data will be added to the given buffer.
  explicit BreakLines(osmium::memory::Buffer& buffer) :
    m_buffer(buffer) {
    }

  void way(const osmium::Way& way) {
#if 0
    {
      osmium::builder::WayBuilder builder{m_buffer};
      copy_attributes(builder, way);
      copy_tags(builder, way.tags());

      // Copy the node list over to the new way.
      builder.add_item(way.nodes());
    }
    m_buffer.commit();

    osmium::Way& new_way = m_buffer.get<osmium::Way>(0);
#endif
#if 1

    if (!way.tags().has_key("highway")) return;

    osmium::geom::WKTFactory<> factory;
    auto wkt = factory.create_linestring(way);

    std::cout << "original way " << way.id() << ": ";
    for (const auto& n : way.nodes()) {
      std::cout << n.ref() << ",";
    }
    std::cout << "\n";
    bool first{true};
    bool last{false};
    for (const auto& n : way.nodes()) {
      osmium::geom::WKTFactory<> factory;
      auto wkt = factory.create_point(n);
      if (first) {
        first = false;
        std::cout << "first:" << n.ref() << ",";
      } else if (nodeSet[n.positive_ref()] == 1) {
        std::cout << n.ref() << ",";
      } else if (nodeSet[n.positive_ref()] > 1) {
        last = true;
        std::cout << n.ref() << "\n";
      }
      if (last) {
        last = false;
        std::cout << "first:" << n.ref() << ",";
      }
    }
    std::cout << "\n\n";
#endif
  }
};

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

#if 0
  MyHandler handler;
  osmium::apply(reader, location_handler, handler);
#endif

  osmium::memory::Buffer buffer(10240);

  NodeCounter nchandler;
  BreakLines breaklines(buffer);
  osmium::apply(reader, location_handler, nchandler, breaklines);
  for (const auto &e : nodeSet) {
    if (e.second == 1) continue;
    std::cout << e.first << " " << e.second << "\n";
  }

  reader.close();
}

