
# Integration test for VeloDB Server and Client

$BuildDir = "d:\Velodb\build\Release"
$ServerExe = "$BuildDir\velodb-server.exe"
$ClientExe = "$BuildDir\velodb-client.exe"
$TestDataDir = "d:\Velodb\build\integration_test_data"

# Clean up test data
if (Test-Path $TestDataDir) {
    Remove-Item -Path $TestDataDir -Recurse -Force
}
New-Item -ItemType Directory -Path $TestDataDir | Out-Null

Write-Host "Starting VeloDB Server..." -ForegroundColor Cyan
$ServerProcess = Start-Process -FilePath $ServerExe -ArgumentList "--data-dir", $TestDataDir, "--port", "9000" -PassThru -NoNewWindow -RedirectStandardOutput "$TestDataDir\server.log"

# Wait for server to start
Start-Sleep -Seconds 2

function Run-Client {
    param([string]$cmd)
    Write-Host "Running Client: $cmd" -ForegroundColor Gray
    $clArgs = $cmd.Split(" ")
    $output = & $ClientExe @clArgs
    $joined = ($output -join " ").Trim()
    Write-Host "Result: $joined" -ForegroundColor DarkGray
    return $joined
}

try {
    # Test PUT
    $res = Run-Client "put 1 100"
    if ($res -ne "OK") { throw "PUT failed: $res" }

    $res = Run-Client "put 2 200"
    if ($res -ne "OK") { throw "PUT failed: $res" }

    # Test GET
    $res = Run-Client "get 1"
    if ($res -ne "VALUE 100") { throw "GET 1 failed: $res" }

    $res = Run-Client "get 2"
    if ($res -ne "VALUE 200") { throw "GET 2 failed: $res" }

    $res = Run-Client "get 3"
    if ($res -ne "NOT_FOUND") { throw "GET 3 should be NOT_FOUND: $res" }

    # Test RANGE
    $res = Run-Client "range 1 2"
    if ($res -notlike "RANGE 2*") { throw "RANGE failed: $res" }

    # Test BACKUP
    $backupPath = "$TestDataDir\backup.bin"
    $res = Run-Client "backup $backupPath"
    if ($res -ne "BACKUP_OK") { throw "BACKUP failed: $res" }
    if (-not (Test-Path $backupPath)) { throw "Backup file not created" }

    Write-Host "`nIntegration Test PASSED!" -ForegroundColor Green
}
catch {
    Write-Host "`nIntegration Test FAILED: $_" -ForegroundColor Red
    exit 1
}
finally {
    Write-Host "Stopping Server..." -ForegroundColor Cyan
    Stop-Process -Id $ServerProcess.Id -Force
}
