# configure
```
cmake -B build -G Ninja
```

# build (includes tests)
```
cmake --build build --parallel
```

# build tests only
```
cmake --build build --target bincache_tests
```

# run tests
```
ctest --test-dir build --output-on-failure
```

# or run the test binary directly
```
./build/bincache_tests
```
