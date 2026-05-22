$trees = Get-Content -Path .\trees.cpp -Raw
$source = Get-Content -Path .\Source.cpp -Raw
$source = $source.Replace("void drawTree(float x, float z, int type)", $trees + "`r`nvoid drawTree(float x, float z, int type)")
Set-Content -Path .\Source.cpp -Value $source
