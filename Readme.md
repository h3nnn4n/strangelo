# Something something raytracer

[![clang-format](https://github.com/h3nnn4n/raytracer-adventures/actions/workflows/clang-format-check.yml/badge.svg?branch=main)](https://github.com/h3nnn4n/raytracer-adventures/actions/workflows/clang-format-check.yml)
[![clang-tidy](https://github.com/h3nnn4n/raytracer-adventures/actions/workflows/clang-tidy.yml/badge.svg)](https://github.com/h3nnn4n/raytracer-adventures/actions/workflows/clang-tidy.yml)
[![cppcheck](https://github.com/h3nnn4n/raytracer-adventures/actions/workflows/cppcheck.yml/badge.svg)](https://github.com/h3nnn4n/raytracer-adventures/actions/workflows/cppcheck.yml)
[![cpplint](https://github.com/h3nnn4n/raytracer-adventures/actions/workflows/cpplint.yml/badge.svg)](https://github.com/h3nnn4n/raytracer-adventures/actions/workflows/cpplint.yml)

Adventures on tracing rays for fun and... Yeah just fun. No profits here.

This implements a path tracer using an OpenGL compute shader. Spheres and
triangle primitives are supported, with diffuse, metallic and dielectric
materials.

There are a few bugs laying around causing some visual artifacts, and some
portions are implemented in a plain wrong way (but giving reasonable outputs).

## Setting up

- Run `git submodules update --init --recursive` in the `deps` folder
- Create a `build` dir in the glfw folder and run `cmake .. -DBUILD_SHARED_LIBS=ON` and then `make`
- Create a `build` dir in the cJSON folder and run `cmake ..` and then `make`
- Build the project with `make`

# LICENSE

All code outside of the `deps` folder is under the [MIT](LICENSE). Stuff in
`deps` has their own license there.

Some portions of the code were copy-pasted verbatim or with minor
modifications. Proper attribution is given next to such occurrences.
