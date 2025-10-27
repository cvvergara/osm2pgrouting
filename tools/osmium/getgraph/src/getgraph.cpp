#include <iostream>

#include <osmium/handler.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/visitor.hpp>

class Nid_Widtags : public osmium::handler::Handler {
public:
    void way(const osmium::Way& way) {
        std::cout << "way " << way.id() << '\n';
        for (const osmium::Tag& t : way.tags()) {
            std::cout << t.key() << "=" << t.value() << '\n';
        }
    }

    void node(const osmium::Node& node) {
        std::cout << "node " << node.id() << '\n';
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


  Nid_Widtags handler;
  osmium::apply(reader, handler);
  reader.close();
}
