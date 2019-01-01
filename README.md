![build status](https://gitlab.com/mvojacek/growstack/badges/master/build.svg)
# GrowStack

This C/asm PoC executes a function with its stack moved to the heap, allowing for larger stacks.

[Download latest build](https://gitlab.com/mvojacek/growstack/-/jobs/artifacts/master/download?job=build)

The main function first executes a large recursive function with this PoC, using about 2GB or memory for the
stack, which should succeed, and then executes the same function on the system stack, which should fail.

# Building

```sh
cmake .
make
```

# License

All rights reserved.

# Disclaimer

I am not responsible for anything that happens,
including but not limited to, alien invasions,
computers becoming self-aware, Mark Zuckerberg
collecting your data, your grandmother
getting scammed online, or the FBI taking you
into custody for cyber crime.
