
$data_path = 'C:\Work\OSG\OpenSceneGraph-3.2.1\build\bin\data\models'
cd "C:\Work\OSG\OpenSceneGraph-3.2.1\build\bin"
$source = Get-ChildItem -Path $data_path -Filter "*"
$cmd  = $env:simex_dir + "\bin\release\conv.exe  "
$source |% { & $cmd @( ($data_path + "\" + $_.name  + "\" + $_.name  +".dae")  , ($data_path + "\" + $_.name + "\" + $_.name + ".osgb") ) }

