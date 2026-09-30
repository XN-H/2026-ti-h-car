$ErrorActionPreference = 'Stop'

$OutputPath = 'C:\Users\n4916\Desktop\新建文件夹 (4)\2026电赛H题_车载平衡滚球运动控制系统_设计报告_重写版.docx'
$PdfPath = 'C:\Users\n4916\Documents\TI 小车\report_work\qa\2026电赛H题_设计报告_重写版.pdf'

function Find-Range($Document, [string]$Text, [int]$Start = 0) {
    $range = $Document.Range($Start, $Document.Content.End)
    $find = $range.Find
    $find.ClearFormatting()
    $find.Text = $Text
    $find.Forward = $true
    $find.Wrap = 0
    $find.Format = $false
    if ($find.Execute()) { return $range }
    return $null
}

function Clear-BetweenHeadings($Document, [string]$StartText, [string]$EndText) {
    $startRange = Find-Range $Document $StartText 0
    $endRange = Find-Range $Document $EndText 0
    if ($null -eq $startRange -or $null -eq $endRange) {
        throw "Cannot locate pending slot: $StartText -> $EndText"
    }
    $startPos = $startRange.Paragraphs.Item(1).Range.End
    $endPos = $endRange.Paragraphs.Item(1).Range.Start
    if ($endPos -gt $startPos) { $Document.Range($startPos, $endPos).Text = '' }
}

function Format-Heading($Paragraph, [int]$Size) {
    $Paragraph.Range.Font.Name = '宋体'
    $Paragraph.Range.Font.NameFarEast = '宋体'
    $Paragraph.Range.Font.Size = $Size
    $Paragraph.Range.Font.Bold = -1
    $Paragraph.Format.Alignment = 0
    $Paragraph.Format.FirstLineIndent = 0
    $Paragraph.Format.SpaceBefore = 0
    $Paragraph.Format.SpaceAfter = 0
    $Paragraph.Format.LineSpacingRule = 4
    $Paragraph.Format.LineSpacing = 22
    $Paragraph.Format.KeepWithNext = -1
    $Paragraph.Format.KeepTogether = -1
}

$wps = $null
$document = $null
try {
    $wps = New-Object -ComObject kwps.Application
    $wps.Visible = $false
    $wps.DisplayAlerts = 0
    $document = $wps.Documents.Open($OutputPath, $false, $false)

    Clear-BetweenHeadings $document '三、电路与程序设计' '四、测试方案与测试结果'
    Clear-BetweenHeadings $document '四、测试方案与测试结果' '五、参考文献'

    $chapter5 = Find-Range $document '五、参考文献' 0
    $section3End = $document.Sections.Item(3).Range.End - 1
    if ($section3End -gt $chapter5.Paragraphs.Item(1).Range.End) {
        $document.Range($chapter5.Paragraphs.Item(1).Range.End, $section3End).Text = ''
    }

    $appendix2 = Find-Range $document '附录2：源程序' 0
    $documentEnd = $document.Content.End - 1
    if ($documentEnd -gt $appendix2.Paragraphs.Item(1).Range.End) {
        $document.Range($appendix2.Paragraphs.Item(1).Range.End, $documentEnd).Text = ''
    }

    foreach ($heading in @('三、电路与程序设计', '四、测试方案与测试结果', '五、参考文献', '附录1：电路原理图', '附录2：源程序')) {
        $range = Find-Range $document $heading 0
        if ($range) { Format-Heading $range.Paragraphs.Item(1) 16 }
    }

    $document.Fields.Update() | Out-Null
    $document.Save()
    if (Test-Path -LiteralPath $PdfPath) { Remove-Item -LiteralPath $PdfPath -Force }
    $document.ExportAsFixedFormat($PdfPath, 17)
    "OUTPUT=$OutputPath"
    "PAGES=$($document.ComputeStatistics(2))"
    "SECTIONS=$($document.Sections.Count)"
    "PARAGRAPHS=$($document.Paragraphs.Count)"
}
finally {
    if ($document) { $document.Close(0) }
    if ($wps) { $wps.Quit() }
}
