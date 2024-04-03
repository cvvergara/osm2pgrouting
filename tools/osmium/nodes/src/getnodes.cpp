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
#if 0
    /* this check shoud match the split checks */
    if (!way.tags().has_key("highway")) return;
#endif
    for (const auto& node : way.nodes())
      nodeSet[node.ref()]++;
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

  void copy_nodes(
      osmium::memory::Buffer& buffer,
      osmium::builder::WayBuilder& builder,
      const osmium::WayNodeList& nodes,
      const std::vector<osmium::object_id_type>& nodes_to_copy) {
      osmium::builder::WayNodeListBuilder wnl_builder{buffer, &builder};
      // Copy the node list to the new way.
      // use front to start copying and back to end the copy
      bool found_front{false};
      size_t front = 0;
      for (std::size_t i = 0; i < nodes.size() ; ++i) {
        /* Note the node */
        if (!found_front && nodes[i].ref()!= nodes_to_copy[front]) continue;
        /* The next node is not the wanted one */
        if (!found_front && (i + 1 < nodes.size()) && nodes[i + 1].ref() != nodes_to_copy[front + 1]) continue;
        /* Node belongs to the segment */
        osmium::NodeRef nr(nodes[i].ref(), nodes[i].location());
        wnl_builder.add_node_ref(nr);
        if (found_front && nodes[i].ref() == nodes_to_copy.back()) break;
        /* Placing this assignment at the end
         * takes care of circular ways like by creating a loop.
         * 250078288,656832218,656081895,250078290,656081899,250078288
         * TODO further break up the loop
         * 250078288,656832218,656081895,250078290,656081899,
         * 656081899,250078288
         */
        found_front = true;
        ++front;
      }
  }

  void copy_way(
      osmium::memory::Buffer& buffer,
      const osmium::Way& way,
      const std::vector<osmium::object_id_type>& nodes_to_copy) {
    /* the way builder */
    osmium::builder::WayBuilder builder{buffer};

    /* keep atrributes and tags */
    copy_attributes(builder, way);
    copy_tags(builder, way.tags());
    copy_nodes(buffer, builder, way.nodes(), nodes_to_copy);
  }

  void split_way(const osmium::Way& way) {
    bool first{true};
    bool last{false};
    std::vector<std::vector<osmium::object_id_type>> segments;
    std::vector<osmium::object_id_type> segment;
    for (const auto& n : way.nodes()) {
      if (first) {
        first = false;
        segment.push_back(n.ref());
      } else if (nodeSet[n.ref()] == 1) {
        segment.push_back(n.ref());
      } else if (nodeSet[n.ref()] > 1) {
        last = true;
        segment.push_back(n.ref());
      }
      if (last) {
        segments.push_back(segment);
        segment.clear();
        last = false;
        segment.push_back(n.ref());
      }
    }
    if (!last) {
      /* when the last node is a dead end */
      if (segment.size() > 1) segments.push_back(segment);
    }

    std::cout << "\noriginal way: ";
    for (const auto& n : way.nodes()) {
      std::cout << n.ref() << ",";
    }
    for (const auto &s : segments) {
      const int buffer_size = 10240;

      /* buffer for the new way auto grows */
      osmium::memory::Buffer way_buffer{buffer_size, osmium::memory::Buffer::auto_grow::yes};

      /* copy the way into the buffer and commit the buffer*/
      copy_way(way_buffer, way, s);
      way_buffer.commit();

      osmium::Way& new_way = way_buffer.get<osmium::Way>(0);

      std::cout << "\n     segment: ";
      for (const auto& n : new_way.nodes()) {
        std::cout << n.ref() << ",";
      }
    }
    std::cout << "\n\n";
  }

  public:
  // Constructor. New data will be added to the given buffer.
  explicit BreakLines(osmium::memory::Buffer& buffer) :
    m_buffer(buffer) {
    }

  void way(const osmium::Way& way) {
#if 0
    /* this check shoud match the node count */
    if (!way.tags().has_key("highway")) return;
#endif
    split_way(way);
  }

#if 0
    const int buffer_size = 10240;

    /* buffer for the new way auto grows */
    osmium::memory::Buffer way_buffer{buffer_size, osmium::memory::Buffer::auto_grow::yes};

    /* copy the way into the buffer and commit the buffer*/
    copy_way(way_buffer, way);
    way_buffer.commit();

    osmium::Way& new_way = way_buffer.get<osmium::Way>(0);

    std::cout << "original way " << way.id() << ": ";
    for (const auto& n : way.nodes()) {
      std::cout << n.ref() << ",";
    }
    std::cout << "\n";

    std::cout << "     new way " << way.id() << ": ";
    for (const auto& n : new_way.nodes()) {
      std::cout << n.ref() << ",";
    }
    std::cout << "\n";
#endif
#if 0
  {
    osmium::builder::WayBuilder builder{m_buffer};
    copy_attributes(builder, way);
    copy_tags(builder, way.tags());

    {
      osmium::builder::WayNodeListBuilder wnl_builder{m_buffer, &builder};
      // Copy the node list over to the new way.
      for (const auto& n : way.nodes()) {
        osmium::NodeRef nr(n.ref(), n.location());
        wnl_builder.add_node_ref(nr);
      }
    }

    builder.add_item(way.nodes());
  }
  m_buffer.commit();

  osmium::Way& new_way = m_buffer.get<osmium::Way>(0);

  std::cout << "original way " << way.id() << ": ";
  for (const auto& n : way.nodes()) {
    std::cout << n.ref() << ",";
  }
  std::cout << "\n";

  std::cout << "     new way " << way.id() << ": ";
  for (const auto& n : new_way.nodes()) {
    std::cout << n.ref() << ",";
  }
  std::cout << "\n";
#endif
#if 0

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
    } else if (nodeSet[n.ref()] == 1) {
      std::cout << n.ref() << ",";
    } else if (nodeSet[n.ref()] > 1) {
      last = true;
      std::cout << n.ref() << "\n";
    }
    if (last) {
      last = false;
      std::cout << "first:" << n.ref() << ",";
    }
  }
  std::cout << "\n\n";
}
#endif
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

