# Program
- `iobound.c`: Try `$ ./iobound.c -p` for displaying the header for the result
- `coldstart.sh`: At First clear page cache, then run `iobound.c`. This also accepts option `-p`.

# Experiment
## Approaches
- [x] check the return value of `read()`
- [ ] read HDD not SSD
- [ ]  write dummy data on SSD, then read (requires destructible SSD parition)

## By `check the return value of read()`
found `read error: Invalid argument`

[This](https://stackoverflow.com/questions/10996539/read-from-a-hdd-with-o-direct-fails-with-22-einval-invalid-argument) might be a help for understanding `O_DIRECT`
