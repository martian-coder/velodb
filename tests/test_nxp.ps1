# Test NXP Integration
$DataDir = "./test_nxp_data"
if (Test-Path $DataDir) { Remove-Item -Recurse -Force $DataDir }
New-Item -ItemType Directory -Path $DataDir

$ServerPath = "d:\Velodb\build\Release\velodb-server.exe"
$ClientPath = "d:\Velodb\build\Release\velodb-client.exe"

Write-Host "Starting VeloDB NXP Server..."
$ServerProcess = Start-Process $ServerPath -ArgumentList "--data-dir $DataDir --port 9001" -PassThru -WindowStyle Hidden
Start-Sleep -Seconds 2

try {
    Write-Host "Testing PUT operations..."
    1..100 | ForEach-Object {
        & $ClientPath --port 9001 PUT $_ ($_ * 10)
    }

    Write-Host "Testing GET operation..."
    $GetVal = & $ClientPath --port 9001 GET 50
    if ($GetVal -match "VALUE 500") {
        Write-Host "GET 50: SUCCESS"
    } else {
        Write-Error "GET 50: FAILED ($GetVal)"
        exit 1
    }

    Write-Host "Testing RANGE operation..."
    $RangeOutput = & $ClientPath --port 9001 RANGE 1 10
    if ($RangeOutput -match "RANGE 10") {
        Write-Host "RANGE test: SUCCESS"
    } else {
        Write-Error "RANGE test: FAILED`n$RangeOutput"
        exit 1
    }

    Write-Host "NXP Integration Test PASSED!"
} finally {
    Write-Host "Stopping Server..."
    Stop-Process -Id $ServerProcess.Id -Force
}
