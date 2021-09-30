add_library(ui OBJECT
	about_window.cpp
	about_window.h
	about_window.ui
	graph_delegate.cpp
	graph_delegate.h
	main_window.cpp
	main_window.h
	main_window.ui
)
set_property(TARGET ui PROPERTY AUTOUIC ON)
set_property(TARGET ui PROPERTY AUTOMOC ON)
set_property(TARGET ui PROPERTY AUTORCC ON)
