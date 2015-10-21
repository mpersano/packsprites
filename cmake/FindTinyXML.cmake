FIND_PATH(TinyXML_INCLUDE_DIR tinyxml.h)
FIND_LIBRARY(TinyXML_LIBRARY NAMES tinyxml)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
	TinyXML REQUIRED_VARS
	TinyXML_INCLUDE_DIR
	TinyXML_LIBRARY)