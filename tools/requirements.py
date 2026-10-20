"""requirements.py - check what is needed to build and run the Recoil remake, with a download link for each missing item.

usage: python tools/requirements.py [--dgvoodoo DIR] [--contributor]
  --dgvoodoo DIR   also check a dgVoodoo folder (DDraw.dll + D3DImm.dll + dgVoodoo.conf) and its version
  --contributor    also check the Python packages used by the verification tools (prover, oracle checks)
Exit code 0 when everything required is present. See REQUIREMENTS.md for the same list in prose.
"""
import ctypes, os, shutil, subprocess, sys

PYTHON_MIN = (3, 10)
TESTED = {
    'python': '3.12.10',
    'vs': 'Visual Studio Build Tools 2022 17.14 (MSVC 14.44, CMake 3.31.6, Ninja 1.12.1 - CMake and Ninja come with it)',
    'dgvoodoo': '2.8.7.3',   # ProductVersion of dgVoodoo2's DDraw.dll / D3DImm.dll the remake was tested with (2.8.7.5 also played, 2026-10-08)
}
LINKS = {
    'python': 'https://www.python.org/downloads/windows/',
    'vs': 'https://visualstudio.microsoft.com/visual-cpp-build-tools/',
    'dgvoodoo': 'https://github.com/dege-diosg/dgVoodoo2/releases  (official site: http://dege.fw.hu/dgVoodoo2/)',
    'capstone': 'https://pypi.org/project/capstone/   (pip install capstone)',
    'pefile': 'https://pypi.org/project/pefile/   (pip install pefile)',
    'minidump': 'https://pypi.org/project/minidump/   (pip install minidump)',
}
results = []


def report(name, ok, detail, link_key=None, required=True):
    results.append((name, ok, required))
    mark = 'OK     ' if ok else ('MISSING' if required else 'missing (optional)')
    print('[%s] %s - %s' % (mark, name, detail))
    if not ok and link_key: print('          get it: %s' % LINKS[link_key])


def file_product_version(path):
    """ProductVersion string from a DLL's version resource (Win32 API through ctypes, no extra packages)"""
    v = ctypes.windll.version
    size = v.GetFileVersionInfoSizeW(path, None)
    if not size: return None
    buf = ctypes.create_string_buffer(size)
    if not v.GetFileVersionInfoW(path, 0, size, buf): return None
    p = ctypes.c_void_p(); n = ctypes.c_uint()
    if not v.VerQueryValueW(buf, '\\', ctypes.byref(p), ctypes.byref(n)): return None
    ffi = ctypes.cast(p, ctypes.POINTER(ctypes.c_uint32 * 13)).contents   # VS_FIXEDFILEINFO
    ms, ls = ffi[4], ffi[5]   # dwProductVersionMS / LS
    return '%d.%d.%d.%d' % (ms >> 16, ms & 0xffff, ls >> 16, ls & 0xffff)


def check_python():
    ok = sys.version_info[:2] >= PYTHON_MIN
    report('Python %d.%d+' % PYTHON_MIN, ok, 'running %s (tested %s)' % (sys.version.split()[0], TESTED['python']), 'python')


def find_vs():
    vswhere = os.path.join(os.environ.get('ProgramFiles(x86)', r'C:\Program Files (x86)'), 'Microsoft Visual Studio', 'Installer', 'vswhere.exe')
    if not os.path.exists(vswhere): return None
    try:
        out = subprocess.run([vswhere, '-latest', '-products', '*', '-requires', 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64',
                              '-property', 'installationPath'], capture_output=True, text=True).stdout.strip()
        return out or None
    except OSError:
        return None


def check_vs():
    vs = find_vs()
    report('Visual Studio 2022 C++ build tools', bool(vs), (vs or 'not found') +
           ' - install the "Desktop development with C++" workload (MSVC x86/x64, Windows SDK, C++ CMake tools); tested: ' + TESTED['vs'], 'vs')
    if not vs: return
    cm = os.path.join(vs, r'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe')
    nj = os.path.join(vs, r'Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe')
    report('CMake (with Visual Studio)', os.path.exists(cm) or bool(shutil.which('cmake')), cm if os.path.exists(cm) else 'add the "C++ CMake tools for Windows" component', 'vs')
    report('Ninja (with Visual Studio)', os.path.exists(nj) or bool(shutil.which('ninja')), nj if os.path.exists(nj) else 'add the "C++ CMake tools for Windows" component', 'vs')


def check_windows():
    sysdir = os.path.join(os.environ.get('SystemRoot', r'C:\Windows'), 'SysWOW64')
    report('MFC 4.2 runtime (mfc42.dll, ships with Windows)', os.path.exists(os.path.join(sysdir, 'mfc42.dll')), sysdir)
    report('DirectPlay (only for multiplayer; Windows Features > Legacy Components > DirectPlay)',
           os.path.exists(os.path.join(sysdir, 'dplayx.dll')), 'optional', required=False)


def check_dgvoodoo(d):
    files = ['DDraw.dll', 'D3DImm.dll', 'dgVoodoo.conf']
    have = [f for f in files if os.path.exists(os.path.join(d, f))]
    report('dgVoodoo2 files in %s' % d, len(have) == 3, 'found %s; needs DDraw.dll + D3DImm.dll from the MS\\x86 folder of the dgVoodoo2 zip, and dgVoodoo.conf'
           % (', '.join(have) or 'nothing'), 'dgvoodoo')
    if os.path.exists(os.path.join(d, 'DDraw.dll')):
        v = file_product_version(os.path.join(d, 'DDraw.dll'))
        same = v == TESTED['dgvoodoo']
        report('dgVoodoo2 version', True, '%s (tested with %s%s)' % (v, TESTED['dgvoodoo'], '' if same else ' - other versions are untested'))


def check_contributor():
    for mod in ('capstone', 'pefile', 'minidump'):
        try: __import__(mod); ok = True
        except ImportError: ok = False
        report('Python package %s (verification tools)' % mod, ok, 'used by tools/prove_identity.py, bytediff, snapshot tools', mod, required=False)


if __name__ == '__main__':
    print('Recoil remake - requirements\n')
    check_python(); check_vs(); check_windows()
    if '--dgvoodoo' in sys.argv: check_dgvoodoo(sys.argv[sys.argv.index('--dgvoodoo') + 1])
    else: print('[info   ] dgVoodoo2 (DirectDraw for modern Windows): get it from %s - tested version %s; pass --dgvoodoo DIR to check a folder'
               % (LINKS['dgvoodoo'], TESTED['dgvoodoo']))
    if '--contributor' in sys.argv: check_contributor()
    print('[info   ] your own copy of Recoil, 1999-01-29 build (e.g. the "Recoil Classic" CD): installed folder, zip, CD folder, .iso, .bin/.cue or .nrg '
          '(a full disc image also gives the CD music)')
    missing = [n for n, ok, req in results if req and not ok]
    print('\n' + ('everything required is present' if not missing else 'missing: ' + '; '.join(missing)))
    sys.exit(1 if missing else 0)
