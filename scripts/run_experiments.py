#!/usr/bin/env python3
#
# @file
#
# @section LICENSE
#
# Copyright (C) 2017 by the Georgia Tech Research Institute (GTRI)
#
# This file is part of SCRIMMAGE.
#
#   SCRIMMAGE is free software: you can redistribute it and/or modify it under
#   the terms of the GNU Lesser General Public License as published by the
#   Free Software Foundation, either version 3 of the License, or (at your
#   option) any later version.
#
#   SCRIMMAGE is distributed in the hope that it will be useful, but WITHOUT
#   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
#   License for more details.
#
#   You should have received a copy of the GNU Lesser General Public License
#   along with SCRIMMAGE.  If not, see <http://www.gnu.org/licenses/>.
#
# @author Kevin DeMarco <kevin.demarco@gtri.gatech.edu>
# @author Eric Squires <eric.squires@gtri.gatech.edu>
# @author Gregory Cooke <gregory.cooke@gtri.gatech.edu>
# @date 28 November 2018
# @version 0.1.0
# @brief Brief file description.
# @section DESCRIPTION
# A Long description goes here.
#
#

import os
import sys
import multiprocessing
import subprocess
import datetime
import argparse
import argcomplete
import generate_scenarios


def call_scrimmage(arg):
    subprocess.call(arg, shell=True)


parser = argparse.ArgumentParser(description="Process arguments for batch "
                                 "scrimmage runs")
parser.add_argument('-t', '--tasks', help='Number of simulation runs',
                    type=int)
parser.add_argument('-m', '--mission', help='Mission file')
parser.add_argument('-r', '--ranges', help='Ranges file for generating new'
                    ' scenarios')
parser.add_argument('-p', '--parallel', help='Maximum number of parallel runs',
                    default=1, type=int)

argcomplete.autocomplete(parser)
args = parser.parse_args()

if not os.path.isfile(args.mission):
    sys.exit('Mission file does not exist')

# See if the user is using the ranges file
if not args.ranges:
    print('not using ranges file')

if args.parallel > multiprocessing.cpu_count():
    print("""=================================================================
WARNING: MAX_PARALLEL_TASKS is set to {},
but you only have {} processors.
====================================================================""".format(
          args.parallel, multiprocessing.cpu_count()))

MISSION_FILE = args.mission
ROOT_LOG = os.path.join(os.path.expanduser("~"), "swarm-log")
LOG_FILE_DIR = os.path.join(ROOT_LOG, "log")
MISSION_FILE_DIR = os.path.join(ROOT_LOG, "missions")

if not os.path.exists(LOG_FILE_DIR):
    os.makedirs(LOG_FILE_DIR)
if not os.path.exists(MISSION_FILE_DIR):
    os.makedirs(MISSION_FILE_DIR)

generate_scenarios.from_run_experiments(args, LOG_FILE_DIR, MISSION_FILE_DIR)

ids_files = [(os.path.join(MISSION_FILE_DIR, "{}.xml".format(i))) for i in
             range(1, args.tasks + 1)]

STARTTIME = datetime.datetime.now()

# Make the pool of processors
pool = multiprocessing.Pool(args.parallel)
tasks = ['scrimmage -j 0 -t {0} {1} > {2}/{0}.log 2>&1'.format(i + 1,
         ids_files[i], LOG_FILE_DIR) for i in range(0, args.tasks)]
# Map tasks to pools
results = pool.map_async(call_scrimmage, tasks)
# Don't let anything else go to the pool and wait for everything in the pool to
# finish
pool.close()
pool.join()

ENDTIME = datetime.datetime.now()
print("Completed {} simulations in {} seconds".format(args.tasks, (ENDTIME -
      STARTTIME).total_seconds()))
