$ErrorActionPreference = 'Stop'

$DocumentPath = 'C:\Users\n4916\Desktop\新建文件夹 (4)\2026电赛H题_车载平衡滚球运动控制系统_设计报告_重写版.docx'
$BackupPath = 'C:\Users\n4916\Documents\TI 小车\report_work\backups\写入第三章前.docx'
$PdfPath = 'C:\Users\n4916\Documents\TI 小车\report_work\qa\chapter3_review.pdf'
$FigureDir = 'C:\Users\n4916\Documents\TI 小车\report_work\figures'

$Chapter3 = @'
三、电路与程序设计
1、电路的设计
（1）系统总体框图
本系统采用分布式控制结构。MSPM0G3507经S28A扩展板连接灰度传感器、编码器、MPU6050、OLED和TB6612FNG，完成循迹、速度闭环与任务管理；OpenMV H7 Plus完成钢球定位和摆杆控制；第二摄像机配合Robot-Link独立图传。总体框图如图1所示。
[[FIG1]]
图1  系统总体框图
（2）主控制及底盘驱动电路
TB6612FNG方向端接PA13、PA14、PA16、PA17，PWMA、PWMB接PB2、PB3；电机A、B编码器分别接PA25/PA26和PB20/PB24，由编码器反馈实现双轮速度闭环与差速转向。
RYDZ的S2～S7接PB16、PB17、PA22、PA24、PA27、PA9，S8接PA12；MPU6050的I²C和中断端接PA0、PA1、PA7；OLED接PA28、PA31、PB14、PB15。
（3）钢球检测与摆杆执行电路
MSPM0G3507经PB6（TX）、PB7（RX）与OpenMV UART3的P5（RX）、P4（TX）连接，以115200 bit/s传送纵向加速度。OpenMV将位置控制与加速度前馈叠加，经UART1的P1（TX）、P0（RX）向X42S发送绝对位置命令，再由齿轮齿条调节摆杆高度。主要连接如图2所示。
[[FIG2]]
图2  S28A扩展板主要硬件连接图
（4）电源
系统由两块12 V、2600 mAh电池和一块充电宝供电：电池1经S28A供主控、底盘及传感器，电池2经稳压模块供OpenMV和X42S，充电宝供第二摄像机与Robot-Link；各控制接口共地并滤波去耦。
2、程序的设计
（1）程序功能描述与设计思路
1）MSPM0G3507程序
MSPM0以10 ms状态机完成任务调度；循迹PD和编码器PI控制底盘，S8识别起止位置。纵向加速度发送至OpenMV，OLED每100 ms刷新，任务结束时短路制动。
2）OpenMV及图传程序
OpenMV经阈值分割和α-β跟踪估计钢球状态，将加速度前馈与位置控制量叠加后，通过UART绝对位置命令控制X42S，目标丢失超时回平；第二摄像机经Robot-Link向上位机图传。
（2）程序流程图
系统软件流程如图3所示。
[[FIG3]]
图3  系统软件流程图
'@

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

function Set-Style($Paragraph, [string]$Kind) {
    $r = $Paragraph.Range
    $f = $Paragraph.Format
    $r.Font.Name = '宋体'
    $r.Font.NameFarEast = '宋体'
    $r.Font.Size = 12
    $r.Font.Bold = 0
    $f.Alignment = 3
    $f.FirstLineIndent = 24
    $f.LeftIndent = 0
    $f.RightIndent = 0
    $f.SpaceBefore = 0
    $f.SpaceAfter = 0
    $f.LineSpacingRule = 4
    $f.LineSpacing = 22
    $f.KeepWithNext = 0
    $f.KeepTogether = 0

    switch ($Kind) {
        'chapter' {
            $r.Font.Size = 16; $r.Font.Bold = -1
            $f.Alignment = 0; $f.FirstLineIndent = 0
            $f.KeepWithNext = -1; $f.KeepTogether = -1
        }
        'section' {
            $r.Font.Size = 14; $r.Font.Bold = -1
            $f.Alignment = 0; $f.FirstLineIndent = 0
            $f.KeepWithNext = -1; $f.KeepTogether = -1
        }
        'subsection' {
            $r.Font.Size = 12; $r.Font.Bold = -1
            $f.Alignment = 0; $f.FirstLineIndent = 0
            $f.KeepWithNext = -1; $f.KeepTogether = -1
        }
        'caption' {
            $r.Font.Size = 10.5
            $f.Alignment = 1; $f.FirstLineIndent = 0
            $f.KeepTogether = -1
        }
        'marker' {
            $f.Alignment = 1; $f.FirstLineIndent = 0
            $f.LineSpacingRule = 0; $f.LineSpacing = 12
            $f.KeepWithNext = -1; $f.KeepTogether = -1
        }
    }
}

function Insert-Figure($Document, [string]$Marker, [string]$Path, [double]$WidthPoints, [string]$AltText) {
    $markerRange = Find-Range $Document $Marker 0
    if ($null -eq $markerRange) { throw "Figure marker missing: $Marker" }
    $paragraphStart = $markerRange.Paragraphs.Item(1).Range.Start
    $markerRange.Text = ''
    $insertRange = $Document.Range($paragraphStart, $paragraphStart)
    $shape = $Document.InlineShapes.AddPicture($Path, $false, $true, $insertRange)
    Add-Type -AssemblyName System.Drawing
    $image = [System.Drawing.Image]::FromFile($Path)
    try { $heightPoints = $WidthPoints * $image.Height / $image.Width }
    finally { $image.Dispose() }
    $shape.LockAspectRatio = 0
    $shape.Width = $WidthPoints
    $shape.Height = $heightPoints
    $shape.LockAspectRatio = -1
    try { $shape.AlternativeText = $AltText } catch { }
    $shape.Range.ParagraphFormat.Alignment = 1
    $shape.Range.ParagraphFormat.FirstLineIndent = 0
    $shape.Range.ParagraphFormat.SpaceBefore = 0
    $shape.Range.ParagraphFormat.SpaceAfter = 0
    $shape.Range.ParagraphFormat.LineSpacingRule = 0
    $shape.Range.ParagraphFormat.KeepWithNext = -1
    $shape.Range.ParagraphFormat.KeepTogether = -1
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

    Replace-Block $document '三、电路与程序设计' '四、测试方案与测试结果' $Chapter3

    $start = Find-Range $document '三、电路与程序设计' 0
    $end = Find-Range $document '四、测试方案与测试结果' 0
    $chapterRange = $document.Range($start.Paragraphs.Item(1).Range.Start, $end.Paragraphs.Item(1).Range.Start)
    foreach ($paragraph in $chapterRange.Paragraphs) {
        $text = $paragraph.Range.Text.Trim([char]13, [char]7, [char]32)
        if ([string]::IsNullOrWhiteSpace($text)) { continue }
        if ($text -match '^三、') { Set-Style $paragraph 'chapter' }
        elseif ($text -match '^\d+、') { Set-Style $paragraph 'section' }
        elseif ($text -match '^（\d+）' -or $text -match '^\d+）') { Set-Style $paragraph 'subsection' }
        elseif ($text -match '^图\d+') { Set-Style $paragraph 'caption' }
        elseif ($text -match '^\[\[FIG\d+\]\]$') { Set-Style $paragraph 'marker' }
        else { Set-Style $paragraph 'body' }
    }

    Insert-Figure $document '[[FIG1]]' (Join-Path $FigureDir 'fig1_system_block.png') 250.0 '车载平衡滚球运动控制系统总体框图'
    Insert-Figure $document '[[FIG2]]' (Join-Path $FigureDir 'fig2_hardware_connections.png') 250.0 'S28A扩展板主要硬件连接图'
    Insert-Figure $document '[[FIG3]]' (Join-Path $FigureDir 'fig3_software_flow.png') 390.0 'MSPM0G3507与OpenMV系统软件流程图'

    foreach ($caption in @('图1  系统总体框图', '图2  S28A扩展板主要硬件连接图', '图3  系统软件流程图')) {
        $range = Find-Range $document $caption 0
        if ($range) { Set-Style $range.Paragraphs.Item(1) 'caption' }
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
    "INLINE_SHAPES=$($document.InlineShapes.Count)"
    "PDF=$PdfPath"
}
finally {
    if ($document -and $ownsDocument) { $document.Close(0) }
    if ($wps -and $ownsWps) { $wps.Quit() }
}
