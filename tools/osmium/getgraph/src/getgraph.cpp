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


std::map<int64_t, size_t> node_count;
std::map<int64_t, std::string> vertices;

class NodeCount : public osmium::handler::Handler {
  public:
    void node(const osmium::Node& node) {
      const osmium::TagList& tags = node.tags();
      const char* highway = tags["highway"];
      const char* nameptr = tags["name"];

#ifdef ADDNODES
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
#endif

      if (!highway) return;
      if (!node.tags().empty()) node_count[node.id()]++;
    }


    void way(const osmium::Way& way) {
      const osmium::TagList& tags = way.tags();
      const char* nameptr = tags["name"];
      std::string name = nameptr? std::string("'") + nameptr + std::string("'") : "NULL";
      const char* highway = way.tags()["highway"];

#ifdef ADDNODES
      if (wfirst) {
        std::cout << "\\.\n";
        std::cout << "COPY osm_ways (osm_id, name, geom) FROM stdin WITH DELIMITER '\t' NULL 'NULL' CSV;\n";
        wfirst = false;
      }
#endif

      std::string points = "";
      for (const auto &n : way.nodes()) {
#ifdef ADDNODES
        /*
         * get the points of the way
         */
        if (true /* --addnodes */) {
          points += points.empty()? "" : ",";
          points += std::to_string(n.lon()) + " " + std::to_string(n.lat());
        }
#endif

        /*
         * Count nodes to detect where to split
         */
        if (highway) {
          node_count[n.ref()]++;
        }
      }

#ifdef ADDNODES
      if (true /* --addnodes */) {
        std::cout << way.id() << "\t" << name
          << "\t\"LINESTRING(" << points << ")\"\n";
      }
#endif
    }
  private:
    bool nfirst = true;
    bool wfirst = true;
};

class Wayid_NodeLocationsofWays : public osmium::handler::Handler {
  bool wfirst = true;
  std::string get_point(const osmium::NodeRef& n) {
          std::ostringstream oss;
          oss  << std::setprecision(15) << "\"POINT(" << n.lon() << " " << n.lat() << ")\"";
          return oss.str();
  }

  public:
    void way(const osmium::Way& way) {
      const osmium::TagList& tags = way.tags();
      const char* nameptr = tags["name"];
      std::string name = nameptr? nameptr : "NULL";
      const char* highway = way.tags()["highway"];
      if (!highway) return;

      if (wfirst) {
        std::cout << "\\.\n";
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
#if 0
        std::cout << "NEXT: " << points << "\n";
#endif

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
#if 0
          std::cout << "newBroken && psize == 1: " << points << "\n";
#endif
          newBroken = false;
          /* vertices table contains the first node of the segment */
          vertices[n.ref()] = get_point(n);
          continue;
        }

        if (!(node_count.find(n.ref()) == node_count.end())) {
          /*
           * found a node in the node_count list
           * - INSERT because it's the last node of the edge
           * - Add to the vertices table
           */
          vertices[n.ref()] = get_point(n);
#if 0
          std::cout << "INSERT because it's the last node of the edge\n" << points << "\n";
#endif
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
        vertices[n.ref()] = get_point(n);
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

#ifdef ADDNODES
  /*
   * This is the end of the osm_ways COPY
   */
  if (true /* --addnodes */) {
    std::cout << "\\.\n";
  }
#endif

  /* nodes that are in more than one way */
#if 0
  std::cout << "total nodes: " << node_count.size();
#endif

  for (auto it = node_count.cbegin(); it != node_count.cend(); ) {
    if (it->second == 1)  {
      node_count.erase(it++);
    } else {
      ++it;
    }
  }
#if 0
  std::cout << "nodes in more than one way: " << node_count.size();
#endif

  osmium::io::Reader reader2{in_file_name, otypes};
  osmium::apply(reader2, location_handler, way_location_handler);
  reader.close();

  /* print the vertices table */
  std::cout << "COPY ways_vertices_pgr (osm_id, geom) FROM stdin WITH DELIMITER '\t' NULL 'NULL' CSV;\n";
  for (const auto v : vertices) {
    std::cout <<  std::setprecision (15)
      << v.first << "\t" << v.second << "\n";
  }
  std::cout << "\\.\n";
}
