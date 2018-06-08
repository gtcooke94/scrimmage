/*!
 * @file
 *
 * @section LICENSE
 *
 * Copyright (C) 2017 by the Georgia Tech Research Institute (GTRI)
 *
 * This file is part of SCRIMMAGE.
 *
 *   SCRIMMAGE is free software: you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the
 *   Free Software Foundation, either version 3 of the License, or (at your
 *   option) any later version.
 *
 *   SCRIMMAGE is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 *   License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with SCRIMMAGE.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @author Kevin DeMarco <kevin.demarco@gtri.gatech.edu>
 * @author Eric Squires <eric.squires@gtri.gatech.edu>
 * @date 31 July 2017
 * @version 0.1.0
 * @brief Brief file description.
 * @section DESCRIPTION
 * A Long description goes here.
 *
 */

#include <scrimmage/common/Postgres.h>
#include <iostream>
#include <algorithm>

using std::cout;
using std::endl;
using std::string;

namespace scrimmage {

Postgres::~Postgres() {
    txn_.exec("commit;");
}

Postgres::Postgres(string dbname, string user, string password)
    : conn_("dbname=" + dbname + " user=" + user + " password=" + password), txn_(conn_) {}

bool Postgres::create_schema(string schema_name) {
    std::size_t last_dash_pos = schema_name.find_last_of("/");
    std::string timestamp = schema_name.substr(last_dash_pos + 1, schema_name.length());
    std::replace(timestamp.begin(), timestamp.end(), '-', '_');
    timestamp.insert(0, "s");
    try {
        txn_.exec("CREATE SCHEMA IF NOT EXISTS " + timestamp + ";");
        schema_name_ = timestamp;
        return true;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

} // namespace scrimmage
