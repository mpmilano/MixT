#!/bin/bash

echo 'delete from causalstore."IntStore";' | psql -U research -d DataStore
echo 'delete from causalstore."BlobStore";' | psql -U research -d DataStore
echo 'insert into causalstore."IntStore" select * from causalstore."SimpleTestIntStore";' | psql -U research -d DataStore
../../euc/vacuum.sh localhost
./causal_relay 8877
