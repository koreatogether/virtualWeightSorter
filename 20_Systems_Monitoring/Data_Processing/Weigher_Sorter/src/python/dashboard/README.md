# Weigher-Sorter Dashboard

This folder contains a Streamlit-based dashboard to visualize test results produced by the weigher-sorter simulator.

## Features
- Auto-discovery of latest CSV/JSON results under `../result/test_runner/` and `../result/performance_monitor/`
- Summary KPIs, rankings, and detailed tables
- Interactive filters (generator, algorithm)
- Charts: success rate ranking, avg time, memory peak, scatter plots

## Run
Use PowerShell in the project root:

```powershell
# First time: install deps (pandas, streamlit, plotly)
# If you use uv/venv, activate it first, then:
pip install pandas streamlit plotly

# Launch the dashboard
streamlit run src\python\dashboard\app.py
```

If your results are saved elsewhere, provide a path via query params:

```powershell
streamlit run src\python\dashboard\app.py -- --results-dir "e:\\project\\06_weigher-sorter\\src\\python\\result"
```

## Notes
- The app looks for files like `weigher_sorter_test_results_*.csv|json` and `test_results.csv`.
- You can drop additional result files into the result folder; the app will show them.
