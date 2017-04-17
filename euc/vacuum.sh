#!/bin/bash
echo 'vacuum analyze causalstore."IntStore"; vacuum analyze causalstore."BlobStore"; vacuum analyze causalstore.counters; vacuum analyze "BlobStore"."BlobStore"; vacuum analyze "BlobStore"."IntStore"' | psql -h $1 DataStore;
