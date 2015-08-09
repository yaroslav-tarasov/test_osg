@echo off

set osg_bin_path = %OSG_DIR%\OpenSceneGraph-3.2.1\build\bin

if not exist %osg_bin_path%\data md osg_bin_path\data
if not exist %osg_bin_path%\data\materials md osg_bin_path\data\materials
if not exist %osg_bin_path%\data\materials\blender md osg_bin_path\data\blender

@echo on

mklink  %osg_bin_path%\data\materials\blender\grass\grass.frag %~dp0\grass.frag 
mklink  %osg_bin_path%\data\materials\blender\grass\grass.geom %~dp0\grass.geom 
mklink  %osg_bin_path%\data\materials\blender\grass\grass.vert %~dp0\grass.vert

@echo off
pause
