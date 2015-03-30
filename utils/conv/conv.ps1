
$data_path = $env:osg_dir + '\OpenSceneGraph-3.2.1\build\bin\data\models'
$osg_path  = $env:osg_dir + '\OpenSceneGraph-3.2.1\build\bin'
cd $osg_path
$source = Get-ChildItem -Path $data_path -Filter "*"
$cmd  = $env:simex_dir + "\bin\release\conv.exe  "
$source |% { & $cmd @( ($data_path + "\" + $_.name  + "\" + $_.name  +".dae")  , ($data_path + "\" + $_.name + "\" + $_.name + ".osgb") ) }

$cmd2  = $env:simex_dir + "\bin\release\daeTest.exe  "
$source |% { & $cmd2 @( ($data_path + "\" + $_.name  + "\" + $_.name  +".dae") ) }
