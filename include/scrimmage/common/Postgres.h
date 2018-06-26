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

#ifndef INCLUDE_SCRIMMAGE_COMMON_POSTGRES_H_
#define INCLUDE_SCRIMMAGE_COMMON_POSTGRES_H_

#include <pqxx/pqxx>
#include <fstream>
#include <map>
#include <list>
#include <string>
#include <utility>
#include <memory>
#include <vector>

namespace scrimmage {

class Postgres {
 public:
     Postgres(std::string dbname, std::string user, std::string password);
     ~Postgres();

     bool create_schema(std::string schema_name);
     bool create_summary_table(const std::list<std::string> & headers);
     bool create_table(const std::list<std::string> & columns, const
             std::list<std::string> & types, const std::string & table_name);
     bool insert(const std::list<std::string> & columns, const
        std::list<std::string> & values, const std::string & table_name);
     bool prepare_summary_insert(const std::list<std::string> & columns);
     bool insert_into_summary(const std::map<int, std::map<std::string, double>>
             & team_metrics, const std::map<int, double> & team_scores);

 protected:
     pqxx::connection conn_;
     // pqxx::nontransaction ntxn_;
     pqxx::work txn_;
     template<class T> pqxx::prepare::invocation& prep_dynamic(std::vector<T> data, pqxx::prepare::invocation& inv);
     std::string schema_name_;
     std::map<std::string, std::vector<std::string>> tables_columns_;
};
} // namespace scrimmage

#endif // INCLUDE_SCRIMMAGE_COMMON_POSTGRES_H_
