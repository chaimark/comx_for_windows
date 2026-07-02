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
$brace_level = 0
$local_arrays = New-Object System.Collections.Generic.HashSet[string]   # 仅自动（栈）数组
$static_arrays = New-Object System.Collections.Generic.HashSet[string]  # static 数组
$tainted_vars = New-Object System.Collections.Generic.HashSet[string]
$total_errors = 0

$line_num = 0
foreach ($line in $lines) {
    $line_num++
    $line = $line.Trim()
    if (-not $line) { continue }

    # 1. 检测函数头
    if ($line -match '^\s*(?:static|extern|inline)?\s*[\w\*]+\s+(\w+)\s*\([^)]*\)\s*\{') {
        $current_function = $Matches[1]
        $brace_level = 1
        $local_arrays.Clear()
        $static_arrays.Clear()
        $tainted_vars.Clear()
        continue
    }

    # 2. 更新花括号深度
    if ($current_function -ne "Global") {
        $open = ($line -replace "[^{]", "").Length
        $close = ($line -replace "[^}]", "").Length
        $brace_level += ($open - $close)

        if ($brace_level -eq 0) {
            $current_function = "Global"
            continue
        }
    }

    if ($current_function -eq "Global") { continue }

    # 3. 检测局部数组声明（区分 static 和自动）
    if ($line -match '\b(static\s+)?(char|int\d*_t|uint\d*_t|int)\s+(\w+)\s*\[[^\]]+\]') {
        $array_name = $Matches[3]
        if ($Matches[1] -and $Matches[1].Trim() -eq "static") {
            [void]$static_arrays.Add($array_name)
        } else {
            [void]$local_arrays.Add($array_name)
        }
    }

    # 4. 检测 newString（暂未使用）
    if ($line -match '\bnewString\s*\(\s*(\w+)') {
        [void]$tainted_vars.Add($Matches[1])
    }

    # 5. 检测 NEW_NAME 赋值（仅当源是自动数组时标记污染）
    if ($line -match '(\w+)\s*=\s*NEW_NAME\s*\(\s*(\w+)\s*\)') {
        $var = $Matches[1]
        $src = $Matches[2]
        if ($local_arrays.Contains($src)) {   # 只关心自动数组
            [void]$tainted_vars.Add($var)
        }
        # 若 src 在 static_arrays 中，则不污染
    }

    # 6. 污染传播（赋值）
    if ($line -match '(?:strnew\s*\*?\s*)?(\w+)\s*=\s*&?(\w+)\b') {
        $dest = $Matches[1]
        $src = $Matches[2]
        if ($tainted_vars.Contains($src)) {
            [void]$tainted_vars.Add($dest)
        }
    }

    # 7. 检查逃逸点：全局变量赋值
    if ($line -match '\b(g_\w+|g_saved_str)\s*=\s*&?(\w+)\b') {
        $glob = $Matches[1]
        $src = $Matches[2]
        if ($tainted_vars.Contains($src)) {
            Write-Host "${FilePath}(${line_num}): " -NoNewline
            Write-Host "warning: " -ForegroundColor Magenta -NoNewline
            Write-Host "[function $current_function] Variable '$src' assigned to global '$glob'"
            $total_errors++
        }
    }

    # 8. 检查逃逸点：返回栈数据（只针对污染变量）
    if ($line -match '\breturn\s+(\w+)\b') {
        $ret_var = $Matches[1]
        if ($tainted_vars.Contains($ret_var)) {
            Write-Host "${FilePath}(${line_num}): " -NoNewline
            Write-Host "error: " -ForegroundColor Red -NoNewline
            Write-Host "[function $current_function] Variable '$ret_var' is stack data"
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