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

#include <scrimmage/plugins/metrics/CPA/CPA.h>

#include <scrimmage/plugin_manager/RegisterPlugin.h>
#include <scrimmage/entity/Entity.h>
#include <scrimmage/math/State.h>
#include <scrimmage/parse/ParseUtils.h>
#include <scrimmage/common/Utilities.h>
#include <scrimmage/metrics/Metrics.h>

#include <scrimmage/pubsub/Message.h>
#include <scrimmage/pubsub/Subscriber.h>
#include <scrimmage/msgs/Collision.pb.h>
#include <scrimmage/msgs/Event.pb.h>

#include <iostream>
#include <limits>

using std::cout;
using std::endl;

namespace sc = scrimmage;
namespace sm = scrimmage_msgs;

REGISTER_PLUGIN(scrimmage::Metrics,
                scrimmage::metrics::CPA,
                CPA_plugin)

namespace scrimmage {
namespace metrics {

CPA::CPA() {
}

void CPA::init(std::map<std::string, std::string> &params) {
    for (auto &kv : *id_to_ent_map_) {
        cpa_map_[kv.first] = CPAData();

    }
}

bool CPA::step_metrics(double t, double dt) {
    for (auto &kv : *id_to_ent_map_) {
        for (auto &kv2 : *id_to_ent_map_) {
            if (kv != kv2) {
                double cur_distance = (kv.second->state()->pos() -
                        kv2.second->state()->pos()).norm();
                if (cur_distance < cpa_map_[kv.first].distance()) {
                    cpa_map_[kv.first].set_distance(cur_distance);
                    cpa_map_[kv.first].set_closest_entity(kv2.first);
                    cpa_map_[kv.first].set_time(t);
                }
            }
        }
    }
    return true;
}

void CPA::calc_team_scores() {
    // for (auto &kv : team_coll_scores_) {
        // int team_id = kv.first;
        // Score &score = kv.second;
        // team_metrics_[team_id]["ground_coll"] = score.ground_collisions();
        // team_scores_[team_id] = score.score();
    // }
    for (auto &kv : *id_to_ent_map_) {
        team_metrics_[kv.second->id().team_id()]["entity"] = kv.first;
        team_metrics_[kv.second->id().team_id()]["cpa"] =
            cpa_map_[kv.first].distance();
        team_metrics_[kv.second->id().team_id()]["closest_entity"] =
            cpa_map_[kv.first].closest_entity();
        team_metrics_[kv.second->id().team_id()]["time"] =
            cpa_map_[kv.first].time();


    }
    // list the headers we want put in the csv file
    headers_.push_back("entity");
    headers_.push_back("cpa");
    headers_.push_back("closest entity");
    headers_.push_back("time");
}

void CPA::print_team_summaries() {
    // for (std::map<int, Score>::iterator it = team_coll_scores_.begin();
         // it != team_coll_scores_.end(); ++it) {

        // cout << "Score: " << it->second.score() << endl;
        // cout << sc::generate_chars("-", 70) << endl;
    // }
}
} // namespace metrics
} // namespace scrimmage
