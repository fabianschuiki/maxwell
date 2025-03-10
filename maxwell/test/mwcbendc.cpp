/* Copyright (c) 2013-2014 Fabian Schuiki */
/** @file This program invokes the C backend on a list of nodes in the
 * repository. */
#include "maxwell/ast/Repository.hpp"
#include "maxwell/backend/c/CodeGenerator.hpp"
#include "maxwell/backend/c/database.hpp"
#include "maxwell/sqlite3.hpp"
#include <cstdlib>
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

using ast::NodeId;
using namespace backendc;

int main(int argc, char *argv[])
{
	sqlite3* db = NULL;
	try {
		ast::Repository repo("mwcrepo");

		// Open the database.
		int rc = sqlite3_open("mwcrepo/bendc.db", &db);
		if (rc) {
			std::stringstream s;
			s << "Can't open database: %s" << sqlite3_errmsg(db);
			throw std::runtime_error(s.str());
		}
		sqlite3_exec(db, "PRAGMA foreign_keys = ON", NULL, NULL, NULL);

		// Make sure the database has the right schema.
		database(db).prepareFragmentsSchema();

		// Read the node IDs from the input.
		std::vector<NodeId> ids;
		for (int i = 1; i < argc; i++) {
			ids.push_back(NodeId(std::string(argv[i])));
		}

		// Run the code generator for the given node IDs. Then output the list
		// files that were generated.
		CodeGenerator cg(repo, db);
		cg.run(ids.begin(), ids.end());

		for (std::map<NodeId,std::string>::const_iterator i = cg.names.begin(); i != cg.names.end(); i++)
			std::cout << i->second << '\n';

		// Close the database.
		sqlite3_close(db);

	} catch (std::exception &e) {
		std::cerr << "*** \033[31;1mexception:\033[0m " << e.what() << "\n";
		if (db) sqlite3_close(db);
		return 1;
	}
	return 0;
}
