
CTD scene exporter for 3ds max 5 ( also 5.1 )

version:  1.1

date:     10/15/2004

author:   a. botorabi

e-mail:   botorabi@gmx.net



release notes:
--------------

this exporter creates neoengine animation keyframe and mesh files out of a 3ds max model.


what's new
----------

this release has no further feature additions in comparison to version 1.0. the only differences
in this release are a) the integration of the source package into new ctd source directory structure of release 0.4.0.
and b) usage of static neoengine libs instead of dynamic ones.


special notes:
------------

1. use '~' as prefix in your mesh names to get their keyframes exported

2. the exporter needs the IGame.dll of 3ds max.


build notes
-----------

there are vc7.1 project files to build the package. further you need 3ds max's sdk. set the include and lib paths of 
3ds max sdk into vc7.1 studio's project settins ( menu Tools/Options.../Projects/VC++ Directories )


install notes
-------------

1. copy the file 'CTD-Kfanimexporter5.dle' in utils/exporters directory into <3dsmax install path>/plugins

2. copy the file 'IGame.dll' in lib directory into <3dsmax install path>



rewards
-------

- thanks to neoengine team for their excellent work; the exporter code is basing on neoengine's 'nsceexport' plugin code.

