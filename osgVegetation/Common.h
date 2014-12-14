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

