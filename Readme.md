# Something something raytracer

Adventures on tracing rays for fun and... Yeah just fun. No profits here.

## Setting up

- Run `git submodules update --init --recursive` in the `deps` folder
- Create a `build` dir in the glfw folder and run `cmake .. -DBUILD_SHARED_LIBS=ON` and then `make`
- Create a `build` dir in the cJSON folder and run `cmake ..` and then `make`
- Build the project with `make`

# LICENSE

- All code outside of the `deps` folder is under the [MIT](LICENSE). Stuff in `deps` has their own license there.
