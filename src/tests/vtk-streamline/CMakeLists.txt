set( _exeName vtk-streamline )
_addExecutable( Test ${_exeName}
    vtk-streamline.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)

set_property( TARGET ${_exeName} PROPERTY FOLDER "Tests/VTK")
