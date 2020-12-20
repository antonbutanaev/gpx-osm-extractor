#include <iostream>
#include <map>
#include <vector>
#include <string.h>
#include <pugixml.hpp>

using namespace std;

struct Coord
{
	double lon,lat;
};

int main()
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("/home/anton/temp/mos.osm");
    if (!result)
        return -1;

    cout << "Loaded OK" << endl;

    map<int, Coord> nodes;
    for (const auto &item: doc.child("osm")) {
    	const auto name = item.name();
    	//cout << "Name <" << name << ">" << endl;
    	if (0 == strcmp(name, "node")) {
    		//cout << "Node " << item.attribute("id").as_int() << ' ' << item.attribute("lon").as_double() << ' ' << item.attribute("lat").as_double() << endl;
    		nodes[item.attribute("id").as_int()] = {item.attribute("lon").as_double(), item.attribute("lat").as_double()};

    	}
    }

    for (const auto &item: doc.child("osm")) {
    	const auto name = item.name();
    	if (0 == strcmp(name, "way")) {
    		vector<Coord> points;
    		bool ok = false;
    		for (const auto &i: item) {
    			if (const auto id = i.attribute("ref").as_int())
    			{
    				const auto nodesIt = nodes.find(id);
    				if (nodesIt == nodes.end()) {
    					cerr << "1" << endl;
    					return -2;
    				}

    				points.push_back(nodesIt->second);
    				//cout << nodesIt->second.lat << ' ' << nodesIt->second.lon << endl;

    			}

    			const auto k = i.attribute("k").as_string();
    			const auto v = i.attribute("v").as_string();
    			if (0 == strcmp(k, "name") && 0 == strcmp(v, "mos22"))
    				ok = true;
    		}

    		if (ok)
    			for (const auto &it: points)
    				cout << "Pt " << it.lat << ' ' << it.lon << endl;
    	}
    }

}
