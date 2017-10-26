#!/bin/bash

echo 'delete from "BlobStore"."IntStore";' | psql -U research -d DataStore
echo 'delete from "BlobStore"."BlobStore";' | psql -U research -d DataStore
echo 'insert into "BlobStore"."IntStore" select * from "BlobStore"."SimpleTestIntStore";' | psql -U research -d DataStore
../../euc/vacuum.sh localhost
./strong_relay_tracked 8876
