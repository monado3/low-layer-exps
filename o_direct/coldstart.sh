#!/bin/bash
sudo -- sh -c "sync; echo 1 > /proc/sys/vm/drop_caches"
sudo -- sh -c "./iobound.out $@"
