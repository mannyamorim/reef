add_library(ui OBJECT
	about_window.cpp
	about_window.h
	about_window.ui
	deselectable_list_view.cpp
	deselectable_list_view.h
	dock_widget_title_bar.cpp
	dock_widget_title_bar.h
	graph_delegate.cpp
	graph_delegate.h
	main_window.cpp
	main_window.h
	main_window.ui
)
set_property(TARGET ui PROPERTY AUTOUIC ON)
set_property(TARGET ui PROPERTY AUTOMOC ON)
set_property(TARGET ui PROPERTY AUTORCC ON)
