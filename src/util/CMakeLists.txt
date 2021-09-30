add_library(util OBJECT
	block_allocator.h
	error.h
	preferences.h
	reef_string.h
	version.h
)
set_target_properties(util PROPERTIES LINKER_LANGUAGE CXX)
