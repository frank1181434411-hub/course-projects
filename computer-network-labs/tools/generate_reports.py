"""Generate local Word reports from Markdown, using each course template's page
layout/styles. A fresh package excludes sample text, screenshots and identities.
Requires python-docx. Course templates and generated reports stay outside Git.
"""
import argparse
import copy
from pathlib import Path
import re
from docx import Document
from docx.shared import Pt, Inches
from docx.oxml.ns import qn
from docx.enum.style import WD_STYLE_TYPE

ROOT=Path(__file__).resolve().parents[1]
ITEMS=[
 ('lab01-ipv6-tunnel','实验一资料/学生实验报告模版（实验一）.docx','实验一-IPv6隧道-待验证.docx','网络设备认知与 IPv6 隧道设计'),
 ('lab02-winsock-basics','实验报告模板/实验报告（实验二）.docx','实验二-Winsock全双工通信.docx','网络编程基础'),
 ('lab03-windows-socket','实验报告模板/学生实验报告模版（实验三）.docx','实验三-WindowsSocket与FTP.docx','Windows Socket 编程'),
]

def plain(text):
    text=re.sub(r'\[([^]]+)\]\([^)]+\)',r'\1',text)
    return text.replace('**','').replace('`','')

def generate(materials,destination):
    destination.mkdir(parents=True,exist_ok=True)
    for lab,template,filename,title in ITEMS:
        template_doc=Document(materials/template)
        doc=Document()
        # Copy template formatting only, never its body/media or identity metadata.
        doc.part._styles_part._element=copy.deepcopy(template_doc.styles.element)
        section=copy.deepcopy(template_doc.sections[0]._sectPr)
        for tag in ['headerReference','footerReference']:
            for ref in list(section.findall(qn('w:'+tag))):section.remove(ref)
        current=doc.sections[0]._sectPr
        current.getparent().replace(current,section)
        normal=doc.styles['Normal'];normal.font.name='宋体';normal.font.size=Pt(12)
        normal.element.get_or_add_rPr().rFonts.set(qn('w:eastAsia'),'宋体')
        normal.paragraph_format.line_spacing=1.3
        for name in ['Title','Heading 1','Heading 2']:
            if name not in doc.styles:
                added=doc.styles.add_style(name,WD_STYLE_TYPE.PARAGRAPH)
                added.base_style=normal
            if name in doc.styles:
                doc.styles[name].font.name='黑体'
                doc.styles[name].element.get_or_add_rPr().rFonts.set(qn('w:eastAsia'),'黑体')
                doc.styles[name].font.size=Pt(22 if name=='Title' else 16 if name=='Heading 1' else 14)
                doc.styles[name].font.bold=True
        doc.core_properties.author=''
        doc.core_properties.last_modified_by=''
        doc.core_properties.title='计算机网络 '+title
        doc.core_properties.subject='课程实验报告；个人信息待填写'
        doc.add_heading('学生实验报告',0)
        for value in [f'实验课程名称：计算机网络',f'实验项目名称：{title}','专业名称：________________','班级：________________','学号：________________','学生姓名：________________','指导教师：________________','实验日期：2026 年 11 月 28 日','实验性质：设计型实验']:
            doc.add_paragraph(value)
        if lab.startswith('lab01'):doc.add_paragraph('状态：待实际搭建与验证，不能作为完整实验成果提交。')
        else:doc.add_paragraph('状态：本机实际构建与测试通过；个人信息请按实际情况填写。')
        doc.add_page_break()
        lines=(ROOT/lab/'report.md').read_text(encoding='utf-8').splitlines()
        code=False;table_lines=[]
        def flush_table():
            if not table_lines:return
            rows=[[plain(cell.strip()) for cell in line.strip().strip('|').split('|')] for line in table_lines]
            rows=[row for row in rows if not all(re.fullmatch(r'[:\- ]+',cell or ' ') for cell in row)]
            table=doc.add_table(rows=0,cols=max(map(len,rows)))
            if 'Table Grid' in doc.styles:table.style='Table Grid'
            for row in rows:
                cells=table.add_row().cells
                for index,value in enumerate(row):cells[index].text=value
            table_lines.clear()
        for line in lines:
            if line.startswith('```'):
                flush_table();code=not code;continue
            if code:
                p=doc.add_paragraph();p.paragraph_format.space_after=Pt(0)
                run=p.add_run(line);run.font.name='Consolas';run.font.size=Pt(9);continue
            if line.startswith('|'):
                table_lines.append(line);continue
            flush_table()
            image=re.fullmatch(r'!\[([^]]*)\]\(([^)]+)\)',line)
            if image:
                path=(ROOT/lab/image.group(2)).resolve()
                if path.exists():
                    width=min(6.2,(doc.sections[0].page_width-doc.sections[0].left_margin-doc.sections[0].right_margin)/914400)
                    doc.add_picture(str(path),width=Inches(width));doc.add_paragraph(image.group(1))
                continue
            if line.startswith('# '):continue
            if line.startswith('## '):doc.add_heading(plain(line[3:]),level=1)
            elif line.strip():doc.add_paragraph(plain(line))
        flush_table()
        path=destination/filename;doc.save(path)
        # Reopen to ensure the generated package and its relationships are valid.
        verify=Document(path)
        assert len(verify.paragraphs)>20
        print(path)

if __name__=='__main__':
    ap=argparse.ArgumentParser();ap.add_argument('--materials',required=True,type=Path);ap.add_argument('--destination',required=True,type=Path)
    args=ap.parse_args();generate(args.materials,args.destination)
