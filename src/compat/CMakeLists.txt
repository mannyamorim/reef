add_library(compat OBJECT
	cpp_git.h
)
set_target_properties(compat PROPERTIES LINKER_LANGUAGE CXX)
