import os
import markdown
import subprocess
import tempfile

def convert_md_to_pdf(md_file_path, pdf_file_path):
    with open(md_file_path, 'r', encoding='utf-8') as f:
        md_content = f.read()

    # Convert Markdown to HTML
    html_content = markdown.markdown(md_content, extensions=['extra', 'codehilite', 'tables', 'toc'])

    # Premium CSS for better looks and Korean support
    full_html = f"""
    <!DOCTYPE html>
    <html lang="ko">
    <head>
    <meta charset="utf-8">
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Inter:wght@400;700&family=Noto+Sans+KR:wght@300;400;700&display=swap');
        
        body {{
            font-family: 'Inter', 'Noto Sans KR', 'Malgun Gothic', sans-serif;
            font-size: 11pt;
            line-height: 1.7;
            color: #24292e;
            max-width: 900px;
            margin: 0 auto;
            padding: 40px;
            background-color: white;
        }}
        h1, h2, h3, h4, h5, h6 {{
            font-weight: 700;
            line-height: 1.25;
            margin-top: 24px;
            margin-bottom: 16px;
            border-bottom: 1px solid #eaecef;
            padding-bottom: 0.3em;
        }}
        h1 {{ font-size: 2em; }}
        h2 {{ font-size: 1.5em; }}
        h3 {{ font-size: 1.25em; }}
        
        a {{ color: #0366d6; text-decoration: none; }}
        a:hover {{ text-decoration: underline; }}
        
        code {{
            font-family: 'SFMono-Regular', Consolas, 'Liberation Mono', Menlo, monospace;
            background-color: rgba(27,31,35,0.05);
            padding: 0.2em 0.4em;
            border-radius: 3px;
            font-size: 85%;
        }}
        pre {{
            background-color: #f6f8fa;
            padding: 16px;
            border-radius: 6px;
            overflow: auto;
            line-height: 1.45;
            margin-bottom: 16px;
        }}
        pre code {{
            background-color: transparent;
            padding: 0;
            font-size: 100%;
            word-break: normal;
            white-space: pre;
        }}
        table {{
            border-collapse: collapse;
            width: 100%;
            margin-bottom: 16px;
        }}
        th, td {{
            padding: 6px 13px;
            border: 1px solid #dfe2e5;
        }}
        tr:nth-child(2n) {{ background-color: #f6f8fa; }}
        th {{ font-weight: 600; background-color: #f2f2f2; }}
        
        blockquote {{
            padding: 0 1em;
            color: #6a737d;
            border-left: 0.25em solid #dfe2e5;
            margin: 0 0 16px 0;
        }}
        
        img {{ max-width: 100%; box-sizing: content-box; }}
        
        .important-alert {{
            background-color: #fffbdd;
            border: 1px solid #d1d5da;
            padding: 16px;
            margin-bottom: 16px;
            border-radius: 6px;
        }}
        
        /* PDF specific tweaks */
        @media print {{
            body {{ padding: 0; }}
            pre, blockquote, table {{ page-break-inside: avoid; }}
        }}
    </style>
    </head>
    <body>
        <div class="markdown-body">
            {html_content}
        </div>
    </body>
    </html>
    """

    # Use a temporary file for HTML
    with tempfile.NamedTemporaryFile(suffix='.html', delete=False, mode='w', encoding='utf-8') as tmp:
        tmp.write(full_html)
        tmp_path = tmp.name

    try:
        # Edge browser path
        edge_path = r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe"
        
        # Command to print to PDF
        # We use --no-sandbox to avoid permission issues in some environments
        # --print-to-pdf produces the PDF at the specified path
        cmd = [
            edge_path,
            "--headless",
            "--disable-gpu",
            "--no-sandbox",
            f"--print-to-pdf={pdf_file_path}",
            tmp_path
        ]
        
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode == 0:
            return None
        else:
            return f"Edge Error: {result.stderr}"
            
    except Exception as e:
        return str(e)
    finally:
        if os.path.exists(tmp_path):
            os.remove(tmp_path)

if __name__ == "__main__":
    docs_to_convert = [
        (r'E:\project\09_aquaPonics\docs\edgeAI\README.md', r'E:\project\09_aquaPonics\docs\edgeAI\README.pdf'),
        (r'E:\project\09_aquaPonics\docs\edgeAI\implementation_examples.md', r'E:\project\09_aquaPonics\docs\edgeAI\implementation_examples.pdf'),
        (r'E:\project\09_aquaPonics\docs\productFails\failed_companies_report.md', r'E:\project\09_aquaPonics\docs\productFails\failed_companies_report.pdf')
    ]

    for md, pdf in docs_to_convert:
        if not os.path.exists(md):
            print(f"File not found: {md}")
            continue
            
        print(f"Converting {md} to {pdf}...")
        error = convert_md_to_pdf(md, pdf)
        if error:
            print(f"Error converting {md}: {error}")
        else:
            print(f"Successfully converted {md}")
