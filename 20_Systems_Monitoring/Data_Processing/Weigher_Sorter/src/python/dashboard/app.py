import argparse
import glob
import json
import os
from dataclasses import dataclass
from typing import Dict, List, Optional, Tuple

import pandas as pd
import plotly.express as px
import streamlit as st

# ------------------------------------------------------------
# Utilities for locating and loading result files
# ------------------------------------------------------------

RESULT_BASE_DEFAULT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'result'))
TR_DIR = os.path.join(RESULT_BASE_DEFAULT, 'test_runner')
PM_DIR = os.path.join(RESULT_BASE_DEFAULT, 'performance_monitor')


@dataclass
class LoadedResults:
    summary: Optional[pd.DataFrame]
    details: Optional[pd.DataFrame]
    meta: Dict


def _find_latest_file(patterns: List[str]) -> Optional[str]:
    candidates: List[Tuple[float, str]] = []
    for pat in patterns:
        for path in glob.glob(pat):
            try:
                mtime = os.path.getmtime(path)
                candidates.append((mtime, path))
            except OSError:
                pass
    if not candidates:
        return None
    candidates.sort(reverse=True)
    return candidates[0][1]


def load_results(results_dir: Optional[str] = None) -> LoadedResults:
    base = results_dir or RESULT_BASE_DEFAULT
    tr_dir = os.path.join(base, 'test_runner')
    pm_dir = os.path.join(base, 'performance_monitor')

    # Look for test runner results (preferred: CSV, else JSON)
    csv_path = _find_latest_file([
        os.path.join(tr_dir, 'weigher_sorter_test_results_*.csv')
    ])
    json_path = _find_latest_file([
        os.path.join(tr_dir, 'weigher_sorter_test_results_*.json')
    ])

    df_summary: Optional[pd.DataFrame] = None
    df_details: Optional[pd.DataFrame] = None
    meta: Dict = {}

    if csv_path and os.path.exists(csv_path):
        try:
            df_summary = pd.read_csv(csv_path)
        except Exception:
            df_summary = None

    if json_path and os.path.exists(json_path):
        try:
            with open(json_path, 'r', encoding='utf-8') as f:
                data = json.load(f)
            meta = data.get('test_summary', {})
            comb = data.get('combination_results', {})
            # Normalize JSON to DataFrame similar to CSV
            rows = []
            for key, metrics in comb.items():
                gen, algo = key.split('_', 1)
                row = {'combination': key, 'generator': gen, 'algorithm': algo}
                row.update(metrics)
                rows.append(row)
            if rows:
                df_details = pd.DataFrame(rows)
        except Exception:
            df_details = None

    # Fallback to performance monitor CSV if no test_runner summary
    if df_summary is None:
        pm_csv = _find_latest_file([
            os.path.join(pm_dir, 'test_results*.csv'),
            os.path.join(pm_dir, 'test_results.csv'),
        ])
        if pm_csv and os.path.exists(pm_csv):
            try:
                df_summary = pd.read_csv(pm_csv)
            except Exception:
                df_summary = None

    return LoadedResults(summary=df_summary, details=df_details, meta=meta)


# ------------------------------------------------------------
# Streamlit UI
# ------------------------------------------------------------

def render_dashboard(results: LoadedResults):
    st.set_page_config(page_title='Weigher-Sorter Dashboard', layout='wide')
    st.title('Weigher-Sorter Test Results Dashboard')

    # Sidebar controls
    with st.sidebar:
        st.header('Data Source')
        base = st.session_state.get('results_dir', RESULT_BASE_DEFAULT)
        base = st.text_input('Results base directory', value=base)
        if st.button('Reload'):
            st.session_state['results_dir'] = base
            st.experimental_rerun()

        st.markdown('---')
        st.caption('Looking under:')
        st.code(os.path.join(base, 'test_runner'))
        st.code(os.path.join(base, 'performance_monitor'))

    # Load (re-load) data
    if 'results_dir' in st.session_state and st.session_state['results_dir'] != base:
        base = st.session_state['results_dir']
    loaded = load_results(base)

    if loaded.summary is None and loaded.details is None:
        st.warning('No results found. Run tests to generate CSV/JSON under result/ folders.')
        return

    # KPIs from meta or derive from data
    col1, col2, col3, col4 = st.columns(4)
    if loaded.meta:
        col1.metric('Total Tests', f"{loaded.meta.get('total_tests_run', 0):,}")
        col2.metric('Overall Success %', f"{loaded.meta.get('overall_success_rate', 0):.1f}%")
        col3.metric('Duration (s)', f"{loaded.meta.get('duration_seconds', 0):.1f}")
        col4.metric('Constraint Violations', f"{loaded.meta.get('constraint_violations', 0):,}")
    elif loaded.summary is not None and not loaded.summary.empty:
        total_tests = int(loaded.summary['total_tests'].sum()) if 'total_tests' in loaded.summary else 0
        avg_success = loaded.summary['success_rate'].mean() if 'success_rate' in loaded.summary else 0
        col1.metric('Total Tests', f"{total_tests:,}")
        col2.metric('Avg Success %', f"{avg_success:.1f}%")
        col3.metric('Files', 'CSV only')
        col4.metric('Violations', 'n/a')

    st.markdown('---')

    # Combined DataFrame for plots
    df = None
    if loaded.details is not None:
        df = loaded.details.copy()
    elif loaded.summary is not None:
        df = loaded.summary.copy()

    if df is None or df.empty:
        st.info('No data rows to display.')
        return

    # Ensure necessary columns exist
    for c in ['generator', 'algorithm']:
        if c not in df.columns and 'combination' in df.columns:
            # Split from combination
            parts = df['combination'].astype(str).str.split('_', n=1, expand=True)
            if parts.shape[1] == 2:
                df['generator'] = parts[0]
                df['algorithm'] = parts[1]

    # Filters
    gen_options = sorted(df['generator'].dropna().unique()) if 'generator' in df.columns else []
    algo_options = sorted(df['algorithm'].dropna().unique()) if 'algorithm' in df.columns else []
    sel_cols = st.columns(3)
    sel_gen = sel_cols[0].multiselect('Filter Generators', gen_options, default=gen_options)
    sel_algo = sel_cols[1].multiselect('Filter Algorithms', algo_options, default=algo_options)
    sel_cols[2].write(' ')

    if sel_gen and 'generator' in df.columns:
        df = df[df['generator'].isin(sel_gen)]
    if sel_algo and 'algorithm' in df.columns:
        df = df[df['algorithm'].isin(sel_algo)]

    # Tabs for different views
    tab1, tab2, tab3, tab4 = st.tabs([
        'Rankings', 'Timing', 'Memory', 'Table'
    ])

    with tab1:
        st.subheader('Success Rate Ranking')
        if 'success_rate' in df.columns:
            fig = px.bar(
                df.sort_values('success_rate', ascending=False),
                x='combination', y='success_rate', color='generator',
                text=df['success_rate'].map(lambda v: f"{v:.1f}%")
            )
            fig.update_layout(xaxis_tickangle=-45, height=450)
            st.plotly_chart(fig, use_container_width=True)
        else:
            st.info('success_rate column not found.')

    with tab2:
        st.subheader('Average Time (ms) by Combination')
        if 'avg_time_ms' in df.columns:
            fig = px.bar(
                df.sort_values('avg_time_ms'),
                x='combination', y='avg_time_ms', color='algorithm',
                text=df['avg_time_ms'].map(lambda v: f"{v:.1f}ms")
            )
            fig.update_layout(xaxis_tickangle=-45, height=450)
            st.plotly_chart(fig, use_container_width=True)
        else:
            st.info('avg_time_ms column not found.')

        st.subheader('Success vs Time')
        if 'success_rate' in df.columns and 'avg_time_ms' in df.columns:
            # Try to add trendline if statsmodels is available
            trendline = None
            try:
                import statsmodels  # noqa: F401
                trendline = 'ols'
            except Exception:
                trendline = None
            fig = px.scatter(
                df, x='avg_time_ms', y='success_rate', color='algorithm', symbol='generator',
                hover_name='combination', trendline=trendline
            )
            st.plotly_chart(fig, use_container_width=True)

    with tab3:
        st.subheader('Memory Peak (KB) by Combination')
        if 'memory_peak_kb' in df.columns:
            fig = px.bar(
                df.sort_values('memory_peak_kb'),
                x='combination', y='memory_peak_kb', color='generator',
                text=df['memory_peak_kb'].map(lambda v: f"{v:.1f}KB")
            )
            fig.update_layout(xaxis_tickangle=-45, height=450)
            st.plotly_chart(fig, use_container_width=True)
        else:
            st.info('memory_peak_kb column not found.')

    with tab4:
        st.subheader('Data Table')
        st.dataframe(df, use_container_width=True)


# ------------------------------------------------------------
# Entry point (for direct `streamlit run app.py`)
# ------------------------------------------------------------

if __name__ == '__main__':
    # Streamlit ignores argparse by default; we keep it for completeness when run as a script
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument('--results-dir', type=str, default=None, help='Base results directory')
    args, _ = parser.parse_known_args()

    st.session_state['results_dir'] = args.results_dir or RESULT_BASE_DEFAULT
    render_dashboard(load_results(args.results_dir))
