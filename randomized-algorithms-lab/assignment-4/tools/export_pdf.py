"""Export the report with the same Markdown, mathtext and Edge flow as assignment 3."""
import argparse
import base64
import io
import os
import re
import subprocess
import sys
from pathlib import Path

from run import BUILD, ROOT

EDGE = Path("C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe")
PREVIOUS_TOOLS = ROOT.parent/"assignment-3/build/pdf-tools"
STYLE = """
@page {size:A4;margin:17mm 16mm;}
* {box-sizing:border-box;}
body {font-family:"Microsoft YaHei","SimSun",sans-serif;color:#20242a;font-size:10.2pt;
      line-height:1.58;margin:0;overflow-wrap:anywhere;}
h1 {font-size:17pt;line-height:1.45;margin:0 0 14pt;color:#143b5f;}
h2 {font-size:14pt;color:#143b5f;border-bottom:1px solid #bacbd9;padding-bottom:4pt;
    margin:18pt 0 9pt;break-after:avoid;}
p {margin:6pt 0;orphans:3;widows:3;}
figure {break-inside:avoid;margin:9pt 0 12pt;text-align:center;}
figure img {display:block;max-width:100%;max-height:155mm;margin:0 auto;}
figcaption {font-size:9pt;margin-top:3pt;color:#475569;}
img.math {display:inline-block;vertical-align:-.25em;margin:0 1pt;max-width:100%;}
.equation {text-align:center;margin:10pt 0;break-inside:avoid;}
table {border-collapse:collapse;width:100%;font-size:8.2pt;line-height:1.45;margin:8pt 0;}
thead {display:table-header-group;}
tr {break-inside:avoid;}
th,td {border:1px solid #c8d2dd;padding:4pt 3pt;text-align:left;}
th {background:#eaf0f6;color:#143b5f;}
tbody tr:nth-child(even) {background:#f8fafc;}
pre {font-family:Consolas,"Microsoft YaHei",monospace;font-size:8.2pt;line-height:1.45;
     background:#f3f5f7;padding:8pt;white-space:pre-wrap;border:1px solid #d5dde6;break-inside:avoid;}
code {font-family:Consolas,"Microsoft YaHei",monospace;font-size:.94em;}
a {color:#205b87;text-decoration:none;}
"""


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--edge",type=Path,default=EDGE)
    parser.add_argument("--pdf-tools",type=Path,default=PREVIOUS_TOOLS)
    args = parser.parse_args()
    if args.pdf_tools.is_dir():
        sys.path.append(str(args.pdf_tools))
    os.environ.setdefault("MPLCONFIGDIR",str(ROOT/".mplconfig"))
    import markdown2
    from matplotlib.font_manager import FontProperties
    from matplotlib.mathtext import math_to_image

    report = ROOT/"docs/实验报告.md"
    text = report.read_text(encoding="utf-8")
    tokens = {}

    def token(html):
        key = f"REPORTPLACEHOLDER{len(tokens)}TOKEN"
        tokens[key] = html
        return key

    def formula(match, display=False):
        buffer = io.BytesIO()
        math_to_image("$"+match[1].strip()+"$",buffer,prop=FontProperties(size=11),format="svg")
        raw = buffer.getvalue()
        width = re.search(rb'width="([\d.]+)pt"',raw)[1].decode()
        height = re.search(rb'height="([\d.]+)pt"',raw)[1].decode()
        data = base64.b64encode(raw).decode()
        html = f'<img class="math" style="width:{width}pt;height:{height}pt" src="data:image/svg+xml;base64,{data}">'
        return "\n\n"+token(f'<div class="equation">{html}</div>')+"\n\n" if display else token(html)

    def figure(match):
        data = base64.b64encode((report.parent/match[2]).read_bytes()).decode()
        return token(f'<figure><img src="data:image/png;base64,{data}"><figcaption>{match[1]}</figcaption></figure>')

    text = re.sub(r"\$\$(.*?)\$\$",lambda m:formula(m,True),text,flags=re.S)
    text = re.sub(r"\$([^\n$]+)\$",formula,text)
    text = re.sub(r"^!\[([^]]+)\]\(([^)]+)\)$",figure,text,flags=re.M)
    body = markdown2.markdown(text,extras=["tables","fenced-code-blocks"])
    for key,html in tokens.items():
        if html.startswith(("<figure","<div")):
            body = body.replace(f"<p>{key}</p>",html)
        else:
            body = body.replace(key,html)
    assert "REPORTPLACEHOLDER" not in body
    BUILD.mkdir(exist_ok=True)
    html = BUILD/"report.html"
    html.write_text(f'<!doctype html><html lang="zh-CN"><meta charset="utf-8">'
                    f'<title>蒙特卡洛积分实验报告</title><style>{STYLE}</style><body>{body}</body></html>',
                    encoding="utf-8")
    pdf = BUILD/"report.pdf"
    pdf.unlink(missing_ok=True)
    cmd = [str(args.edge),"--headless","--disable-gpu","--no-first-run","--no-pdf-header-footer",
           "--disable-extensions",f"--user-data-dir={BUILD/'edge-profile'}",
           f"--print-to-pdf={pdf}",html.as_uri()]
    p = subprocess.run(cmd,capture_output=True,timeout=120)
    if p.returncode or not pdf.is_file():
        raise RuntimeError(p.stderr.decode(errors="replace"))
    pdf.replace(report.with_suffix(".pdf"))
    print("PASS PDF printed with embedded formulae and figures")


if __name__=="__main__":
    main()
