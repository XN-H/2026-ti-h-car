from pathlib import Path
from math import atan2, cos, sin, pi

from PIL import Image, ImageDraw, ImageFont


OUT = Path(r"C:\Users\n4916\Documents\TI 小车\report_work\figures")
OUT.mkdir(parents=True, exist_ok=True)

FONT_REG = r"C:\Windows\Fonts\msyh.ttc"
FONT_BOLD = r"C:\Windows\Fonts\msyhbd.ttc"

BLACK = (30, 36, 42)
BLUE = (44, 86, 137)
BLUE_LIGHT = (226, 237, 249)
GRAY = (110, 118, 126)
GRAY_LIGHT = (242, 244, 246)
GREEN_LIGHT = (229, 243, 235)
ORANGE_LIGHT = (252, 239, 219)
WHITE = (255, 255, 255)


def font(size, bold=False):
    return ImageFont.truetype(FONT_BOLD if bold else FONT_REG, size)


def multiline_center(draw, box, text, fnt, fill=BLACK, spacing=6):
    x1, y1, x2, y2 = box
    bounds = draw.multiline_textbbox((0, 0), text, font=fnt, spacing=spacing, align="center")
    w = bounds[2] - bounds[0]
    h = bounds[3] - bounds[1]
    draw.multiline_text(
        ((x1 + x2 - w) / 2, (y1 + y2 - h) / 2 - bounds[1]),
        text,
        font=fnt,
        fill=fill,
        spacing=spacing,
        align="center",
    )


def box(draw, xy, text, fill=GRAY_LIGHT, outline=BLACK, width=3, radius=16, size=31, bold=False):
    draw.rounded_rectangle(xy, radius=radius, fill=fill, outline=outline, width=width)
    multiline_center(draw, xy, text, font(size, bold=bold))


def arrow(draw, start, end, color=BLACK, width=4, label=None, label_offset=(0, -34), size=25):
    draw.line([start, end], fill=color, width=width)
    angle = atan2(end[1] - start[1], end[0] - start[0])
    head = 16
    left = (
        end[0] - head * cos(angle - pi / 6),
        end[1] - head * sin(angle - pi / 6),
    )
    right = (
        end[0] - head * cos(angle + pi / 6),
        end[1] - head * sin(angle + pi / 6),
    )
    draw.polygon([end, left, right], fill=color)
    if label:
        mx = (start[0] + end[0]) / 2 + label_offset[0]
        my = (start[1] + end[1]) / 2 + label_offset[1]
        bounds = draw.textbbox((0, 0), label, font=font(size))
        tw = bounds[2] - bounds[0]
        th = bounds[3] - bounds[1]
        draw.rounded_rectangle((mx - tw / 2 - 8, my - 5, mx + tw / 2 + 8, my + th + 5), radius=5, fill=WHITE)
        draw.text((mx - tw / 2, my), label, font=font(size), fill=color)


def title(draw, text, width):
    fnt = font(38, bold=True)
    b = draw.textbbox((0, 0), text, font=fnt)
    draw.text(((width - (b[2] - b[0])) / 2, 24), text, font=fnt, fill=BLACK)


def save(img, name):
    img.save(OUT / name, dpi=(300, 300), optimize=True)


def make_system_block():
    w, h = 1800, 940
    img = Image.new("RGB", (w, h), WHITE)
    d = ImageDraw.Draw(img)
    title(d, "车载平衡滚球运动控制系统总体框图", w)

    msp = (625, 150, 1175, 325)
    openmv = (625, 475, 1175, 645)
    box(d, msp, "C07A / MSPM0G3507\nS28A扩展板", BLUE_LIGHT, BLUE, size=36, bold=True)
    box(d, openmv, "OpenMV H7 Plus\n钢球视觉定位与控制", GREEN_LIGHT, BLUE, size=35, bold=True)

    left_boxes = [
        ((60, 115, 430, 215), "RYDZ灰度传感器\nS2～S8"),
        ((60, 255, 430, 355), "双路编码器"),
        ((60, 395, 430, 495), "MPU6050\n纵向加速度"),
        ((60, 535, 430, 635), "按键与任务选择"),
    ]
    for xy, text in left_boxes:
        box(d, xy, text, GRAY_LIGHT, GRAY, size=30)
        arrow(d, (xy[2], (xy[1] + xy[3]) / 2), (msp[0], min(max((xy[1] + xy[3]) / 2, msp[1] + 25), msp[3] - 25)), color=GRAY)

    tb = (1370, 120, 1740, 255)
    oled = (1370, 300, 1740, 405)
    x42 = (1370, 485, 1740, 635)
    box(d, tb, "TB6612FNG\n双直流电机", ORANGE_LIGHT, BLUE, size=31, bold=True)
    box(d, oled, "OLED显示", GRAY_LIGHT, BLUE, size=31)
    box(d, x42, "X42S闭环步进电机\n齿轮齿条调节摆杆", ORANGE_LIGHT, BLUE, size=30, bold=True)
    arrow(d, (msp[2], 215), (tb[0], 190), color=BLUE, label="PWM/方向")
    arrow(d, (msp[2], 285), (oled[0], 352), color=BLUE, label="显示数据", label_offset=(0, -24))
    arrow(d, (msp[0] + 275, msp[3]), (openmv[0] + 275, openmv[1]), color=BLUE, label="UART：a_y")
    arrow(d, (openmv[2], 565), (x42[0], 560), color=BLUE, label="UART1：P1/P0")

    camera = (60, 700, 380, 800)
    robot = (520, 700, 840, 800)
    laptop = (980, 700, 1300, 800)
    box(d, camera, "第二摄像机", GRAY_LIGHT, GRAY, size=30)
    box(d, robot, "Robot-Link V5.0 MT", BLUE_LIGHT, BLUE, size=29)
    box(d, laptop, "笔记本上位机\n显示/录像/回放", GRAY_LIGHT, GRAY, size=29)
    arrow(d, (camera[2], 750), (robot[0], 750), color=GRAY)
    arrow(d, (robot[2], 750), (laptop[0], 750), color=GRAY, label="Wi-Fi视频")

    power = (1390, 710, 1740, 815)
    box(d, power, "两块12 V电池\n+ 充电宝", GREEN_LIGHT, BLUE, size=30, bold=True)
    d.text((1340, 830), "电源分别向控制、执行和图传模块供电", font=font(24), fill=GRAY)

    save(img, "fig1_system_block.png")


def make_hardware_connections():
    w, h = 1800, 980
    img = Image.new("RGB", (w, h), WHITE)
    d = ImageDraw.Draw(img)
    title(d, "S28A扩展板主要硬件连接图", w)

    center = (620, 155, 1180, 720)
    box(d, center, "S28A扩展板\n+\nC07A核心板\nMSPM0G3507", BLUE_LIGHT, BLUE, size=42, bold=True)

    left = [
        ((45, 115, 500, 245), "RYDZ灰度：S2～S8\nPB16、PB17、PA22、PA24\nPA27、PA9、PA12"),
        ((45, 285, 500, 405), "编码器A：PA25/PA26\n编码器B：PB20/PB24"),
        ((45, 445, 500, 565), "MPU6050\nI²C：PA0/PA1，INT：PA7"),
        ((45, 605, 500, 725), "OLED\nPA28、PA31、PB14、PB15"),
    ]
    for xy, text in left:
        box(d, xy, text, GRAY_LIGHT, GRAY, size=27)
        arrow(d, (xy[2], (xy[1] + xy[3]) / 2), (center[0], (xy[1] + xy[3]) / 2), color=GRAY)

    right = [
        ((1300, 105, 1760, 280), "TB6612FNG\n方向：PA13/PA14、PA16/PA17\nPWM：PB2/PB3\n→ E01011199双电机"),
        ((1300, 335, 1760, 475), "OpenMV H7 Plus\nUART3 P4/P5 对接 PB7/PB6\n115200 bit/s"),
        ((1300, 530, 1760, 650), "任务按键：PB8\n启动/停止：灰度S8"),
    ]
    for xy, text in right:
        box(d, xy, text, ORANGE_LIGHT if xy == right[0][0] else GRAY_LIGHT, BLUE, size=27)
        arrow(d, (center[2], (xy[1] + xy[3]) / 2), (xy[0], (xy[1] + xy[3]) / 2), color=BLUE)

    p1 = (70, 800, 500, 920)
    p2 = (685, 800, 1115, 920)
    p3 = (1300, 800, 1730, 920)
    box(d, p1, "12 V电池1 → S28A\n主控、底盘及传感器", GREEN_LIGHT, BLUE, size=28, bold=True)
    box(d, p2, "12 V电池2 → 稳压模块\nOpenMV与X42S", GREEN_LIGHT, BLUE, size=28, bold=True)
    box(d, p3, "充电宝\n第二摄像机与Robot-Link", GREEN_LIGHT, BLUE, size=28, bold=True)
    independence = "三路电源相互独立，控制信号共地"
    ib = d.textbbox((0, 0), independence, font=font(24))
    d.text(((w - (ib[2] - ib[0])) / 2, 752), independence, font=font(24), fill=GRAY)

    save(img, "fig2_hardware_connections.png")


def flow_box(draw, cx, y, text, fill=GRAY_LIGHT, width=600, height=88, bold=False, size=27):
    xy = (cx - width // 2, y, cx + width // 2, y + height)
    box(draw, xy, text, fill, BLUE, size=size, bold=bold)
    return xy


def diamond(draw, cx, cy, text, width=330, height=120, size=26):
    pts = [(cx, cy - height // 2), (cx + width // 2, cy), (cx, cy + height // 2), (cx - width // 2, cy)]
    draw.polygon(pts, fill=ORANGE_LIGHT, outline=BLUE)
    draw.line(pts + [pts[0]], fill=BLUE, width=3)
    multiline_center(draw, (cx - width // 2, cy - height // 2, cx + width // 2, cy + height // 2), text, font(size, bold=True))
    return pts


def make_software_flow():
    w, h = 1800, 500
    img = Image.new("RGB", (w, h), WHITE)
    d = ImageDraw.Draw(img)
    title(d, "系统软件流程图", w)
    d.text((35, 128), "MSPM0", font=font(30, bold=True), fill=BLUE)
    d.text((35, 330), "OpenMV", font=font(30, bold=True), fill=BLUE)

    xs = [(190, 110, 520, 220), (590, 110, 920, 220), (990, 110, 1320, 220), (1390, 110, 1760, 220)]
    top_text = [
        "初始化外设\n陀螺仪标定",
        "10 ms调度\n灰度PD＋编码器PI",
        "发送纵向加速度\n刷新OLED与计时",
        "任务完成：短路制动\n否则返回调度",
    ]
    top_fill = [BLUE_LIGHT, GREEN_LIGHT, GRAY_LIGHT, ORANGE_LIGHT]
    for xy, text_value, fill_value in zip(xs, top_text, top_fill):
        box(d, xy, text_value, fill_value, BLUE, size=28, bold=True if xy == xs[0] else False)
    for a, b in zip(xs, xs[1:]):
        arrow(d, (a[2], 165), (b[0], 165), color=BLUE)

    ys = [(190, 300, 520, 420), (590, 300, 920, 420), (990, 300, 1320, 420), (1390, 300, 1760, 420)]
    bottom_text = [
        "采集ROI\n阈值分割筛选钢球",
        "α-β估计位置速度\n接收并滤波加速度",
        "位置PID＋前馈\n限幅与变化率限制",
        "UART控制X42S\n丢失超时后回平",
    ]
    bottom_fill = [BLUE_LIGHT, GRAY_LIGHT, GREEN_LIGHT, ORANGE_LIGHT]
    for xy, text_value, fill_value in zip(ys, bottom_text, bottom_fill):
        box(d, xy, text_value, fill_value, BLUE, size=28, bold=True if xy == ys[0] else False)
    for a, b in zip(ys, ys[1:]):
        arrow(d, (a[2], 360), (b[0], 360), color=BLUE)

    save(img, "fig3_software_flow.png")


if __name__ == "__main__":
    make_system_block()
    make_hardware_connections()
    make_software_flow()
    for path in sorted(OUT.glob("fig*.png")):
        print(f"{path.name}\t{path.stat().st_size}")
