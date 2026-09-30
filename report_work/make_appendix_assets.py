from pathlib import Path
from PIL import Image, ImageDraw, ImageFont


OUT = Path(r"C:\Users\n4916\Documents\TI 小车\report_work\figures")
OUT.mkdir(parents=True, exist_ok=True)

FONT_REG = r"C:\Windows\Fonts\msyh.ttc"
FONT_BOLD = r"C:\Windows\Fonts\msyhbd.ttc"

NAVY = "#174A7E"
BLUE = "#2E75B6"
BLUE_LIGHT = "#EAF2FB"
GREEN = "#3C8D66"
GREEN_LIGHT = "#EAF6EF"
ORANGE = "#B86B24"
ORANGE_LIGHT = "#FFF1E2"
GRAY = "#5B6570"
GRAY_LIGHT = "#F2F4F6"
RED = "#B43A3A"
WHITE = "#FFFFFF"


def font(size, bold=False):
    return ImageFont.truetype(FONT_BOLD if bold else FONT_REG, size)


def box(draw, xy, title, body, fill, outline, title_size=34, body_size=26):
    x0, y0, x1, y1 = xy
    draw.rounded_rectangle(xy, radius=18, fill=fill, outline=outline, width=4)
    draw.text(((x0 + x1) / 2, y0 + 28), title, font=font(title_size, True), fill=NAVY, anchor="ma")
    if body:
        draw.multiline_text(
            ((x0 + x1) / 2, y0 + 86),
            body,
            font=font(body_size),
            fill="#1F2933",
            anchor="ma",
            align="center",
            spacing=10,
        )


def arrow(draw, start, end, label="", color=BLUE, width=5, label_offset=(0, -18)):
    draw.line([start, end], fill=color, width=width)
    x0, y0 = start
    x1, y1 = end
    dx, dy = x1 - x0, y1 - y0
    length = max((dx * dx + dy * dy) ** 0.5, 1)
    ux, uy = dx / length, dy / length
    px, py = -uy, ux
    tip = (x1, y1)
    left = (x1 - 22 * ux + 10 * px, y1 - 22 * uy + 10 * py)
    right = (x1 - 22 * ux - 10 * px, y1 - 22 * uy - 10 * py)
    draw.polygon([tip, left, right], fill=color)
    if label:
        mx = (x0 + x1) / 2 + label_offset[0]
        my = (y0 + y1) / 2 + label_offset[1]
        bbox = draw.multiline_textbbox((mx, my), label, font=font(22, True), anchor="mm", align="center", spacing=5)
        draw.rounded_rectangle((bbox[0] - 8, bbox[1] - 5, bbox[2] + 8, bbox[3] + 5), radius=8, fill=WHITE)
        draw.multiline_text((mx, my), label, font=font(22, True), fill=color, anchor="mm", align="center", spacing=5)


img = Image.new("RGB", (2400, 1550), WHITE)
d = ImageDraw.Draw(img)
d.text((1200, 45), "车载平衡滚球运动控制系统主要电气连接原理图", font=font(48, True), fill=NAVY, anchor="ma")

# Central controller.
main = (865, 260, 1515, 790)
box(
    d,
    main,
    "C07A核心板 + S28A扩展板",
    "MSPM0G3507\n循迹PD / 编码器PI / 状态机\nMPU6050采集与UART通信",
    BLUE_LIGHT,
    BLUE,
    38,
    28,
)

# Sensors and feedback on the left.
track = (70, 170, 700, 380)
encoder = (70, 435, 700, 620)
mpu = (70, 675, 700, 870)
box(d, track, "RYDZ八路灰度传感器", "S2~S7：PB16、PB17、PA22、PA24、PA27、PA9\nS8：PA12（起止识别）", GRAY_LIGHT, GRAY, 32, 23)
box(d, encoder, "左右轮编码器", "电机A：PA25 / PA26\n电机B：PB20 / PB24", GRAY_LIGHT, GRAY, 32, 25)
box(d, mpu, "MPU6050惯性传感器", "I²C：PA0(SDA) / PA1(SCL)\n数据就绪中断：PA7", GRAY_LIGHT, GRAY, 32, 25)

arrow(d, (track[2], 275), (main[0], 355), "数字输入")
arrow(d, (encoder[2], 525), (main[0], 500), "正交编码")
arrow(d, (mpu[2], 770), (main[0], 655), "I²C + INT")

# Actuators and communications on the right.
tb = (1710, 150, 2330, 405)
openmv = (1710, 475, 2330, 720)
x42 = (1710, 800, 2330, 1015)
video = (1710, 1100, 2330, 1335)
box(d, tb, "TB6612FNG双路直流电机驱动", "A：PA13/PA14，PWMA=PB2\nB：PA16/PA17，PWMB=PB3\n输出接左右底盘电机", ORANGE_LIGHT, ORANGE, 31, 23)
box(d, openmv, "OpenMV H7 Plus", "钢球视觉定位、α-β跟踪\n位置控制与加速度前馈", GREEN_LIGHT, GREEN, 34, 26)
box(d, x42, "X42S闭环步进电机", "UART1：P1(TX)→R/A/H\nP0(RX)←T/B/L，115200 bit/s\n齿轮齿条调节摆杆", ORANGE_LIGHT, ORANGE, 32, 23)
box(d, video, "第二摄像机 + Robot-Link V5.0 MT", "USB视频采集并通过Wi-Fi传输\n笔记本RobotLinkRecorder显示、录像", GRAY_LIGHT, GRAY, 30, 24)

arrow(d, (main[2], 360), (tb[0], 280), "PWM + 方向")
arrow(d, (main[2], 585), (openmv[0], 585), "PB6(TX)→P5(RX)\nPB7(RX)←P4(TX)\n115200 bit/s", label_offset=(0, -62))
arrow(d, (openmv[0] + 310, openmv[3]), (x42[0] + 310, x42[1]), "Emm_V5 UART命令", color=GREEN, label_offset=(150, 0))

# Power domains.
batt1 = (70, 1000, 690, 1175)
batt2 = (70, 1235, 690, 1425)
laptop = (865, 1115, 1515, 1325)
box(d, batt1, "12 V电池1（2600 mAh）", "S28A输入；为主控、底盘、传感器供电", GREEN_LIGHT, GREEN, 31, 24)
box(d, batt2, "12 V电池2（2600 mAh）", "经稳压模块为OpenMV与X42S供电", GREEN_LIGHT, GREEN, 31, 24)
box(d, laptop, "5 V充电宝与笔记本电脑", "充电宝为第二摄像机和Robot-Link供电\n笔记本通过Wi-Fi接收图像", BLUE_LIGHT, BLUE, 31, 24)

arrow(d, (batt1[2], 1080), (main[0], 745), "12 V / 5 V / 3.3 V", color=GREEN, label_offset=(-30, 20))
arrow(d, (laptop[2], 1205), (video[0], 1215), "5 V + Wi-Fi", color=BLUE)

d.rounded_rectangle((400, 1460, 2000, 1525), radius=16, fill="#FFF5F5", outline=RED, width=3)
d.text((1200, 1492), "三路电源相互独立；MSPM0G3507、OpenMV与X42S控制接口必须共地。", font=font(27, True), fill=RED, anchor="mm")

img.save(OUT / "appendix_system_circuit.png", dpi=(300, 300))

# Crop the vendor TB6612 schematic to its electrically meaningful area so the
# symbols and net names stay readable after placement on an A4 appendix page.
tb_source = Path(r"C:\Users\n4916\Documents\TI 小车\report_work\circuit_sources\tb6612-1.png")
if tb_source.exists():
    tb = Image.open(tb_source).convert("RGB")
    cropped = tb.crop((500, 220, 1510, 900))
    framed = Image.new("RGB", (cropped.width + 60, cropped.height + 60), WHITE)
    framed.paste(cropped, (30, 30))
    framed.save(OUT / "appendix_tb6612_cropped.png", dpi=(300, 300))
