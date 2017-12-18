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

#ifndef ArduPilot_H_
#define ArduPilot_H_
#include <scrimmage/autonomy/Autonomy.h>

#include <scrimmage/plugins/motion/Multirotor/Multirotor.h>
#include <scrimmage/plugins/motion/Multirotor/MultirotorState.h>
#include <scrimmage/plugins/motion/RigidBody6DOF/RigidBody6DOFState.h>

#include <thread>
#include <mutex>

namespace scrimmage {
namespace autonomy {
class ArduPilot : public scrimmage::Autonomy {
private:
    static const int MAX_NUM_SERVOS = 16;
    /*
      packet received by Scrimmage with state of ArduPilot servos
      */
    struct servo_packet {
        uint16_t servos[MAX_NUM_SERVOS];
    };

    /*
      state packet sent from Scrimmage to ArduPilot
     */
    struct fdm_packet {
        uint64_t timestamp_us; // simulation time in microseconds
        double latitude, longitude;
        double altitude;
        double heading;
        double speedN, speedE, speedD;
        double xAccel, yAccel, zAccel;
        double rollRate, pitchRate, yawRate;
        double roll, pitch, yaw;
        double airspeed;
    };

public:
    ArduPilot();
    ~ArduPilot();
    virtual void init(std::map<std::string,std::string> &params);
    virtual bool step_autonomy(double t, double dt);
    virtual void close(double t);
protected:
    double previous_step_time;

    bool do_listen_to_ardu;
    bool do_send_to_ardu;
    std::thread ardu_listener_thr;
    std::thread ardu_sender_thr;
    void ardu_listener();
    void ardu_sender();

    std::shared_ptr<scrimmage::motion::Multirotor> multirotor_;
    std::shared_ptr<scrimmage::motion::MultirotorState> desired_rotor_state_;

    std::mutex servo_pkt_mutex_;
    servo_packet servo_pkt_;

    std::mutex state_6dof_mutex_;
    scrimmage::motion::RigidBody6DOFState state_6dof_;

};
} // namespace autonomy
} // namespace scrimmage
#endif
