file(REMOVE_RECURSE
  "libmfu.pdb"
  "libmfu.so"
  "libmfu.so.4.0.0"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/mfu.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
