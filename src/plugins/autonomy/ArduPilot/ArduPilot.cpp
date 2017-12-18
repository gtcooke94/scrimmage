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
 * @author Michael Day <michael.day@gtri.gatech.edu>
 * @date 20 Nov 2017
 * @version 0.1.0
 * @brief Class to interface with ArduPilot SIL executable.
 * @section DESCRIPTION
 * Class to interface with ArduPilot SIL executable.
 *
 */

#include <iostream>
#include <limits>
#include <fstream>

#include <scrimmage/plugin_manager/RegisterPlugin.h>
#include <scrimmage/entity/Entity.h>
#include <scrimmage/math/Angles.h>
#include <scrimmage/math/State.h>
#include <scrimmage/parse/ParseUtils.h>

#include <scrimmage/plugins/autonomy/ArduPilot/ArduPilot.h>
#include <scrimmage/parse/MissionParse.h>

//Asio includes
#include <asio/ip/udp.hpp>

using std::cout;
using std::cerr;
using std::endl;
using asio::ip::udp;

namespace sc = scrimmage;

REGISTER_PLUGIN(scrimmage::Autonomy,
                scrimmage::autonomy::ArduPilot,
                ArduPilot_plugin)

namespace scrimmage {
namespace autonomy {

ArduPilot::ArduPilot()
    : previous_step_time(-1.0)
    , do_listen_to_ardu(true)
    , do_send_to_ardu(true)
    , ardu_listener_thr(&ArduPilot::ardu_listener, this) //start listener thread
    , ardu_sender_thr(&ArduPilot::ardu_sender, this) //start listener thread
{
}

ArduPilot::~ArduPilot()
{
    //Destrutors currently don't work in SCRIMMAGE
    //WORKAROUND: use close method inherited from Plugin class.
}

void ArduPilot::init(std::map<std::string,std::string> &params)
{
    multirotor_ = std::dynamic_pointer_cast<sc::motion::Multirotor>(parent_->motion());
    if (multirotor_ == nullptr) {
        cout << "WARNING: MultirotorTests can't control the motion "
             << "model for this entity." << endl;
    }

    desired_rotor_state_ = std::make_shared<sc::motion::MultirotorState>();
    desired_rotor_state_->set_input_type(sc::motion::MultirotorState::InputType::PWM);
    desired_rotor_state_->prop_input().resize(multirotor_->rotors().size());
    desired_state_ = desired_rotor_state_;
}

//runs as the Plugin closes
void ArduPilot::close(double t)
{
    //try to get threads and associated file handles or any other I/O stuff
    //to clean up before we shut down completely.
    do_listen_to_ardu = false;
    do_send_to_ardu = false;
}

bool ArduPilot::step_autonomy(double t, double dt)
{

    //std::cout << "state pos 0: " << state_->pos()(0) << std::endl; //x
    //std::cout << "state pos 1: " << state_->pos()(1) << std::endl; //y
    //std::cout << "state pos 2: " << state_->pos()(2) << std::endl; //z
/*
    std::cout << "state vel 0: " << state_->vel()(0) << std::endl;
    std::cout << "state vel 1: " << state_->vel()(1) << std::endl;
    std::cout << "state vel 2: " << state_->vel()(2) << std::endl;

    std::cout << "state roll: " << Angles::rad2deg(state_->quat().roll()) << std::endl;
    std::cout << "state pitch: " << Angles::rad2deg(state_->quat().pitch()) << std::endl;
    std::cout << "state yaw: " << Angles::rad2deg(state_->quat().yaw()) << std::endl;

    desired_state_->pos()(0) = 0.0f; // aileron  (x-axis rotation) (-1 to 1)
    desired_state_->pos()(1) = 0.0f; // elevator (y-axis rotation) (-1 to 1)
    desired_state_->pos()(2) = 0.0f; // rudder   (z-axis rotation) (-1 to 1)
    desired_state_->vel()(0) = 0.0f; //throttle (0 to 1)
*/
    //cout << "setting desired state in ArudPilot autonomy plugin.\n";

    //desired_state_->pos()(2) = 0.0f; // rudder   (z-axis rotation) (-1 to 1)
    //desired_state_->vel()(0) = 0.4f;

    previous_step_time = t;

    servo_pkt_mutex_.lock();
    for (int i = 0; i < desired_rotor_state_->prop_input().size(); i++) {
        desired_rotor_state_->prop_input()(i) = servo_pkt_.servos[i];
    }
    servo_pkt_mutex_.unlock();

    desired_state_ = desired_rotor_state_;

    return true;
}

void ArduPilot::ardu_listener() {
    //connect to ardupilot
    // TODO: un-hard code port.
    uint32_t port = 5002;
    asio::io_service io_service;
    udp::socket sock(io_service, udp::endpoint(udp::v4(), port));
    udp::endpoint sender_endpoint;
    std::size_t reply_len = 0;

    std::string debug_log = parent_->mp()->log_dir() + "/ardu_error.txt";
    std::ofstream dbg_strm;
    dbg_strm.open(debug_log, std::ofstream::out);
    dbg_strm << "SCRIMMAGE Ardupilot plugin error log.\n";

    //listen for control inputs from ardupilot
    while (do_listen_to_ardu) {
        servo_packet pkt = {0}; //reinitialize ever iteration to ensure clean.

        try {
            reply_len = sock.receive_from(
                asio::buffer(&pkt, sizeof(pkt)), sender_endpoint);

        } catch (std::exception& e) {
            cerr << "Socket error: << " << e.what() << endl;
            dbg_strm << "Socket error: << " << e.what() << endl;
        }

        if (reply_len < sizeof(pkt)) {
            continue; //ignore packets without sufficient data
        }

        servo_pkt_mutex_.lock();
        servo_pkt_ = pkt;
        servo_pkt_mutex_.unlock();

        //TODO: delte lines
        //dbg_strm << "Got a packet with " << reply_len << " bytes: \n";
        //dbg_strm << pkt.servos[0] << endl;
        //cout << pkt.servos[0] << endl;

        //TODO: endian handling
        //
        //TODO: send servo inputs just received to appropriate SCRIMMAGE model
    }
    dbg_strm.close();
}

void ArduPilot::ardu_sender() {
    //connect to ardupilot
    asio::io_service io_service;

    //TODO: un-hardcode IP address and port
    udp::socket s(io_service, udp::endpoint(udp::v4(), 0));
    udp::resolver resolver(io_service);
    udp::endpoint endpoint = *resolver.resolve({udp::v4(), "127.0.0.1", "5003"});
    //forward aircraft state to ArduPilot
    while (do_send_to_ardu) {
        fdm_packet fdm_pkt = {0}; //re-init every iteration to keep clean

        //TODO: remaining data needs to get into fdm_packet to send to Ardu
        fdm_pkt.timestamp_us = uint64_t(previous_step_time * 1.0e6);
        fdm_pkt.latitude = 921.0;

        //TODO: endian handling.
        try {
            s.send_to(asio::buffer(&fdm_pkt, sizeof(fdm_packet)), endpoint);
        } catch (std::exception& e) {
            //TODO: log
            cerr << "Exception: " << e.what() << "\n";
        }
    }
}

} // namespace autonomy
} // namespace scrimmage
