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

// TODO: Replace global txn_ with a local copy for each important transaction
Postgres::~Postgres() {
    // txn_.exec("commit;");
    // TODO: Figure out the right way to disconnect and stuff
    // txn_.disconnect();
}

Postgres::Postgres(string dbname, string user, string password)
    : conn_("dbname=" + dbname + " user=" + user + " password=" + password),
    txn_(conn_) {}

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

bool Postgres::create_summary_table(const std::list<std::string> & headers) {

    // TODO, how to get the types for the headers? This is, in general, a
    // problem. Can assume the user knows how to supply it
    return true;
}

bool Postgres::create_table(const std::list<std::string> & columns, const
        std::list<std::string> & types, const std::string & table_name) {
    string query = "CREATE TABLE " +  schema_name_ + "." + table_name +
        " (team_id INT PRIMARY KEY, score DOUBLE PRECISION, ";
    tables_columns_["summary"].push_back("team_id");
    tables_columns_.at("summary").push_back("score");
    auto it1 = columns.begin();
    auto it2 = types.begin();
    for (; it1 != columns.end() && it2 != types.end(); ++it1, ++it2) {
        query += *it1 + " " + *it2 + ", ";
        tables_columns_.at("summary").push_back(*it1);
    }
    query.pop_back();
    query.pop_back();
    query += ");";
    txn_.exec(query);
    txn_.exec("commit;");
    return true;
}

bool Postgres::insert(const std::list<std::string> & columns, const
        std::list<std::string> & values, const std::string & table_name) {
    auto it1 = columns.begin();
    auto it2 = values.begin();
    string query = "INSERT INTO " + table_name + "VALUES (";
    for (; it2 != values.end(); ++it2) {
        query += *it2 + ", ";
    }
    query.pop_back();
    query.pop_back();
    query += ");";
    txn_.exec(query);
    txn_.exec("commit;");
    return true;
}

// Currently unused
bool Postgres::prepare_summary_insert(const std::list<std::string> & columns) {
    string query = "INSERT INTO summary (";
    int i = 1;
    for (auto col: tables_columns_.at("summary")) {
        query += col + ", ";
    }
    query.pop_back();
    query.pop_back();
    query += ") VALUES (";
    for (auto col: tables_columns_.at("summary")) {
        query += "$" + std::to_string(i) + ", ";
        i++;
    }
    query.pop_back();
    query.pop_back();
    query += ");";
    conn_.prepare("summary_insert", query);
    return true;
}

bool Postgres::insert_into_table(const std::map<string, std::map<std::string,
        std::string>> & rows, const std::string & table_name) {
    // Generalized inserting into table
    // Assume we get a map with (row key/identifier: (map with column name:
    // value) in a map with their
    // column names
    // Or should we be unsafe and assume that people will order their vectors
    // correctly and not deal with maps, since we have to go to vectors anyways
    // in the end?
    // For example as it stands right now, ground_coll is in different places
    // when creating the table vs. when bring in the values from the map
    pqxx::work insert_txn(conn_);
    std::vector<string> values;
    pqxx::tablewriter table_writer(insert_txn, schema_name_ + "." + table_name);
    return true;
}

bool Postgres::insert_into_summary(const std::map<int, std::map<std::string,
        double>> & team_metrics, const std::map<int, double> & team_scores) {
    // Hard coded 8 table, we need to actually keep track of this though.
    pqxx::tablewriter table_writer(txn_, schema_name_ + ".summary");
    std::vector<string> values;
    for (auto team_metric: team_metrics) {
        values.push_back(std::to_string(team_metric.first));
        values.push_back(std::to_string(team_scores.at(team_metric.first)));
        for (auto col: tables_columns_.at("summary")) {
            if (!(col == "team_id" || col == "score")) {
                values.push_back(std::to_string(team_metric.second.at(col)));
            }
            // values.push_back(std::to_string(metric_vals.second));
        }
        // for (auto metric_vals: team_metric.second) {
            // i++;
            // values.push_back(std::to_string(metric_vals.second));
        // }
        table_writer.insert(values);
        // table_writer.insert(vals);
        values.clear();
    }
    table_writer.complete();

    // txn_.exec("INSERT INTO summary VALUES (3, 0, 0, 0, 0, 0);");
    txn_.commit();
    // Dynamic statement prep
    // https://stackoverflow.com/questions/31833322/how-to-prepare-statements-and-bind-parameters-in-postgresql-for-c
    // pqxx::prepare::invocation w_invocation = txn_.prepared("summary_insert");
    // prep_dynamic(values, w_invocation);
    // auto res = w_invocation.exec();

    // txn_.exec("commit;");
    return true;
// Try this? https://stackoverflow.com/questions/43922809/libpq-how-to-pass-bulk-data
}

template<class T> pqxx::prepare::invocation&
Postgres::prep_dynamic(std::vector<T> data, pqxx::prepare::invocation& inv) {
    for (auto data_val : data)
        inv(data_val);
    return inv;
}

    // Here's how we write the csv in SimControl right now
    // for (auto const &team_str_double : team_metrics) {

        // // Each line starts with team_id,score
        // csv_str += std::to_string(team_str_double.first);
        // csv_str += "," + std::to_string(team_scores[team_str_double.first]);

        // // Loop over all possible headers, if the header doesn't exist for
        // // a specific team, default the value for that header to zero.
        // for (std::string header : headers) {
            // csv_str += ",";

            // auto it = team_str_double.second.find(header);
            // if (it != team_str_double.second.end()) {
                // csv_str += std::to_string(it->second);
            // } else {
                // csv_str += std::to_string(0.0);
            // }
        // }
        // csv_str += "\n";
    // }

} // namespace scrimmage
