param (
    [string]$FilePath
)

if (-not $FilePath) {
    Write-Host "[ERROR] Use Rules: .\check_strnew.ps1 .\test_strnew.c" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path $FilePath)) {
    Write-Host "[ERROR] find file Fail: $FilePath" -ForegroundColor Red
    exit 1
}

$content = [System.IO.File]::ReadAllText((Resolve-Path $FilePath))
$content = $content -replace '//.*', ''
$content = $content -replace '/\*[\s\S]*?\*/', ''

$lines = $content -split "`r?`n"

$current_function = "Global"
$local_arrays = New-Object System.Collections.Generic.HashSet[string]
$tainted_vars = New-Object System.Collections.Generic.HashSet[string]
$total_errors = 0

$line_num = 0
foreach ($line in $lines) {
    $line_num++
    $line = $line.Trim()
    if (-not $line) { continue }

    if ($line -match '\b(strnew|void|int|char\*?)\s+(\w+)\s*\([^)]*\)\s*\{') {
        $current_function = $Matches[2]
        $local_arrays.Clear()
        $tainted_vars.Clear()
        continue
    }

    if ($line -eq "}") {
        $current_function = "Global"
        continue
    }

    if ($current_function -eq "Global") { continue }

    if ($line -match '\b(char|int\d*_t|uint\d*_t|int)\s+(\w+)\s*\[[^\]]+\]') {
        [void]$local_arrays.Add($Matches[2])
    }

    if ($line -match '\bnewString\s*\(\s*(\w+)') {
        [void]$tainted_vars.Add($Matches[1])
    }

    if ($line -match '(\w+)\s*=\s*NEW_NAME\s*\(\s*(\w+)\s*\)') {
        $var = $Matches[1]
        $src = $Matches[2]
        if ($local_arrays.Contains($src)) {
            [void]$tainted_vars.Add($var)
        }
    }

    if ($line -match '(?:strnew\s*\*?\s*)?(\w+)\s*=\s*&?(\w+)\b') {
        $dest = $Matches[1]
        $src = $Matches[2]
        if ($tainted_vars.Contains($src)) {
            [void]$tainted_vars.Add($dest)
        }
    }

    # 7. 检查逃逸点 A: 赋值给全局变量 (已修复冒号解析冲突)
    if ($line -match '\b(g_\w+|g_saved_str)\s*=\s*&?(\w+)\b') {
        $glob = $Matches[1]
        $src = $Matches[2]
        if ($tainted_vars.Contains($src)) {
            Write-Host "${FilePath}(${line_num}): " -NoNewline
            Write-Host "warning: " -ForegroundColor Magenta -NoNewline
            Write-Host "[function $current_function] Variable '$src' has been assigned to the global variable '$glob'"
            $total_errors++
        }
    }

    # 8. 检查逃逸点 B: 危险的 return 返回栈数据 (已修复冒号解析冲突)
    if ($line -match '\breturn\s+(\w+)\b') {
        $ret_var = $Matches[1]
        if ($tainted_vars.Contains($ret_var)) {
            Write-Host "${FilePath}(${line_num}): " -NoNewline
            Write-Host "error: " -ForegroundColor Red -NoNewline
            Write-Host "[function $current_function] The variable '$ret_var' is stack data"
            $total_errors++
        }
    }
}

if ($total_errors -eq 0) {
    exit 0
} else {
    Write-Host "=================================================================="
    Write-Host "[DONE] check over: find $total_errors errors" -ForegroundColor Red
    exit 1
}

