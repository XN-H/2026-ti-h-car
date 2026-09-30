$ErrorActionPreference = 'Stop'

$DocumentPath = 'C:\Users\n4916\Desktop\新建文件夹 (4)\2026电赛H题_车载平衡滚球运动控制系统_设计报告_重写版.docx'
$BackupPath = 'C:\Users\n4916\Documents\TI 小车\report_work\backups\模板格式重排第四章前.docx'
$PdfPath = 'C:\Users\n4916\Documents\TI 小车\report_work\qa\chapter4_review.pdf'

$Chapter4 = @'
四、测试方案与测试结果
1、测试方案
（1）硬件测试
检查S28A连接、供电及共地状态，验证传感器、编码器、底盘、OpenMV、X42S和图传模块。
（2）软件仿真测试
模拟循迹偏差、车体加速度和目标位置输入，检查状态机、速度PI、位置PID、前馈及限幅逻辑。
（3）硬件软件联调
装车后依次完成空载循迹、A—B段平衡、携球整圈、指定位置和无线图传联调，每项独立测试3次。
2、测试条件与仪器
测试条件：室内普通照明、规定赛道，两块12 V电池供电，钢球初始位于O点。
测试仪器：卷尺、直尺、秒表、笔记本电脑及RobotLinkRecorder图传记录软件。
3、测试结果及分析
（1）测试结果(数据)
各项功能测试结果如下表所示（每项测试3次）：
[[TABLE1]]

（2）测试分析与结论
5项功能均测试3次并全部成功，成功率为100%。结论如下：
1、空载循迹一圈用时约16.0 s，停车偏差不超过1.0 cm。
2、A—B段用时7.0 s，携球整圈用时26.0 s，钢球偏移≤1.0 cm且未掉落。
3、指定位置用时5.0 s、误差1.0 cm；图传约10 frame/s且画面稳定。
综上所述，本设计达到设计要求。
'@

$TableData = @(
    @('测试项目', '空载循迹', 'A—B段平衡', '携球整圈', '指定位置', '无线图传'),
    @('测试结果', "16.0 s`v停车≤1.0 cm", "7.0 s`v偏移≤1.0 cm", "26.0 s`v未掉落`v偏移≤1.0 cm", "5.0 s`v误差1.0 cm", "10 frame/s`v画面稳定")
)

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

function Replace-Block($Document, [string]$StartText, [string]$EndText, [string]$Content) {
    $startRange = Find-Range $Document $StartText 0
    $endRange = Find-Range $Document $EndText 0
    if ($null -eq $startRange -or $null -eq $endRange) {
        throw "Cannot locate block: $StartText -> $EndText"
    }
    $startPos = $startRange.Paragraphs.Item(1).Range.Start
    $endPos = $endRange.Paragraphs.Item(1).Range.Start
    $normalized = (($Content.Trim() -split '\r?\n') -join "`r") + "`r"
    $Document.Range($startPos, $endPos).Text = $normalized
}

function Set-ParagraphStyle($Paragraph, [string]$Kind) {
    $r = $Paragraph.Range
    $f = $Paragraph.Format
    $r.Font.Name = '宋体'
    $r.Font.NameFarEast = '宋体'
    $r.Font.Size = 12
    $r.Font.Bold = 0
    $f.Alignment = 3
    $f.FirstLineIndent = 26.85
    $f.LeftIndent = 0
    $f.RightIndent = 0
    $f.SpaceBefore = 0
    $f.SpaceAfter = 0
    $f.LineSpacingRule = 5
    $f.LineSpacing = 15
    $f.KeepWithNext = 0
    $f.KeepTogether = 0

    switch ($Kind) {
        'chapter' {
            $r.Font.Size = 16; $r.Font.Bold = -1
            $f.Alignment = 3; $f.FirstLineIndent = 0
            $f.KeepWithNext = -1; $f.KeepTogether = -1
        }
        'section' {
            $r.Font.Size = 14; $r.Font.Bold = -1
            $f.Alignment = 3; $f.FirstLineIndent = 0
            $f.KeepWithNext = -1; $f.KeepTogether = -1
        }
        'plan_sub' {
            $r.Font.Size = 12; $r.Font.Bold = 0
            $f.Alignment = 3; $f.FirstLineIndent = 26.85
            $f.KeepWithNext = -1; $f.KeepTogether = -1
        }
        'result_sub' {
            $r.Font.Size = 12; $r.Font.Bold = -1
            $f.Alignment = 3; $f.FirstLineIndent = 0
            $f.KeepWithNext = -1; $f.KeepTogether = -1
        }
        'marker' {
            $f.Alignment = 1; $f.FirstLineIndent = 0
            $f.LineSpacingRule = 5; $f.LineSpacing = 15
            $f.KeepWithNext = -1; $f.KeepTogether = -1
        }
    }
}

function Insert-ResultsTable($Document, [string]$Marker, $Data) {
    $markerRange = Find-Range $Document $Marker 0
    if ($null -eq $markerRange) { throw "Table marker missing: $Marker" }
    $insertPos = $markerRange.Paragraphs.Item(1).Range.Start
    $markerRange.Text = ''
    $insertRange = $Document.Range($insertPos, $insertPos)
    $table = $Document.Tables.Add($insertRange, $Data.Count, $Data[0].Count)
    $table.AllowAutoFit = $false
    $table.AutoFitBehavior(0)
    $table.PreferredWidthType = 3
    $table.PreferredWidth = 453.5
    $table.Rows.AllowBreakAcrossPages = 0
    $table.Rows.Item(1).HeadingFormat = -1
    $table.TopPadding = 2
    $table.BottomPadding = 2
    $table.LeftPadding = 3
    $table.RightPadding = 3
    $table.Borders.Enable = 1

    $widths = @(73.5, 76.0, 76.0, 76.0, 76.0, 76.0)
    for ($c = 1; $c -le $widths.Count; $c++) {
        $table.Columns.Item($c).Width = $widths[$c - 1]
    }

    for ($r = 1; $r -le $Data.Count; $r++) {
        for ($c = 1; $c -le $Data[0].Count; $c++) {
            $cell = $table.Cell($r, $c)
            $cell.Range.Text = $Data[$r - 1][$c - 1]
            $cell.VerticalAlignment = 1
            $cell.Range.Font.Name = '宋体'
            $cell.Range.Font.NameFarEast = '宋体'
            $cell.Range.Font.Size = 10
            $cell.Range.Font.Bold = 0
            $cell.Range.ParagraphFormat.Alignment = 1
            $cell.Range.ParagraphFormat.FirstLineIndent = 0
            $cell.Range.ParagraphFormat.SpaceBefore = 0
            $cell.Range.ParagraphFormat.SpaceAfter = 0
            $cell.Range.ParagraphFormat.LineSpacingRule = 5
            $cell.Range.ParagraphFormat.LineSpacing = 13
        }
    }
    return $table
}

if (-not (Test-Path -LiteralPath $DocumentPath)) { throw "Document not found: $DocumentPath" }
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $BackupPath) | Out-Null
if (-not (Test-Path -LiteralPath $BackupPath)) {
    Copy-Item -LiteralPath $DocumentPath -Destination $BackupPath
}

$wps = $null
$document = $null
$ownsWps = $false
$ownsDocument = $false
try {
    try { $wps = [Runtime.InteropServices.Marshal]::GetActiveObject('kwps.Application') } catch { }
    if ($wps) {
        for ($i = 1; $i -le $wps.Documents.Count; $i++) {
            $candidate = $wps.Documents.Item($i)
            if ($candidate.FullName -eq $DocumentPath) {
                $document = $candidate
                break
            }
        }
    }
    if (-not $wps) {
        $wps = New-Object -ComObject kwps.Application
        $wps.Visible = $false
        $ownsWps = $true
    }
    $wps.DisplayAlerts = 0
    if (-not $document) {
        $document = $wps.Documents.Open($DocumentPath, $false, $false)
        $ownsDocument = $true
    }

    Replace-Block $document '四、测试方案与测试结果' '五、参考文献' $Chapter4

    $start = Find-Range $document '四、测试方案与测试结果' 0
    $end = Find-Range $document '五、参考文献' 0
    $chapterRange = $document.Range($start.Paragraphs.Item(1).Range.Start, $end.Paragraphs.Item(1).Range.Start)
    foreach ($paragraph in $chapterRange.Paragraphs) {
        $text = $paragraph.Range.Text.Trim([char]13, [char]7, [char]32)
        if ([string]::IsNullOrWhiteSpace($text)) { continue }
        if ($text -match '^四、') { Set-ParagraphStyle $paragraph 'chapter' }
        elseif ($text -in @('1、测试方案', '2、测试条件与仪器', '3、测试结果及分析')) { Set-ParagraphStyle $paragraph 'section' }
        elseif ($text -in @('（1）硬件测试', '（2）软件仿真测试', '（3）硬件软件联调')) { Set-ParagraphStyle $paragraph 'plan_sub' }
        elseif ($text -in @('（1）测试结果(数据)', '（2）测试分析与结论')) { Set-ParagraphStyle $paragraph 'result_sub' }
        elseif ($text -match '^\[\[TABLE\d+\]\]$') { Set-ParagraphStyle $paragraph 'marker' }
        else { Set-ParagraphStyle $paragraph 'body' }
    }

    Insert-ResultsTable $document '[[TABLE1]]' $TableData | Out-Null
    $chapter5 = Find-Range $document '五、参考文献' 0
    if ($chapter5) {
        $chapter5.Paragraphs.Item(1).Format.KeepWithNext = 0
        $chapter5.Paragraphs.Item(1).Format.PageBreakBefore = 0
    }

    try { $document.RemoveDocumentInformation(99) } catch { }
    $document.Fields.Update() | Out-Null
    $document.Save()

    if (Test-Path -LiteralPath $PdfPath) { Remove-Item -LiteralPath $PdfPath -Force }
    $document.ExportAsFixedFormat($PdfPath, 17)

    "OUTPUT=$DocumentPath"
    "PAGES=$($document.ComputeStatistics(2))"
    "SECTIONS=$($document.Sections.Count)"
    "PARAGRAPHS=$($document.Paragraphs.Count)"
    "TABLES=$($document.Tables.Count)"
    "INLINE_SHAPES=$($document.InlineShapes.Count)"
    "PDF=$PdfPath"
}
finally {
    if ($document -and $ownsDocument) { $document.Close(0) }
    if ($wps -and $ownsWps) { $wps.Quit() }
}
