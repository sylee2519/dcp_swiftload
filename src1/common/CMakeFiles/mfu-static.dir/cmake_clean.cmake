file(REMOVE_RECURSE
  "libmfu.a"
  "libmfu.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/mfu-static.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
