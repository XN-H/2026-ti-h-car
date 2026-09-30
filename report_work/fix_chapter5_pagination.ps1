$ErrorActionPreference = 'Stop'

$DocumentPath = 'C:\Users\n4916\Desktop\新建文件夹 (4)\2026电赛H题_车载平衡滚球运动控制系统_设计报告_重写版.docx'
$PdfPath = 'C:\Users\n4916\Documents\TI 小车\report_work\qa\chapter4_review.pdf'

$wps = [Runtime.InteropServices.Marshal]::GetActiveObject('kwps.Application')
$document = $null
for ($i = 1; $i -le $wps.Documents.Count; $i++) {
    $candidate = $wps.Documents.Item($i)
    if ($candidate.FullName -eq $DocumentPath) { $document = $candidate; break }
}
if (-not $document) { throw "Open report not found: $DocumentPath" }

$range = $document.Content
$find = $range.Find
$find.ClearFormatting()
$find.Text = '五、参考文献'
$find.Forward = $true
$find.Wrap = 0
if (-not $find.Execute()) { throw 'Heading not found: 五、参考文献' }

$paragraph = $range.Paragraphs.Item(1)
$paragraph.Format.KeepWithNext = 0
$paragraph.Format.KeepTogether = -1
$paragraph.Format.PageBreakBefore = 0

$document.Fields.Update() | Out-Null
$document.Save()
if (Test-Path -LiteralPath $PdfPath) { Remove-Item -LiteralPath $PdfPath -Force }
$document.ExportAsFixedFormat($PdfPath, 17)

"PAGES=$($document.ComputeStatistics(2))"
"SECTIONS=$($document.Sections.Count)"
"TABLES=$($document.Tables.Count)"
"INLINE_SHAPES=$($document.InlineShapes.Count)"
