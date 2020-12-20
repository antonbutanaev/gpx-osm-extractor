#include <iostream>
#include <map>
#include <vector>
#include <string.h>

#include <pugixml.hpp>
#include <boost/program_options.hpp>

using namespace std;

struct Coord {
	double lon,lat;
};

int main(int ac, char **av) {
	const char *argHelp = "help";
	string prefix;
	string gpxFileName;
	string trackName;
	string osmFileName;

	namespace po = boost::program_options;
	po::options_description desc("extract tracks from .osm file by prefix and write them to .gpx file");
	desc.add_options()(
	    argHelp, "print help messages")(
		"osm", po::value(&osmFileName), ".osm file name (input)")(
		"gpx", po::value(&gpxFileName)->default_value("track.gpx"), ".gpx file name (output)")(
		"prefix", po::value(&prefix)->default_value("xxx"), "track name prefix to extract")(
		"track-name", po::value(&trackName)->default_value("track extracted from .osm"), "track name in .gpx file");
	po::variables_map vm;
	po::store(po::parse_command_line(ac, av, desc), vm);

	po::notify(vm);

	if (vm.count(argHelp) || osmFileName.empty()) {
		cout << av[0] << ": " << desc << "\n";
		return 0;
	}

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(osmFileName.c_str());
    if (!result) {
    	cerr << "Could not open: " << osmFileName << endl;
        return -1;
    }

    cout << "Loaded: " << osmFileName << endl;

    map<int, Coord> nodes;
    for (const auto &item: doc.child("osm")) {
    	const auto name = item.name();
    	if (0 == strcmp(name, "node")) {
    		nodes[item.attribute("id").as_int()] = {item.attribute("lon").as_double(), item.attribute("lat").as_double()};
    	}
    }

    vector<vector<Coord>> segs;
    for (const auto &item: doc.child("osm")) {
    	const auto name = item.name();
    	if (0 == strcmp(name, "way")) {
    		vector<Coord> points;
    		bool found = false;
    		for (const auto &i: item) {
    			if (const auto id = i.attribute("ref").as_int())
    			{
    				const auto nodesIt = nodes.find(id);
    				if (nodesIt == nodes.end()) {
    					cerr << "Bad way point with id " << id << endl;
    					return -1;
    				}

    				points.push_back(nodesIt->second);
    			} else {
					const auto k = i.attribute("k").as_string();
					const auto v = i.attribute("v").as_string();
					if (
						0 == strcmp(k, "name") &&
						mismatch(prefix.begin(), prefix.end(), v, v + strlen(v)).first == prefix.end()
					)
						found = true;
    			}
    		}

    		if (found)
    			segs.push_back(move(points));
    	}
    }

    cout << "Found " << segs.size() << " segs" << endl;

    pugi::xml_document gpxDoc;
    auto gpx = gpxDoc.append_child("gpx");
    gpx.append_attribute("xmlns:xsi").set_value("http://www.w3.org/2001/XMLSchema-instance");
    gpx.append_attribute("version").set_value("1.0");
    gpx.append_attribute("xmlns").set_value("http://www.topografix.com/GPX/1/0");
    gpx.append_attribute("xsi:schemaLocation").set_value("http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd");
    auto trk = gpx.append_child("trk");
    auto name = trk.append_child("name").append_child(pugi::node_pcdata).set_value(trackName.c_str());
    for (const auto &seg: segs) {
        auto trkseg = trk.append_child("trkseg");
        for (const auto &pt: seg) {
        	auto trkpt = trkseg.append_child("trkpt");
        	trkpt.append_attribute("lon").set_value(pt.lon);
        	trkpt.append_attribute("lat").set_value(pt.lat);
        }
    }

    if(gpxDoc.save_file(gpxFileName.c_str()))
    	cout << "Saved: " << gpxFileName << endl;
    else
    	cerr << "Could not save: " << gpxFileName << endl;
}
