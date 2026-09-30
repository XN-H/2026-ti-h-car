$ErrorActionPreference = 'Stop'

$ReferencePath = 'C:\Users\n4916\Desktop\新建文件夹 (4)\2026设计报告参考模板.doc'
$OutputPath = 'C:\Users\n4916\Desktop\新建文件夹 (4)\2026电赛H题_车载平衡滚球运动控制系统_设计报告_重写版.docx'
$PdfPath = 'C:\Users\n4916\Documents\TI 小车\report_work\qa\2026电赛H题_设计报告_重写版.pdf'
$ExpectedHash = '48285CD8E3F57B1E8F4F3A86F0BB1E9E8E75A2028C9B757BC0DC5A0599790E06'

$Title = '车载平衡滚球运动控制系统（H题）'
$Abstract = '本设计研制了一种车载平衡滚球运动控制系统。系统以MSPM0G3507为主控制器，采用多路灰度传感器和编码器实现小车循迹与速度闭环；OpenMV H7 Plus完成钢球视觉定位，X42S闭环步进电机经齿轮齿条调节摆杆倾角，并引入MPU6050纵向加速度补偿车体运动扰动。独立摄像机和Robot-Link模块实现视频实时传输、录像与回放。测试表明，小车能够完成整圈循迹停车、A-B段动态平衡、整圈动态平衡及任意指定位置控制；O点至+5 cm再至-5 cm运动用时5.0 s，最大位置误差1.0 cm，停车位置偏差1.0 cm，各项功能达到题目要求。'
$Keywords = '关键词：循迹小车；钢球控制；机器视觉；闭环步进电机；无线图传'

$Chapter1 = @'
一、系统方案
本系统主要由小车运动控制模块、循迹检测模块、钢球视觉检测模块、摆杆执行机构、无线图像传输模块及电源模块组成，下面分别对各模块的方案进行论证与选择。
1、主控制器件的论证与选择
1.1.1 控制器选用
方案一：采用STM32F103微控制器。
STM32F103具有较成熟的开发环境和较丰富的应用资料，能够完成常规的电机控制和传感器采集。但本系统需要同时完成多路循迹信号采集、双路编码器测速、双电机PWM控制、惯性测量、串口通信、显示及任务管理，使用该方案需要重新设计软硬件接口。
方案二：采用C07A核心板上的MSPM0G3507微控制器。
MSPM0G3507具有较丰富的定时器、PWM、串口、I²C及GPIO资源，可以满足循迹检测、双轮调速、MPU6050数据采集、OLED显示和OpenMV通信等要求，同时便于利用SysConfig完成外设配置。
通过比较，本设计选择方案二，以MSPM0G3507作为小车运动控制系统的主控制器。
1.1.2 控制系统方案选择
方案一：采用单控制器集中控制。
由同一控制器完成小车循迹、编码器测速、钢球图像处理及摆杆控制，系统结构较为简单，但图像处理的数据量较大，会增加主控制器负担，影响底盘运动控制的实时性。
方案二：采用MSPM0G3507与OpenMV H7 Plus组成的分布式控制系统。
MSPM0G3507负责循迹、测速、底盘驱动、惯性测量和任务管理，OpenMV H7 Plus负责钢球检测和X42S闭环步进电机控制。两个控制器通过串口传递车体纵向加速度信息，使运动控制和视觉处理相互独立。
综合以上两种方案，本设计选择方案二。
2、钢球检测与摆杆执行机构的论证与选择
2.1.1 钢球检测方案选择
方案一：采用普通摄像机配合主控制器进行钢球检测。
该方案需要主控制器完成图像采集、存储和处理，外围接口较为复杂，并会占用较多的运算和存储资源，不利于同时进行小车运动控制。
方案二：采用OpenMV H7 Plus进行钢球检测。
OpenMV H7 Plus集成摄像头和图像处理功能，可以限定感兴趣区域并完成灰度图像处理、目标筛选及钢球位置计算，程序开发和参数调整较为方便。
通过比较，本设计选择方案二，采用OpenMV H7 Plus检测钢球位置。
2.1.2 摆杆执行机构方案选择
方案一：采用大扭矩舵机。
舵机控制方法简单，但其机械间隙和控制死区会影响小角度调节精度，难以满足钢球连续平衡控制的要求。
方案二：采用开环步进电机。
开环步进电机具有较高的位置分辨率，但小车运动产生的振动和负载变化可能造成电机失步，使实际位置与指令位置产生累积偏差。
方案三：采用X42S闭环步进电机。
X42S闭环步进电机具有位置反馈，能够降低失步对摆杆控制的影响，并可通过位置指令实现重复定位。电机经齿轮齿条机构带动摆杆升降，结构紧凑，便于控制摆杆倾角。
综合以上三种方案，本设计选择方案三。
3、循迹及图像传输方案的论证与选择
3.1.1 循迹检测方案选择
方案一：采用摄像头进行视觉循迹。
视觉循迹能够获得较完整的赛道信息，但容易受到环境光照变化影响，并需要占用较多的图像处理资源。
方案二：采用多路灰度传感器进行循迹。
多路灰度传感器响应速度快，信号处理简单。利用六路横向检测信号可以计算黑线相对小车中心的位置偏差，另一路信号用于识别起止位置，便于实现差速循迹和准确停车。
通过比较，本设计选择方案二，采用RYDZ八通道灰度传感器完成赛道检测。
3.1.2 无线图像传输方案选择
方案一：由OpenMV同时完成钢球检测和视频传输。
该方案使用摄像机数量较少，但图像传输会占用OpenMV的处理时间和通信资源，可能降低钢球检测及控制程序的实时性。
方案二：采用独立摄像机和Robot-Link V5.0 MT模块进行图像传输。
第二摄像机专门采集钢球运动画面，通过Robot-Link模块将视频传输至笔记本电脑；上位机软件完成实时显示、录像和截图，保存的视频可用于回放。该方案与钢球视觉控制相互独立，不影响OpenMV的检测帧率。
综合以上两种方案，本设计选择方案二。
'@

$Chapter2 = @'
二、系统理论分析与计算
1、小车循迹控制的分析
（1）灰度位置误差计算
RYDZ八通道灰度传感器中的六路横向探头用于检测赛道黑线。按照探头由左至右的位置，将六路检测量s₁～s₆对应的权值设为：
pᵢ={−2500，−1500，−500，500，1500，2500}
采用加权平均法计算黑线相对小车中心的位置误差：
eₗ(k)=Σsᵢ(k)pᵢ/Σsᵢ(k)
当黑线位于传感器中心时eₗ(k)接近0；黑线偏向左侧或右侧时，误差的符号和大小分别反映偏移方向与偏移程度。当六路探头均未检测到黑线时，控制器保持上一次有效偏差的方向，以提高弯道和短时丢线情况下的连续性。另一路灰度信号用于识别起止标志，配合状态机完成启动、计圈和停车控制。
（2）循迹方向控制
方向控制采用离散PD算法，根据当前偏差及偏差变化量计算左右轮差速修正量：
uₗ(k)=sat{[Kₚₗeₗ(k)+K_dₗ(eₗ(k)−eₗ(k−1))]/1000}
其中，比例环节保证小车及时向黑线中心修正，微分环节抑制转向过冲。直线及弯道运行时Kₚₗ取4～5，K_dₗ取2，差速修正量限幅为10。设小车线速度为v、角速度为ω、左右轮速度分别为v_R和v_L，轮距B=214.2 mm，则差速运动关系为：
v=(v_R+v_L)/2，ω=(v_R−v_L)/B
根据循迹修正量形成左右轮目标速度：
v_R*=v₀+Δu，v_L*=v₀−Δu
由此实现小车沿黑色引导线稳定行驶。
（3）车轮速度闭环控制
左右直流电机均带编码器。控制器每20 ms读取一次编码器增量，得到车轮实际速度，并采用增量式PI算法调节PWM占空比：
u(k)=u(k−1)+Kₚᵥ[eᵥ(k)−eᵥ(k−1)]+Kᵢᵥeᵥ(k)
其中Kₚᵥ=100，Kᵢᵥ=5，PWM输出限幅为0～7000。为克服电机静摩擦和左右驱动差异，右、左电机分别加入412和435的启动补偿。方向PD外环产生两轮目标速度，速度PI内环抑制电池电压、路面阻力和负载变化对车速的影响，从而提高循迹和停车位置的一致性。
2、钢球运动模型与控制分析
（1）钢球沿摆杆运动模型
设钢球质量为m、半径为r、转动惯量为I，钢球沿摆杆的位移为x，摆杆倾角为θ，车体沿摆杆方向的加速度为a。忽略滚动阻力并认为钢球纯滚动，可得：
(m+I/r²)ẍ=m(gsinθ−acosθ)
钢球近似为实心球，I=2mr²/5，因而：
ẍ=5(gsinθ−acosθ)/7
摆杆工作角度较小时，sinθ≈θ、cosθ≈1，模型可简化为：
ẍ≈5(gθ−a)/7
由上式可知，钢球加速度不仅由摆杆倾角决定，还受小车纵向加速度影响。因此，小车加速或制动时仅采用位置反馈会产生明显滞后，需要引入车体加速度前馈补偿。
（2）钢球视觉位置估计
OpenMV H7 Plus在固定感兴趣区域内采集灰度图像，通过阈值分割、形态学滤波和连通域筛选获得钢球中心横坐标u。经标定，静态任务中图像比例系数kₓ=26 pixel/cm，中点像素u₀=160，则钢球相对O点的位置为：
x=(u−u₀)/kₓ
因此+5 cm和−5 cm目标位置分别对应约290 pixel和30 pixel。为减小图像噪声及短时遮挡造成的抖动，控制器采用α-β跟踪器估计位置和速度。设采样周期Tₛ=25 ms，α=0.72，β=0.12，则：
x̂ₖ|ₖ₋₁=x̂ₖ₋₁+Tₛv̂ₖ₋₁
rₖ=xₖ−x̂ₖ|ₖ₋₁
x̂ₖ=x̂ₖ|ₖ₋₁+αrₖ，v̂ₖ=v̂ₖ₋₁+βrₖ/Tₛ
位置估计值用于比例和积分控制，速度估计值用于阻尼反馈，可降低钢球在目标位置附近的往复振荡。
（3）位置PID与加速度前馈控制
设钢球目标位置为x_r，位置误差e(k)=x_r−x̂ₖ，位置PID给出的倾角指令为：
θ_PID(k)=Kₚe(k)+KᵢΣe(k)Tₛ−K_dv̂ₖ
MSPM0G3507以10 ms为基本控制周期读取MPU6050纵向加速度，并通过串口将毫重力单位数据实时发送给OpenMV。加速度信号经零偏校正和一阶低通滤波处理：
a(k)=[a_mg(k)−a₀]g/1000
a_f(k)=λa_f(k−1)+(1−λ)a(k)，λ=0.85
由钢球模型可得抵消车体加速度扰动所需的前馈倾角：
θ_ff(k)=arctan[a_f(k)/g]
最终摆杆倾角指令为：
θ_c(k)=sat[θ_PID(k)+s_aθ_ff(k)]
其中s_a用于统一MPU6050安装方向与摆杆正方向。控制程序对θ_c进行幅值限制和变化率限制；视觉目标暂时丢失时保持短时预测，超过设定时间后令摆杆缓慢回到水平位置。该控制方法利用实时加速度前馈提前抵消小车启停扰动，再由位置PID消除模型误差和稳态位置偏差。
3、摆杆结构及执行机构的计算
（1）摆杆倾角与升降量关系
摆杆转轴到齿条升降作用点的有效长度L=250 mm，设齿条相对水平位置的升降量为h，则摆杆倾角为：
θ=arcsin(h/L)
摆杆铰点距安装基准面的高度为80 mm，齿条总长60 mm，机构有效升降行程为50 mm。因此理论最大倾角为：
θ_max=arcsin(50/250)=11.54°
实际控制中采用较小的倾角限幅，以避免钢球加速度过大或碰撞摆杆端部。
（2）步进电机脉冲与摆杆倾角换算
X42S闭环步进电机每转对应3200个控制脉冲。机构中电机转过180°即1600个脉冲时，齿条移动50 mm，因此齿条位移分辨率为：
Δh=50/1600=0.03125 mm/pulse
小角度条件下θ≈h/L，故单脉冲对应的摆杆角度约为：
Δθ≈0.03125/250=0.000125 rad=0.00716°
将期望倾角换算为相对水平零位的脉冲指令，可写为：
q=8000sinθ_c
加速度前馈对应的脉冲补偿量近似为：
q_ff=8000sinθ_ff≈8000a_f/g
OpenMV将位置PID输出与加速度前馈量叠加后换算为脉冲位置，随后进行位置限幅、变化率限制和零位校正，并发送给X42S闭环驱动器。闭环步进电机的位置反馈能够抑制车辆振动和负载变化造成的失步误差，提高摆杆重复定位精度。
'@

function Get-SharedSha256([string]$Path) {
    $stream = [System.IO.File]::Open($Path, [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read, [System.IO.FileShare]::ReadWrite)
    try {
        $sha = [System.Security.Cryptography.SHA256]::Create()
        try { return ([BitConverter]::ToString($sha.ComputeHash($stream))).Replace('-', '') }
        finally { $sha.Dispose() }
    }
    finally { $stream.Dispose() }
}

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

function Replace-AllText($Document, [string]$OldText, [string]$NewText) {
    $cursor = 0
    while ($true) {
        $found = Find-Range $Document $OldText $cursor
        if ($null -eq $found) { break }
        $start = $found.Start
        $found.Text = $NewText
        $cursor = $start + $NewText.Length
    }
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

function Set-ParagraphStyle($Paragraph, [string]$Kind) {
    $range = $Paragraph.Range
    $format = $Paragraph.Format
    $range.Font.Name = '宋体'
    $range.Font.NameFarEast = '宋体'
    $range.Font.Size = 12
    $range.Font.Bold = 0
    $range.Font.Italic = 0
    $format.SpaceBefore = 0
    $format.SpaceAfter = 0
    $format.LineSpacingRule = 4
    $format.LineSpacing = 22
    $format.Alignment = 3
    $format.LeftIndent = 0
    $format.RightIndent = 0
    $format.FirstLineIndent = 24
    $format.KeepWithNext = 0
    $format.KeepTogether = 0

    switch ($Kind) {
        'chapter' {
            $range.Font.Size = 16; $range.Font.Bold = -1
            $format.Alignment = 0; $format.FirstLineIndent = 0
            $format.KeepWithNext = -1; $format.KeepTogether = -1
        }
        'section' {
            $range.Font.Size = 14; $range.Font.Bold = -1
            $format.Alignment = 0; $format.FirstLineIndent = 0
            $format.KeepWithNext = -1; $format.KeepTogether = -1
        }
        'subsection' {
            $range.Font.Size = 12; $range.Font.Bold = -1
            $format.Alignment = 0; $format.FirstLineIndent = 0
            $format.KeepWithNext = -1; $format.KeepTogether = -1
        }
        'scheme' {
            $format.FirstLineIndent = 0
        }
        'formula' {
            $range.Font.Name = 'Cambria Math'; $range.Font.NameFarEast = '宋体'
            $format.Alignment = 1; $format.FirstLineIndent = 0
            $format.KeepTogether = -1
        }
        'keyword' {
            $format.Alignment = 0; $format.FirstLineIndent = 0
        }
        'abstract' {
            $format.Alignment = 3; $format.FirstLineIndent = 24
        }
    }
}

function Format-ApprovedRange($Document) {
    $begin = Find-Range $Document '一、系统方案' 0
    $finish = Find-Range $Document '三、电路与程序设计' 0
    if ($null -eq $begin -or $null -eq $finish) { throw 'Approved body range not found.' }
    $bodyRange = $Document.Range($begin.Paragraphs.Item(1).Range.Start, $finish.Paragraphs.Item(1).Range.Start)
    $formulaPattern = '^(pᵢ=|eₗ\(|uₗ\(|v=|v_R\*|u\(k\)=|\(m\+I/|ẍ=|ẍ≈|x=|x̂|rₖ=|θ_PID|a\(k\)=|a_f|θ_ff|θ_c|θ=|θ_max|Δh=|Δθ|q=|q_ff)'
    foreach ($paragraph in $bodyRange.Paragraphs) {
        $text = $paragraph.Range.Text.Trim([char]13, [char]7, [char]32)
        if ([string]::IsNullOrWhiteSpace($text)) { continue }
        if ($text -match '^[一二三四五]、') { Set-ParagraphStyle $paragraph 'chapter' }
        elseif ($text -match '^\d+、') { Set-ParagraphStyle $paragraph 'section' }
        elseif ($text -match '^\d+\.\d+\.\d+' -or $text -match '^（\d+）') { Set-ParagraphStyle $paragraph 'subsection' }
        elseif ($text -match '^方案[一二三]：') { Set-ParagraphStyle $paragraph 'scheme' }
        elseif ($text -match $formulaPattern) { Set-ParagraphStyle $paragraph 'formula' }
        else { Set-ParagraphStyle $paragraph 'body' }
    }
}

if ((Get-SharedSha256 $ReferencePath) -ne $ExpectedHash) {
    throw 'The retained template hash changed. Re-distillation is required.'
}
if (Test-Path -LiteralPath $OutputPath) {
    throw "Output already exists; refusing to overwrite: $OutputPath"
}
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $PdfPath) | Out-Null

$wps = $null
$source = $null
$document = $null
try {
    $wps = New-Object -ComObject kwps.Application
    $wps.Visible = $false
    $wps.DisplayAlerts = 0

    $source = $wps.Documents.Open($ReferencePath, $false, $true)
    $source.SaveAs2($OutputPath, 12)
    $source.Close(0)
    $source = $null

    $document = $wps.Documents.Open($OutputPath, $false, $false)

    Replace-AllText $document 'XXX系统（A/B/C…/H题）' $Title

    $abstractHeading = Find-Range $document '摘    要' 0
    $keywordRange = Find-Range $document '关键词' 0
    if ($null -eq $abstractHeading -or $null -eq $keywordRange) { throw 'Abstract slots not found.' }
    $abstractStart = $abstractHeading.Paragraphs.Item(1).Range.End
    $keywordStart = $keywordRange.Paragraphs.Item(1).Range.Start
    $document.Range($abstractStart, $keywordStart).Text = $Abstract + "`r"
    $keywordRange = Find-Range $document '关键词' 0
    $keywordParagraph = $keywordRange.Paragraphs.Item(1).Range
    $keywordParagraph.Text = $Keywords + "`r"

    Replace-Block $document '一、系统方案' '二、系统理论分析与计算' $Chapter1
    Replace-Block $document '二、系统理论分析与计算' '三、电路与程序设计' $Chapter2

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

    $abstractBody = Find-Range $document $Abstract 0
    Set-ParagraphStyle $abstractBody.Paragraphs.Item(1) 'abstract'
    $keywordBody = Find-Range $document $Keywords 0
    Set-ParagraphStyle $keywordBody.Paragraphs.Item(1) 'keyword'

    $cursor = 0
    while ($true) {
        $titleRange = Find-Range $document $Title $cursor
        if ($null -eq $titleRange) { break }
        $p = $titleRange.Paragraphs.Item(1)
        $p.Range.Font.Name = '宋体'; $p.Range.Font.NameFarEast = '宋体'
        $p.Range.Font.Size = 18; $p.Range.Font.Bold = -1
        $p.Format.Alignment = 1; $p.Format.FirstLineIndent = 0
        $cursor = $titleRange.End
    }

    Format-ApprovedRange $document

    if ($document.Sections.Count -ge 3) {
        for ($i = 1; $i -le $document.Sections.Count; $i++) {
            $section = $document.Sections.Item($i)
            $footer = $section.Footers.Item(1)
            if ($i -le 2) {
                $footer.LinkToPrevious = $false
                $footer.Range.Text = ''
            }
            else {
                $footer.LinkToPrevious = ($i -gt 3)
                if ($i -eq 3) {
                    $footer.Range.Text = ''
                    [void]$footer.PageNumbers.Add(2, $true)
                    $footer.PageNumbers.RestartNumberingAtSection = $true
                    $footer.PageNumbers.StartingNumber = 1
                }
                $footer.Range.ParagraphFormat.Alignment = 2
                $footer.Range.Font.Name = 'Times New Roman'
                $footer.Range.Font.Size = 10.5
            }
        }
    }

    try { $document.RemoveDocumentInformation(99) } catch { }
    $document.Fields.Update() | Out-Null
    $document.Save()

    try {
        if (Test-Path -LiteralPath $PdfPath) { Remove-Item -LiteralPath $PdfPath -Force }
        $document.ExportAsFixedFormat($PdfPath, 17)
    }
    catch {
        "PDF_EXPORT_ERROR=$($_.Exception.Message)" | Write-Output
    }

    $pages = $document.ComputeStatistics(2)
    $sections = $document.Sections.Count
    $paragraphs = $document.Paragraphs.Count
    "OUTPUT=$OutputPath"
    "PAGES=$pages"
    "SECTIONS=$sections"
    "PARAGRAPHS=$paragraphs"
    "PDF=$PdfPath"
}
finally {
    if ($document) { $document.Close(0) }
    if ($source) { $source.Close(0) }
    if ($wps) { $wps.Quit() }
}
