
CTD ASE file importer for 3ds max 5 ( also 5.1 )

version:  1.1

date:     10/15/2004

author:   a. botorabi

e-mail:   botorabi@gmx.net


release notes
-------------

this importer reads an ASE file and sets up the geometry considering also a second texture 
coordinate channel ( is usefull for reading files which include base and lightmap textures ).

be aware that the importer has some limitations due to the ase reader lib ( see rewards ):

 - no ASE format checking, invalid format can cause a crash.

 - only one smoothing group per face is allowed


and some limitations due to my limited private time:

 - only geometry objects are imported

 - currently no vertex color import supported ( this may be added later ).

 - all materials in the scene are expected as submaterials in one main big material ( this
     is the way how Gile(s) exportes the ase file currently.


what's new
----------

this release has no further feature additions in comparison to version 1.0. the only difference
in this release is the integration of the source package into new ctd source directory structure of release 0.4.0.


special notes
-------------

the materials are set up as follow:

- the base texture is imported into diffuse map, channel 1

- the second ( lightmap ) channel is imported to self-illumination map, channel 3


build notes
-----------

there are vc7.1 project files to build the package. further you need 3ds max's sdk. set the include and lib paths of 
3ds max sdk into vc7.1 studio's project settins ( menu Tools/Options.../Projects/VC++ Directories )

before you begin to build the project, decompress the package libASE.tar.bz2 in lib directory.


install notes
-------------

1. copy the file 'CTD-ASEimporter5.dle' in utils/exporters directory into <3dsmax install path>/plugins

2. copy the file 'IGame.dll' in lib directory into <3dsmax install path>


now your 3ds max has an additional importer called 'CTD ASE importer'.


rewards
-------

- special thanks to Virtual Object System team ( http://interreality.org/index.html ) for their 
   libASE library which is used to parse and store the ase chunks. i slightly extended this lib 
   to read some more chunks.


