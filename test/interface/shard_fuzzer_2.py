#!/usr/bin/env python
# Copyright 2014 RethinkDB, all rights reserved.

'''This test randomly rebalances tables and shards to probabilistically find bugs in the system.'''

from __future__ import print_function

import pprint, os, sys, time, random, threading

startTime = time.time()

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), os.path.pardir, 'common')))
import driver, scenario_common, utils, vcoptparse

opts = vcoptparse.OptParser()
scenario_common.prepare_option_parser_mode_flags(opts)
opts['num-servers'] = vcoptparse.IntFlag('--num-servers', 1)
opts['num-rows'] = vcoptparse.IntFlag('--num-rows', 10)
opts['num-shards'] = vcoptparse.IntFlag('--num-shards', 1)
opts['num-replicas'] = vcoptparse.IntFlag('--num-replicas', 1)
opts['num-phases'] = vcoptparse.IntFlag('--num-phases', 1)
parsed_opts = opts.parse(sys.argv)
_, command_prefix, serve_options = scenario_common.parse_mode_flags(parsed_opts)

server_names = list("abcdefghijklmnopqrstuvwxyz"[:parsed_opts["num-servers"]])

def make_config_shards(phase):
    shards = []
    for i in xrange(parsed_opts['num-shards']):
        shard = {}
        shard["primary_replica"] = server_names[(i + phase) % len(server_names)]
        shard["replicas"] = []
        assert parsed_opts["num-replicas"] <= len(server_names)
        for j in xrange(parsed_opts['num-replicas']):
            shard["replicas"].append(server_names[(i + j + phase) % len(server_names)])
        shards.append(shard)
    return shards

r = utils.import_python_driver()

print("Spinning up %d servers (%.2fs)" % (len(server_names), time.time() - startTime))
with driver.Cluster(initial_servers=server_names, output_folder='.', command_prefix=command_prefix,
                    extra_options=serve_options, wait_until_ready=True) as cluster:
    cluster.check()
    
    print("Establishing ReQL connection (%.2fs)" % (time.time() - startTime))
    conn = r.connect(host=cluster[0].host, port=cluster[0].driver_port)

    print("Setting up table (%.2fs)" % (time.time() - startTime))
    res = r.db_create("test").run(conn)
    assert res["dbs_created"] == 1, res
    res = r.table_create("test").run(conn)
    assert res["tables_created"] == 1, res
    res = r.table("test").insert(
        r.range(0, parsed_opts["num-rows"]).map({"x": r.row})).run(conn)
    assert res["inserted"] == parsed_opts["num-rows"], res

    for phase in xrange(parsed_opts['num-phases']):
        print("Beginning reconfiguration phase %d (%.2fs)" % (phase + 1, time.time() - startTime))
        shards = make_config_shards(phase)
        res = r.table("test").config().update({"shards": shards}).run(conn)
        assert res["replaced"] == 1 or res["unchanged"] == 1, res
        time.sleep(1)   # work around issue #4265
        print("Waiting for table to become ready (%.2fs)" % (time.time() - startTime))
        res = r.table("test").wait(wait_for = "all_replicas_ready", timeout = 600).run(conn)
        assert res["ready"] == 1, res
        for config_shard, status_shard in \
                zip(shards, res["status_changes"][0]["new_val"]["shards"]):
            # make sure issue #4265 didn't happen
            assert status_shard["primary_replicas"] == [config_shard["primary_replica"]]

    print("Cleaning up (%.2fs)" % (time.time() - startTime))
print("Done. (%.2fs)" % (time.time() - startTime))
