/* This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information. */

#ifdef UA_NO_AMALGAMATION
# include "ua_types.h"
# include "ua_client.h"
# include "ua_client_highlevel.h"
# include "ua_nodeids.h"
# include "ua_network_tcp.h"
# include "ua_config_standard.h"
#else
# include "open62541.h"
# include <string.h>
# include <stdlib.h>
#endif

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <map>
using namespace std;

#include "client-common.h"
#include "client-nodemap.h"
#include "client-nodeid.h"

extern map<int, Group>* gmap;

template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

UA_NodeIdType getUA_NodeID(char* id, UA_NodeId* ua)
{
    string s(id);
    vector<string> x = split(s, ';');

    for(int i = 0; i < (int)x.size(); i++) {

        vector<string> y = split(x[i], '=');
        for(int j = 0; j < (int)x.size(); j++) {

            if(y[j].compare(string("ns")) == 0) {
                ua->namespaceIndex = atoi(y[j+1].c_str());
                continue;
            }
            else if(y[j].compare(string("i")) == 0) {
                ua->identifierType = UA_NODEIDTYPE_NUMERIC;
                ua->identifier.numeric = atoi(y[j+1].c_str());
                break;
            }
            else if(y[j].compare(string("s")) == 0) {
                ua->identifierType = UA_NODEIDTYPE_STRING;
                ua->identifier.string = UA_STRING_ALLOC(y[j+1].c_str());
                break;
            }
        }
    }

    return ua->identifierType;
}