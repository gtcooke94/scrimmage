"""Copies csv files after a scrimmage run to a postgres database."""
import argparse
import os.path
import glob
import pdb
import psycopg2
from psycopg2 import sql
#  from psycopg2 import sql

# We need a log directory and datatypes fed to us
# example:
# python csv_to_postgres.py ~/.scrimmage/logs/2018-06-28_09-41-11/ "DOUBLE
# PRECISION" "TEXT" "INT"
parser = argparse.ArgumentParser()
parser.add_argument("log_dir")
parser.add_argument("data_types", nargs="*")
args = parser.parse_args()
#  pdb.set_trace()
#  args = parser.parse_known_args(["log_dir"])
log_dir = args.log_dir
data_types = args.data_types
#  pdb.set_trace()
#  print(args.log_dir)

# Connect to existing database
conn = psycopg2.connect("dbname=scrimmage user=scrimmage password=scrimmage")

# Open cursor to perform operations
cur = conn.cursor()

# Assume we have the log directory put through

# Assume we pass through tables, columns, datatypes that we want
# Build the table
#  log_dir = '~/.scrimmage/logs/2018-06-28_09-41-11/'
pdb.set_trace()
timestamp = os.path.basename(os.path.dirname(log_dir))
timestamp = timestamp.replace('-', '_')
schema = 's' + timestamp
cur.execute('CREATE SCHEMA IF NOT EXISTS ' + schema + ";")
conn.commit()

# Assume we have a list of tables that we want to make? Or can glob for csvs in
# the log directory
#  data_types = [
#  "INT PRIMARY KEY", "DOUBLE PRECISION",
#  "DOUBLE PRECISION", "DOUBLE PRECISION", "DOUBLE PRECISION",
#  "DOUBLE PRECISION", "DOUBLE PRECISION", "DOUBLE PRECISION"]

#  query = 'CREATE TABLE ' + schema + "." + table_name + "( "
# for (col, data_type) in zip(columns, data_types):

# Actually copy all of the data
files = glob.glob(log_dir + "*.csv")
#  pdb.set_trace()
for fi in files:
    with open(fi, 'r') as f:
        pdb.set_trace()
        # Notice that we don't need the `csv` module.
        # next(f)  # Skip the header row.
        table = os.path.basename(os.path.splitext(fi)[0])
        full_table = "{}.{}".format(schema, table)
        cols = f.readline()
        cols = cols.splitlines()[0]
        cols = cols.split(',')
        # could get datatypes with
        # ["DOUBLE PRECISION" for i in range(0, len(cols)]
        query_string = 'CREATE TABLE {} '.format(full_table)
        to_join = ''.join([col + ' ' + data_type + ', ' for col, data_type in
                           zip(cols, data_types)])
        to_join = to_join[:-2]
        to_join = "({});".format(to_join)
        query_string = query_string + to_join
        query = sql.SQL(query_string)
        cur.execute(query)
        #  cur = conn.cursor()
        #  pdb.set_trace()

        cur.copy_from(f, "{}".format(full_table), sep=',')
        conn.commit()
conn.close()
