"""Copies csv files after a scrimmage run to a postgres database."""
import os.path
import csv
import psycopg2
from psycopg2 import sql
import pdb
#  from psycopg2 import sql

# Connect to existing database
conn = psycopg2.connect("dbname=scrimmage user=scrimmage password=scrimmage")

# Open cursor to perform operations
cur = conn.cursor()

# Assume we have the log directory put through

# Assume we pass through tables, columns, datatypes that we want
# Build the table
log_dir = '~/.scrimmage/logs/2018-06-28_09-41-11/'
timestamp = os.path.basename(os.path.dirname(log_dir))
timestamp = timestamp.replace('-', '_')
schema = 's' + timestamp
cur.execute('CREATE SCHEMA IF NOT EXISTS ' + schema + ";")
conn.commit()

# Assume we have a list of tables that we want to make? Or can glob for csvs in
# the log directory
table = 'summary'
full_table = "{}.{}".format(schema, table)
data_types = [
    "INT PRIMARY KEY", "DOUBLE PRECISION",
    "DOUBLE PRECISION", "DOUBLE PRECISION", "DOUBLE PRECISION",
    "DOUBLE PRECISION", "DOUBLE PRECISION", "DOUBLE PRECISION"]
#  query = 'CREATE TABLE ' + schema + "." + table_name + "( "
# for (col, data_type) in zip(columns, data_types):

# Actually copy all of the data
with open('/home/gcooke3/.scrimmage/logs/2018-06-27_14-23-09/summary.csv', 'r') as f:
#  f = open('/home/gcooke3/.scrimmage/logs/2018-06-27_14-23-09/summary.csv', 'r')
    # Notice that we don't need the `csv` module.
    # next(f)  # Skip the header row.
    cols = f.readline()
    cols = cols.splitlines()[0]
    cols = cols.split(',')
    query_string = 'CREATE TABLE {} '.format(full_table)
    to_join = ''.join([col + ' ' + data_type + ', ' for col, data_type in
                       zip(cols, data_types)])
    to_join = to_join[:-2]
    to_join = "({});".format(to_join)
    query_string = query_string + to_join
    query = sql.SQL(query_string)
    cur.execute(query)
    #  cur = conn.cursor()
    pdb.set_trace()

    cur.copy_from(f, "{}".format(full_table), sep=',')
    conn.commit()
    #  f.close()
