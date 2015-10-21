find_package(Freetype REQUIRED)
find_package(PNG REQUIRED)
find_package(TinyXML REQUIRED)

include_directories(
	${PNG_INCLUDE_DIRS}
	${FREETYPE_INCLUDE_DIRS}
	${TinyXML_INCLUDE_DIR})

set(COMMON_SOURCES
	font.cc
	rect.cc
	panic.cc
	pixmap.cc
	sprite.cc
	sprite_base.cc
	sprite_packer.cc)

add_executable(packfont packfont.cc ${COMMON_SOURCES})

target_link_libraries(
	packfont
	${FREETYPE_LIBRARIES}
	${PNG_LIBRARIES}
	${TinyXML_LIBRARY})

add_executable(packsprites packsprites.cc ${COMMON_SOURCES})

target_link_libraries(
	packsprites
	${FREETYPE_LIBRARIES}
	${PNG_LIBRARIES}
	${TinyXML_LIBRARY})
