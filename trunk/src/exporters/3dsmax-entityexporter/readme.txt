CTD Entity exporter for 3ds max 5 ( also 5.1 )

version:  1.1

date:     10/15/2004

author:   a. botorabi

e-mail:   botorabi@gmx.net



release notes
-------------

this is an exporter for CTD entities. such entities are prefixed with $ in studio max model objects. the user-defined properties are used to specify entity parameters.


what's new
----------

this release has no further feature additions in comparison to version 1.0. the only difference
in this release is the integration of the source package into new ctd source directory structure of release 0.4.0.


how to use
----------

every entity must be identified by its leading at ( $ ) character in its object name. then one of available entity names follows. the instance name is
given by appending ' : ' and an instance name. e.g.:

  to place a static mesh entity in level model a simple mesh ( e.g. a cube ) and give him the following name:

  $StaticMesh:barrel01


its parameters can be defined using mesh's user-defined properties. 

entity parameters must have the following format in user-defined peroperty section:

 
  <Parameter name> [Type] = <Value>


  Parameter name: must be the exact name of any entity's parameter.

  Type must be one of parameter types:  Float, Integer, Vector3, String, StaticMesh 

  Value is the parameter value, for vectors use a space to separate the x, y, and z components.


legal macros for parameter values are:

  $POSITION
  $ROTATION
  $DIMENSIONS
  
  
  
build notes
-----------

there are vc7.1 project files to build the package. further you need 3ds max's sdk. set the include and lib paths of 
3ds max sdk into vc7.1 studio's project settins ( menu Tools/Options.../Projects/VC++ Directories )
  
  
   
install notes
-------------

1. copy the file 'CTD-entityexporter5.dle' in utils/exporters directory into <3dsmax install path>/plugins

2. copy the file 'IGame.dll' in lib directory into <3dsmax install path>


now your 3ds max has an additional importer called 'CTD ASE importer'.


