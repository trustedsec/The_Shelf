$MyPath = Split-Path $PSCommandPath
Set-Location $MyPath

$Source = 'TEMPLATE_MODULE'

If (-Not (Test-Path -Path $Source)){
    throw 'The source TEMPLATE does not exist'
}


$Destination = Read-Host -Prompt 'New module name'

If (Test-Path -Path $Destination){
   # Write-Error 'The module already exists'
   throw 'The module already exists'
}
Else {
    Write-Host 'Copying '$Source' to '$Destination''
}

Copy-Item -Path $Source -Destination $Destination -Recurse

Write-Host 'Batch renaming all files'

Get-ChildItem -Recurse -Path $Destination | ForEach-Object { If ($_.Name.Contains($Source)) { Rename-Item -Path $_.PSPath -NewName $_.Name.Replace($Source,$Destination) } }

Write-Host 'Batch replacing all file contents'

$Files = Get-ChildItem -File -Recurse -Path $Destination
ForEach ($File in $Files) { 
    (Get-Content $File.PSPath) | 
    ForEach-Object { $_ -replace $Source, $Destination } |
    Set-Content $File.PSPath
}

Write-Host 'Successfully created new module'

Get-ChildItem -Path $Destination

Exit

    