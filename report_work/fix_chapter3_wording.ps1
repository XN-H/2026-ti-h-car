$ErrorActionPreference = 'Stop'

$docPath = 'C:\Users\n4916\Desktop\新建文件夹 (4)\2026电赛H题_车载平衡滚球运动控制系统_设计报告_重写版.docx'
$oldText = '两者通过串口交换纵向加速度信息'
$newText = 'MSPM0通过串口向OpenMV传递纵向加速度信息'

$wps = New-Object -ComObject kwps.Application
$wps.Visible = $false
try {
    $doc = $wps.Documents.Open($docPath)
    $range = $doc.Content
    $find = $range.Find
    $find.ClearFormatting()
    $find.Replacement.ClearFormatting()
    $replaced = $find.Execute($oldText, $false, $false, $false, $false, $false, $true, 1, $false, $newText, 2)
    if (-not $replaced) {
        throw "未在文档中找到待替换文字：$oldText"
    }
    $doc.Save()
    $doc.Close()
}
finally {
    $wps.Quit()
    [System.Runtime.InteropServices.Marshal]::ReleaseComObject($wps) | Out-Null
    [GC]::Collect()
    [GC]::WaitForPendingFinalizers()
}

Write-Output "Updated: $docPath"
