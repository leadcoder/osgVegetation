FIND_PATH(OSG_INCLUDE_DIR osg/Node  
	$ENV{OSG_HOME}/include
	$ENV{OSG_HOME}
    $ENV{OSG_DIR}/include
    $ENV{OSG_DIR}
    $ENV{OSGDIR}/include
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/include
	${OSG_DIR}/include
    NO_DEFAULT_PATH
)


FIND_PATH(OSG_LIB_DIR osg.lib		
		$ENV{OSG_HOME}/lib
		$ENV{OSG_HOME}
		$ENV{OSG_DIR}/lib
		$ENV{OSG_DIR}
		${OSG_DIR}/lib
		NO_DEFAULT_PATH)
		
FIND_PATH(OSG_BIN_DIR osgviewer.exe		
		$ENV{OSG_HOME}/bin
		$ENV{OSG_HOME}
		$ENV{OSG_DIR}/bin
		$ENV{OSG_DIR}
		${OSG_DIR}/bin		
		NO_DEFAULT_PATH)