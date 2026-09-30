$ErrorActionPreference = 'Stop'

$DocumentPath = 'C:\Users\n4916\Desktop\新建文件夹 (4)\2026电赛H题_车载平衡滚球运动控制系统_设计报告_重写版.docx'
$BackupPath = 'C:\Users\n4916\Documents\TI 小车\report_work\backups\补充参考文献与附录前.docx'
$PdfPath = 'C:\Users\n4916\Documents\TI 小车\report_work\qa\references_appendices_review.pdf'
$FigureDir = 'C:\Users\n4916\Documents\TI 小车\report_work\figures'
$CircuitDir = 'C:\Users\n4916\Documents\TI 小车\report_work\circuit_sources'

$References = @'
五、参考文献
[1] TEXAS INSTRUMENTS. MSPM0G350x Mixed-Signal Microcontrollers With CAN-FD Interface: SLASEX6C[Z]. 2026.
[2] TEXAS INSTRUMENTS. MSPM0 G-Series 80-MHz Microcontrollers Technical Reference Manual: SLAU846A[Z]. 2023.
[3] TOSHIBA ELECTRONIC DEVICES & STORAGE CORPORATION. TB6612FNG Driver IC for Dual DC Motor[Z]. 2014.
[4] INVENSENSE. MPU-6000 and MPU-6050 Product Specification: PS-MPU-6000A-00, Revision 3.4[Z]. 2013.
[5] OPENMV. OpenMV Cam H7 Plus Quick Reference[EB/OL]. 2026. https://docs.openmv.io/.
[6] 深圳市小二极客科技有限公司. Robot-Link V5.0 MT规格书[Z]. 2022.
[7] WHEELTEC. C07A核心板原理图V1.1（MSPM0G3507）[Z]. 2025.
[8] WHEELTEC. C07A适配S28A底板原理图[Z]. 2025.
[9] WHEELTEC. C07A搭配S28A底板资源分配表[Z]. 2025.
'@

$Appendix1 = @'
附录1：电路原理图
（1）系统主要电气连接原理图
[[APP_SYS]]
图A1  系统主要电气连接原理图
[[PAGEBREAK1]]
（2）C07A核心板原理图
[[C07A]]
图A2  C07A核心板原理图
[[PAGEBREAK2]]
（3）C07A适配S28A底板原理图
[[S28A]]
图A3  C07A适配S28A底板原理图
[[PAGEBREAK3]]
（4）TB6612FNG双路直流电机驱动原理图
[[TB6612]]
图A4  TB6612FNG双路直流电机驱动原理图
'@

$Appendix2 = @'
附录2：源程序
本次电子提交仅包含TI MSPM0G3507主控工程，压缩包名称为“2026电赛H题_TI主控源码_MSPM0G3507.zip”。OpenMV视觉程序和RobotLinkRecorder上位机程序暂不列入本次源码附件。
1、工程说明
主控工程基于MSPM0G3507、TI MSPM0 SDK和SysConfig开发，提供Keil MDK工程文件。程序入口为empty.c，上电后完成时钟、GPIO、PWM、I²C、UART、编码器、MPU6050及OLED初始化，并以10 ms任务节拍运行循迹、速度闭环和任务状态机。
2、源文件组成
（1）Application目录：task_manager、task_1、task_2、task_3、line_follow和angle_turn，分别完成任务调度、三项赛题流程、循迹控制及角度转向。
（2）Hardware目录：board、tracking、mpu6050、imu、imu_uart_link、oled、key和led，完成板级接口、传感器采集、显示及人机交互。
（3）工程根目录：encoder、encoder_pid和motion实现编码器测速、速度PI及底盘运动控制；empty.syscfg与ti_msp_dl_config.c/h保存TI外设配置。
（4）keil目录：包含工程文件、启动文件和链接脚本，不包含Objects等编译中间文件。
3、关键接口配置
灰度循迹信号为PB16、PB17、PA22、PA24、PA27、PA9，S8起止信号为PA12；左右编码器为PA25/PA26和PB20/PB24；底盘PWM为PB2/PB3；MPU6050使用PA0/PA1的I²C和PA7中断；PB6/PB7以115200 bit/s与OpenMV通信。
4、编译与下载
使用Keil MDK打开keil目录中的工程文件，确认MSPM0 SDK与器件包路径后即可编译；通过C07A核心板SWD或板载下载接口烧录至MSPM0G3507。
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

function Normalize-Block([string]$Text) {
    return (($Text.Trim() -split '\r?\n') -join "`r") + "`r"
}

function Replace-To-NextSection($Document, [string]$StartText, [string]$EndText, [string]$Content) {
    $startRange = Find-Range $Document $StartText 0
    $endRange = Find-Range $Document $EndText 0
    if (-not $startRange -or -not $endRange) { throw "Cannot locate range: $StartText -> $EndText" }
    $startPos = $startRange.Paragraphs.Item(1).Range.Start
    $endPos = $endRange.Paragraphs.Item(1).Range.Start
    if ($endPos -le $startPos) { throw "Invalid range order: $StartText -> $EndText" }
    $breakPos = $endPos - 1
    if ($Document.Range($breakPos, $endPos).Text -ne [string][char]12) {
        throw "Expected section break before: $EndText"
    }
    $Document.Range($startPos, $breakPos).Text = Normalize-Block $Content
}

function Replace-To-End($Document, [string]$StartText, [string]$Content) {
    $startRange = Find-Range $Document $StartText 0
    if (-not $startRange) { throw "Cannot locate end block: $StartText" }
    $startPos = $startRange.Paragraphs.Item(1).Range.Start
    $endPos = $Document.Content.End - 1
    $Document.Range($startPos, $endPos).Text = Normalize-Block $Content
}

function Set-ParagraphStyle($Paragraph, [string]$Kind) {
    $r = $Paragraph.Range
    $f = $Paragraph.Format
    $r.Font.Name = '宋体'
    $r.Font.NameFarEast = '宋体'
    $r.Font.Size = 12
    $r.Font.Bold = 0
    $f.Alignment = 3
    $f.FirstLineIndent = 0
    $f.LeftIndent = 0
    $f.RightIndent = 0
    $f.SpaceBefore = 0
    $f.SpaceAfter = 0
    $f.LineSpacingRule = 5
    $f.LineSpacing = 15
    $f.PageBreakBefore = 0
    $f.KeepWithNext = 0
    $f.KeepTogether = 0

    switch ($Kind) {
        'chapter' {
            $r.Font.Size = 16; $r.Font.Bold = -1
            $f.Alignment = 3; $f.FirstLineIndent = 0
            $f.KeepWithNext = -1; $f.KeepTogether = -1
        }
        'reference' {
            $r.Font.Size = 12; $r.Font.Bold = 0
            $f.Alignment = 0; $f.LeftIndent = 21; $f.FirstLineIndent = -21
            $f.KeepTogether = -1
        }
        'subheading' {
            $r.Font.Size = 12; $r.Font.Bold = -1
            $f.Alignment = 3; $f.FirstLineIndent = 0
            $f.KeepWithNext = -1; $f.KeepTogether = -1
        }
        'body' {
            $r.Font.Size = 12; $r.Font.Bold = 0
            $f.Alignment = 3; $f.FirstLineIndent = 24
        }
        'caption' {
            $r.Font.Size = 10.5; $r.Font.Bold = 0
            $f.Alignment = 1; $f.FirstLineIndent = 0
            $f.KeepTogether = -1
        }
        'marker' {
            $f.Alignment = 1; $f.FirstLineIndent = 0
            $f.LineSpacingRule = 0; $f.LineSpacing = 12
            $f.KeepWithNext = -1; $f.KeepTogether = -1
        }
        'pagebreak' {
            $f.Alignment = 1; $f.FirstLineIndent = 0
            $f.LineSpacingRule = 0; $f.LineSpacing = 12
        }
    }
}

function Insert-Figure($Document, [string]$Marker, [string]$Path, [double]$WidthPoints, [string]$AltText) {
    if (-not (Test-Path -LiteralPath $Path)) { throw "Figure missing: $Path" }
    $markerRange = Find-Range $Document $Marker 0
    if (-not $markerRange) { throw "Figure marker missing: $Marker" }
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

function Replace-PageBreakMarker($Document, [string]$Marker) {
    $range = Find-Range $Document $Marker 0
    if (-not $range) { throw "Page-break marker missing: $Marker" }
    $startPos = $range.Paragraphs.Item(1).Range.Start
    $range.Paragraphs.Item(1).Range.Delete() | Out-Null
    $breakRange = $Document.Range($startPos, $startPos)
    $breakRange.InsertBreak(7)
}

if (-not (Test-Path -LiteralPath $DocumentPath)) { throw "Document missing: $DocumentPath" }
New-Item -ItemType Directory -Path (Split-Path -Parent $BackupPath) -Force | Out-Null
if (-not (Test-Path -LiteralPath $BackupPath)) { Copy-Item -LiteralPath $DocumentPath -Destination $BackupPath }

$wps = $null
$document = $null
$ownsWps = $false
$ownsDocument = $false
try {
    try { $wps = [Runtime.InteropServices.Marshal]::GetActiveObject('kwps.Application') } catch { }
    if ($wps) {
        for ($i = 1; $i -le $wps.Documents.Count; $i++) {
            $candidate = $wps.Documents.Item($i)
            if ($candidate.FullName -eq $DocumentPath) { $document = $candidate; break }
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

    Replace-To-NextSection $document '五、参考文献' '附录1：电路原理图' $References
    Replace-To-NextSection $document '附录1：电路原理图' '附录2：源程序' $Appendix1
    Replace-To-End $document '附录2：源程序' $Appendix2

    $refStart = (Find-Range $document '五、参考文献' 0).Paragraphs.Item(1).Range.Start
    $app1Start = (Find-Range $document '附录1：电路原理图' 0).Paragraphs.Item(1).Range.Start
    $app2Start = (Find-Range $document '附录2：源程序' 0).Paragraphs.Item(1).Range.Start
    foreach ($paragraph in $document.Paragraphs) {
        $pos = $paragraph.Range.Start
        if ($pos -lt $refStart) { continue }
        $text = $paragraph.Range.Text.Trim([char]13, [char]12, [char]7, [char]32)
        if ([string]::IsNullOrWhiteSpace($text)) { continue }
        if ($pos -lt $app1Start) {
            if ($text -eq '五、参考文献') { Set-ParagraphStyle $paragraph 'chapter' }
            elseif ($text -match '^\[\d+\]') { Set-ParagraphStyle $paragraph 'reference' }
        }
        elseif ($pos -lt $app2Start) {
            if ($text -match '^附录1：') { Set-ParagraphStyle $paragraph 'chapter' }
            elseif ($text -match '^（\d+）') { Set-ParagraphStyle $paragraph 'subheading' }
            elseif ($text -match '^图A\d+') { Set-ParagraphStyle $paragraph 'caption' }
            elseif ($text -match '^\[\[(APP_SYS|C07A|S28A|TB6612)\]\]$') { Set-ParagraphStyle $paragraph 'marker' }
            elseif ($text -match '^\[\[PAGEBREAK\d+\]\]$') { Set-ParagraphStyle $paragraph 'pagebreak' }
        }
        else {
            if ($text -match '^附录2：') { Set-ParagraphStyle $paragraph 'chapter' }
            elseif ($text -match '^\d+、') { Set-ParagraphStyle $paragraph 'subheading' }
            else { Set-ParagraphStyle $paragraph 'body' }
        }
    }

    Insert-Figure $document '[[APP_SYS]]' (Join-Path $FigureDir 'appendix_system_circuit.png') 453.5 '系统主要电气连接原理图'
    Insert-Figure $document '[[C07A]]' (Join-Path $CircuitDir 'c07a-1.png') 445.0 'C07A核心板原理图'
    Insert-Figure $document '[[S28A]]' (Join-Path $CircuitDir 's28a-1.png') 453.5 'C07A适配S28A底板原理图'
    Insert-Figure $document '[[TB6612]]' (Join-Path $FigureDir 'appendix_tb6612_cropped.png') 453.5 'TB6612FNG双路直流电机驱动原理图'

    Replace-PageBreakMarker $document '[[PAGEBREAK1]]'
    Replace-PageBreakMarker $document '[[PAGEBREAK2]]'
    Replace-PageBreakMarker $document '[[PAGEBREAK3]]'

    foreach ($caption in @('图A1  系统主要电气连接原理图','图A2  C07A核心板原理图','图A3  C07A适配S28A底板原理图','图A4  TB6612FNG双路直流电机驱动原理图')) {
        $range = Find-Range $document $caption 0
        if ($range) { Set-ParagraphStyle $range.Paragraphs.Item(1) 'caption' }
    }

    $chapter5 = Find-Range $document '五、参考文献' 0
    if ($chapter5) { $chapter5.Paragraphs.Item(1).Format.KeepWithNext = -1 }

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
