import json
import os
import sys
from datetime import datetime

import matplotlib.pyplot as plt
import pandas as pd
from reportlab.lib import colors
from reportlab.lib.pagesizes import A4, landscape
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.lib.units import cm
from reportlab.platypus import Image, Paragraph, SimpleDocTemplate, Spacer, PageBreak
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont


# Try to register a Korean-capable TrueType font from common Windows locations.
# Falls back to Helvetica if none are found.
BASE_FONT = 'Helvetica'
_font_candidates = [r"C:\Windows\Fonts\malgun.ttf", r"C:\Windows\Fonts\malgunbd.ttf", r"C:\Windows\Fonts\ARIALUNI.TTF"]
for _fp in _font_candidates:
    try:
        if os.path.exists(_fp):
            pdfmetrics.registerFont(TTFont('ProjectKR', _fp))
            BASE_FONT = 'ProjectKR'
            break
    except Exception:
        # continue to next candidate
        pass


def load_json_results(json_path: str) -> pd.DataFrame:
    with open(json_path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    comb = data.get('combination_results', {})
    rows = []
    for key, metrics in comb.items():
        row = metrics.copy()
        row['combination'] = key
        # ensure numeric types
        rows.append(row)
    df = pd.DataFrame(rows)
    return df, data


def make_bar(df: pd.DataFrame, column: str, out_path: str, title: str, rotate_xticks=True):
    plt.figure(figsize=(10, 5))
    df_sorted = df.sort_values(column, ascending=False)
    x = df_sorted['combination']
    y = df_sorted[column]
    bars = plt.bar(x, y, color='tab:blue')
    plt.title(title)
    plt.ylabel(column)
    if rotate_xticks:
        plt.xticks(rotation=45, ha='right')
    plt.tight_layout()
    plt.savefig(out_path, dpi=150)
    plt.close()


def make_scatter(df: pd.DataFrame, xcol: str, ycol: str, out_path: str, title: str):
    plt.figure(figsize=(8, 6))
    plt.scatter(df[xcol], df[ycol], c='tab:green')
    for i, txt in enumerate(df['combination'].astype(str)):
        plt.annotate(txt, (df[xcol].iloc[i], df[ycol].iloc[i]), fontsize=8, alpha=0.8)
    plt.xlabel(xcol)
    plt.ylabel(ycol)
    plt.title(title)
    plt.tight_layout()
    plt.savefig(out_path, dpi=150)
    plt.close()


def build_pdf(markdown_path: str, images: list, out_pdf: str, meta: dict):
    doc = SimpleDocTemplate(out_pdf, pagesize=A4)
    styles = getSampleStyleSheet()
    # Use the registered font for Korean support
    title_style = ParagraphStyle('Title', parent=styles['Heading1'], alignment=0, fontSize=16, fontName=BASE_FONT)
    h2 = ParagraphStyle('h2', parent=styles['Heading2'], alignment=0, fontSize=12, fontName=BASE_FONT)
    normal = ParagraphStyle('Normal', parent=styles['Normal'], fontName=BASE_FONT)

    story = []

    # Title
    title_text = f"Weigher-Sorter 테스트 리포트 - {datetime.now().strftime('%Y-%m-%d %H:%M')}"
    story.append(Paragraph(title_text, title_style))
    story.append(Spacer(1, 12))

    # Meta
    if meta:
        meta_lines = []
        if 'test_summary' in meta:
            ts = meta['test_summary']
            meta_lines.append(f"총 실행 수: {ts.get('total_tests_run', '-')}")
            meta_lines.append(f"전체 성공률: {ts.get('overall_success_rate', '-')}")
            meta_lines.append(f"전체 소요 시간(초): {ts.get('duration_seconds', '-')}")
        for line in meta_lines:
            story.append(Paragraph(line, normal))
        story.append(Spacer(1, 12))

    # Markdown content (as plain paragraphs)
    if os.path.exists(markdown_path):
        with open(markdown_path, 'r', encoding='utf-8') as f:
            for line in f:
                s = line.strip()
                if not s:
                    story.append(Spacer(1, 6))
                elif s.startswith('# '):
                    story.append(Paragraph(s.lstrip('# ').strip(), title_style))
                elif s.startswith('## '):
                    story.append(Paragraph(s.lstrip('#').strip(), h2))
                else:
                    story.append(Paragraph(s, normal))
    story.append(PageBreak())

    # Images
    for img_path, caption in images:
        if os.path.exists(img_path):
            story.append(Paragraph(caption, h2))
            story.append(Spacer(1, 6))
            img = Image(img_path)
            # scale image to page width
            img.drawHeight = 12 * cm
            img.drawWidth = 18 * cm
            story.append(img)
            story.append(Spacer(1, 12))

    doc.build(story)


def main():
    # default paths
    default_json = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'result', 'test_runner', 'weigher_sorter_test_results_20250825_191332.json'))
    # the docs folder lives at the repository root (three levels up from this file)
    md_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'docs', '02_analysis_weigher_sorter_20250825', 'weigher_sorter_test_results_analysis.md'))
    # helpful debug output when running the script manually
    print('Using analysis markdown:', md_path, 'exists=', os.path.exists(md_path))
    out_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'result', 'pdf'))
    os.makedirs(out_dir, exist_ok=True)

    json_path = sys.argv[1] if len(sys.argv) > 1 else default_json
    out_pdf = os.path.join(out_dir, 'weigher_sorter_test_results_report.pdf')

    df, full = load_json_results(json_path)

    # ensure numeric columns
    for col in ['success_rate', 'avg_time_ms', 'memory_peak_kb']:
        if col in df.columns:
            df[col] = pd.to_numeric(df[col], errors='coerce')

    images = []
    img1 = os.path.join(out_dir, 'success_rate.png')
    make_bar(df, 'success_rate', img1, 'Success Rate by Combination')
    images.append((img1, '성공률(조합별)'))

    img2 = os.path.join(out_dir, 'avg_time_ms.png')
    make_bar(df, 'avg_time_ms', img2, 'Average Time (ms) by Combination')
    images.append((img2, '평균 처리 시간(밀리초) (조합별)'))

    img3 = os.path.join(out_dir, 'memory_peak_kb.png')
    make_bar(df, 'memory_peak_kb', img3, 'Memory Peak (KB) by Combination')
    images.append((img3, '피크 메모리 사용량 (KB) (조합별)'))

    img4 = os.path.join(out_dir, 'success_vs_time.png')
    make_scatter(df, 'avg_time_ms', 'success_rate', img4, 'Success Rate vs Average Time')
    images.append((img4, '성공률 vs 평균 처리시간'))

    build_pdf(md_path, images, out_pdf, full)

    print('PDF generated at', out_pdf)


if __name__ == '__main__':
    main()
