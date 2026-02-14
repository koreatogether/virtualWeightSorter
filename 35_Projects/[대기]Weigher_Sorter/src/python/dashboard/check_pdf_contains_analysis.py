import sys
from pathlib import Path

pdf_path = Path(__file__).resolve().parents[1] / 'result' / 'pdf' / 'weigher_sorter_test_results_report.pdf'
print('Checking PDF:', pdf_path, 'exists=', pdf_path.exists())

try:
    from PyPDF2 import PdfReader
except Exception as e:
    print('PyPDF2 import failed:', e)
    sys.exit(2)

reader = PdfReader(str(pdf_path))
text = ''
for p in range(min(3, len(reader.pages))):
    page_text = reader.pages[p].extract_text() or ''
    text += page_text + '\n\n'

print('\n=== PAGE TEXT START ===\n')
print(text[:8000])
print('\n=== PAGE TEXT END ===\n')

# look for Korean keywords from the analysis
keywords = ['성공률', '요약', '권장', '추천', '분석', '결론']
found = {k: (k in text) for k in keywords}
print('Keywords found:', found)

# exit 0 if any keyword found, else exit 1
if any(found.values()):
    sys.exit(0)
else:
    sys.exit(1)
