from pathlib import Path
from pypdf import PdfReader

pdf = Path(r"C:\Users\n4916\Documents\TI 小车\report_work\qa\chapter4_review.pdf")
reader = PdfReader(str(pdf))
text = "\n".join(page.extract_text() or "" for page in reader.pages)
checks = {
    "pages": len(reader.pages),
    "figure_markers": text.count("[[FIG"),
    "table_markers": text.count("[[TABLE"),
    "xxxx": text.count("XXXX"),
    "fpga": text.count("FPGA"),
    "format_instructions": text.count("格式说明"),
    "chapter4": text.count("四、测试方案与测试结果"),
    "chapter5": text.count("五、参考文献"),
    "success_rows": text.count("100%"),
    "frame_rate": text.count("10 frame/s"),
}
for key, value in checks.items():
    print(f"{key}={value}")
