#!/bin/bash

echo 'delete from causalstore."BlobStore";' | psql -U research -d DataStore
echo 'insert into causalstore."BlobStore" select * from causalstore."MailingListExperimentBlobStore";' | psql -U research -d DataStore
../../euc/vacuum.sh localhost
./causal_relay 8877
