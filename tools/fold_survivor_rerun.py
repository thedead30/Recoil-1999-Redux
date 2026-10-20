"""fold_survivor_rerun.py - fold the result of a survivor re-run into 03_re/staging/verify/mutation_results.csv.

A SURVIVED mutant that a newly linked test kills (03_re/staging/verify/wt_dict_survivor_rerun*.csv, written by a re-run that planted the mutant and ran only the new test)
becomes KILLED in the results file, so promote_ready sees the function as fully mutation-checked. Rows the re-run left SURVIVED / NO-BUILD stay as they are.
usage: python tools/fold_survivor_rerun.py RERUN.csv [--dry]     (run when llm_pipeline mutate is not running: it appends to the same file)
"""
import csv, io, os, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
MRES = os.path.join(ROOT, '03_re', 'staging', 'verify', 'mutation_results.csv')


def main():
    src = sys.argv[1]
    dry = '--dry' in sys.argv
    killed = {(r['name'], r['regex'], r['replacement']) for r in csv.DictReader(io.open(src, encoding='utf-8')) if r['new'] == 'KILLED'}
    rows = list(csv.reader(io.open(MRES, encoding='utf-8', newline='')))
    h = rows[0]
    ik = [h.index('name'), h.index('regex'), h.index('replacement')]
    io_ = h.index('outcome')
    n = 0
    for r in rows[1:]:
        if r and r[io_] == 'SURVIVED' and tuple(r[i] for i in ik) in killed:
            r[io_] = 'KILLED'
            n += 1
    if n and not dry:
        csv.writer(io.open(MRES, 'w', encoding='utf-8', newline=''), lineterminator='\n').writerows(rows)
    print('%d of %d re-run kills folded into mutation_results.csv%s' % (n, len(killed), ' (dry run)' if dry else ''))


if __name__ == '__main__':
    main()
