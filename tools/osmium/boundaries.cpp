#include <iostream> 
#include <osmium/io/pbf_input.hpp> // used to import osm files
#include <osmium/io/pbf_output.hpp>
#include <osmium/geom/wkt.hpp>
#include <osmium/relations/collector.hpp>
#include <osmium/relations/relations_manager.hpp>
#include <osmium/relations/members_database.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/visitor.hpp>
#include <osmium/handler.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>

//#include <osmium/io/xml_input.hpp>

int count = 0;


using index_type = osmium::index::map::SparseMemArray<osmium::unsigned_object_id_type, osmium::Location>;
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

//class BoundaryCollector : public osmium::relations::Collector<BoundaryCollector, true, true, true> {
class BoundaryManager : public osmium::relations::RelationsManager<BoundaryManager, true, true, true> {
	/**
     * Interested in all relations tagged with type=boundary
     *
     * OSM WIKI about boundaries
     * http://wiki.openstreetmap.org/wiki/Key:boundary
     *
     * Overwritten from the base class.
     */	
public:

	bool new_relation(const osmium::Relation& relation) noexcept {
		//std::cout << relation.tags().get_value_by_key("type") << std::endl;
		/*if (relation.tags().has_tag("type", "boundary")) {
			std::cout << "new_relation: boundary" << std::endl;
		}*/
		return relation.tags().has_tag("type", "boundary");
	}


	bool new_member(const osmium::Relation& relation, const osmium::RelationMember& member, std::size_t n) noexcept {
		return member.type() == osmium::item_type::way || 
		member.type() == osmium::item_type::node;
	}

	void complete_relation(osmium::Relation& relation) {
    // Iterate over all members
		//std::cout << relation.tags().get_value_by_key("boundary") << std::endl;

		if (relation.tags().has_tag("boundary", "administrative")) {
			for (const auto& member : relation.members()) {
        	// member.ref() will be 0 for all members you are not interested
        	// in. The objects for those members are not available.
				if (member.ref() != 0) {
					//const osmium::Node* node = this->get_member_node(member.ref());
					const osmium::OSMObject* obj = this->member_database(member.type()).get_object(member.ref());
					if (obj != NULL) {
						this->buffer().add_item(*obj);
						this->buffer().commit();
						//std::cout << "administrative" << std::endl;
					}
				}
			}
		}
	}

};




int main(int argc, char const *argv[])
{
	index_type index;
	location_handler_type location_handler{index};
	location_handler.ignore_errors();


	auto otypes = osmium::osm_entity_bits::relation | osmium::osm_entity_bits::way;
	std::string osm_file_path = "/Users/rohithreddy/Downloads/sample.osm.pbf";
	osmium::io::File input_file{osm_file_path};
	BoundaryManager manager;
	std::cerr << "Pass 1...\n";
	osmium::relations::read_relations(input_file, manager);
	std::cout << "Done reading" << std::endl;
	std::cerr << "Pass 1 done\n";


	std::cerr << "Pass 2...\n";
	osmium::io::Reader reader{input_file};
	osmium::apply(reader, manager.handler());
	
    // Access data in output buffer
    osmium::memory::Buffer buffer;
    buffer = manager.read();
	std::cerr << "Pass 2 done\n";
	/*
	std::cerr << "Pass 2...\n";
	osmium::io::Reader reader2{osm_file_path, otypes};
	osmium::apply(reader2, location_handler, collector.handler([&handler](osmium::memory::Buffer&& buffer) {
		osmium::apply(buffer, handler);
	}));
	reader2.close();
	std::cout << "\\.";
	std::cerr << "Pass 2 done\n";
	*/
	return 0;
}