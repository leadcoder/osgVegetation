#pragma once
#if defined ( WIN32 )
#   	if defined( OSG_VEGETATION_EXPORTS )
#       	define osgvExport __declspec( dllexport )
#   	else
#      		define osgvExport __declspec( dllimport )
#   	endif
#else
#   define osgvExport
#endif


#ifndef OSG_VERSION_GREATER_OR_EQUAL
	#define OSG_VERSION_GREATER_OR_EQUAL(MAJOR, MINOR, PATCH) ((OPENSCENEGRAPH_MAJOR_VERSION>MAJOR) || (OPENSCENEGRAPH_MAJOR_VERSION==MAJOR && (OPENSCENEGRAPH_MINOR_VERSION>MINOR || (OPENSCENEGRAPH_MINOR_VERSION==MINOR && OPENSCENEGRAPH_PATCH_VERSION>=PATCH))))
#endif

#include <stdexcept>
#define OSGV_EXCEPT(a) throw std::runtime_error(a)

