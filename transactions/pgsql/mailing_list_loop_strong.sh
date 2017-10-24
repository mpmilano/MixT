#!/bin/bash

echo 'delete from "BlobStore"."BlobStore";' | psql -U research -d DataStore
echo 'insert into "BlobStore"."BlobStore" select * from "BlobStore"."MailingListExperimentBlobStore;' | psql -U research -d DataStore
../../euc/vacuum.sh localhost
./causal_relay 8877
