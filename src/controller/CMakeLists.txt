add_library(controller OBJECT
	repository_controller.cpp
	repository_controller.h
)
set_property(TARGET controller PROPERTY AUTOMOC ON)
