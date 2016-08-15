mklink  %OSG_DIR%\OpenSceneGraph-3.2.1\build\bin\data\materials\sea\sea.frag     %~dp0\sea.frag 
mklink  %OSG_DIR%\OpenSceneGraph-3.2.1\build\bin\data\materials\grass\grass.frag %~dp0\grass.frag 
mklink  %OSG_DIR%\OpenSceneGraph-3.2.1\build\bin\data\materials\grass\grass.vert %~dp0\grass.vert 
mklink  %OSG_DIR%\OpenSceneGraph-3.2.1\build\bin\data\materials\misc\skinning.vert    %~dp0\skinning.vert 
mklink  %OSG_DIR%\OpenSceneGraph-3.2.1\build\bin\data\materials\misc\skinning_inst.vert    %~dp0\skinning_inst.vert 
mklink  %OSG_DIR%\OpenSceneGraph-3.2.1\build\bin\data\materials\misc\skinning_inst2.vert    %~dp0\skinning_inst2.vert 

IF NOT EXIST %OSG_DIR%\OpenSceneGraph-3.2.1\build\bin\data\materials\flame (
mkdir   %OSG_DIR%\OpenSceneGraph-3.2.1\build\bin\data\materials\flame
)

mklink  %OSG_DIR%\OpenSceneGraph-3.2.1\build\bin\data\materials\flame\flame.frag     %~dp0\flame.frag 
mklink  %OSG_DIR%\OpenSceneGraph-3.2.1\build\bin\data\materials\flame\flame.vert     %~dp0\flame.vert

IF NOT EXIST %OSG_DIR%\OpenSceneGraph-3.2.1\build\bin\data\materials\sp_lit (
mkdir   %OSG_DIR%\OpenSceneGraph-3.2.1\build\bin\data\materials\sp_lit
)

mklink  %OSG_DIR%\OpenSceneGraph-3.2.1\build\bin\data\materials\sp_lit\sp_lit.frag     %~dp0\sp_lit.frag 
mklink  %OSG_DIR%\OpenSceneGraph-3.2.1\build\bin\data\materials\sp_lit\sp_lit.vert     %~dp0\sp_lit.vert


IF NOT EXIST %OSG_DIR%\OpenSceneGraph-3.2.1\build\bin\data\materials\rope (
mkdir   %OSG_DIR%\OpenSceneGraph-3.2.1\build\bin\data\materials\rope
)


mklink  %OSG_DIR%\OpenSceneGraph-3.2.1\build\bin\data\materials\rope\rope.frag     %~dp0\rope.frag 
mklink  %OSG_DIR%\OpenSceneGraph-3.2.1\build\bin\data\materials\rope\rope.vert     %~dp0\rope.vert

@echo off
pause
