set_languages("cxx17")
add_rules("mode.debug", "mode.release")
set_policy("preprocessor.gcc.directives_only", false)

local libs = { "openmp" }
-- local libs = { "fmt" }

add_includedirs("include")
add_requires(table.unpack(libs))

target("sahga_lib")
  set_kind("static")
  add_files("source/sahga/**/*.cpp")
  add_packages(table.unpack(libs))

target("SAHGA")
  set_kind("binary")
  add_files("standalone/sahga.cpp")
  add_packages(table.unpack(libs))
  add_deps("sahga_lib")

target("MPI")
  set_kind("binary")
  add_files("standalone/mpi.cpp")
