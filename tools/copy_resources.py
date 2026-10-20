"""copy_resources.py - copy every resource of the original Recoil.exe (menu, 17 dialogs, string table, bitmap, icons, version) into a
remake executable, byte for byte (CONFIRMED-DATA: the file's own .rsrc entries, all languages).

The ported code loads its resources from its own module (MFC's resource handle = the exe that runs), e.g. MainWnd_Ctor's LoadMenuA
(found 2026-09-30 by the boot probe: without them LoadMenu returned NULL and the constructor faulted). Run as a post-build step
(CMakeLists.txt, target recoil_boot) or by hand.
usage: python tools/copy_resources.py TARGET.exe [SOURCE.exe]
"""
import ctypes, ctypes.wintypes as w, os, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
k = ctypes.WinDLL('kernel32', use_last_error=True)
for fn, res, args in (
        ('LoadLibraryExW', ctypes.c_void_p, [w.LPCWSTR, w.HANDLE, w.DWORD]),
        ('FindResourceExW', ctypes.c_void_p, [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, w.WORD]),
        ('LoadResource', ctypes.c_void_p, [ctypes.c_void_p, ctypes.c_void_p]),
        ('LockResource', ctypes.c_void_p, [ctypes.c_void_p]),
        ('SizeofResource', w.DWORD, [ctypes.c_void_p, ctypes.c_void_p]),
        ('BeginUpdateResourceW', ctypes.c_void_p, [w.LPCWSTR, w.BOOL]),
        ('UpdateResourceW', w.BOOL, [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, w.WORD, ctypes.c_void_p, w.DWORD]),
        ('EndUpdateResourceW', w.BOOL, [ctypes.c_void_p, w.BOOL]),
        ('EnumResourceTypesW', w.BOOL, [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p]),
        ('EnumResourceNamesW', w.BOOL, [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p]),
        ('EnumResourceLanguagesW', w.BOOL, [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p])):
    f = getattr(k, fn)
    f.restype, f.argtypes = res, args
TCB = ctypes.WINFUNCTYPE(w.BOOL, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p)
NCB = ctypes.WINFUNCTYPE(w.BOOL, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p)
LCB = ctypes.WINFUNCTYPE(w.BOOL, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, w.WORD, ctypes.c_void_p)


def key(p):
    """an integer id stays an integer; a named type / name becomes a Python string (the pointer is only valid during enumeration)"""
    return p if p < 0x10000 else ctypes.wstring_at(p)


def arg(v):
    return ctypes.c_void_p(v) if isinstance(v, int) else ctypes.c_wchar_p(v)


def main():
    target = os.path.abspath(sys.argv[1])
    source = sys.argv[2] if len(sys.argv) > 2 else os.path.join(ROOT, '00_original', 'game_install', 'Recoil.exe')
    try:
        n = copy_resources(target, source)
    except RuntimeError as e:
        sys.exit(str(e))
    print('copied %d resources from %s into %s' % (n, os.path.basename(source), os.path.basename(target)))


def copy_resources(target, source):
    """copy every resource of source into target (also used by the player Setup); returns the count, RuntimeError on failure"""
    h = k.LoadLibraryExW(source, None, 0x2 | 0x20)  # LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE
    if not h:
        raise RuntimeError('cannot open %s' % source)
    items = []
    types = []
    k.EnumResourceTypesW(h, TCB(lambda m, t, p: types.append(key(t)) or True), None)
    for t in types:
        names = []
        k.EnumResourceNamesW(h, arg(t), NCB(lambda m, tt, n, p: names.append(key(n)) or True), None)
        for n in names:
            langs = []
            k.EnumResourceLanguagesW(h, arg(t), arg(n), LCB(lambda m, tt, nn, l, p: langs.append(l) or True), None)
            for lang in langs:
                r = k.FindResourceExW(h, arg(t), arg(n), lang)
                size = k.SizeofResource(h, r)
                data = ctypes.string_at(k.LockResource(k.LoadResource(h, r)), size)
                items.append((t, n, lang, data))
    k.FreeLibrary.argtypes = [ctypes.c_void_p]
    k.FreeLibrary(h)
    u = k.BeginUpdateResourceW(target, False)
    if not u:
        raise RuntimeError('cannot update %s (error %d)' % (target, ctypes.get_last_error()))
    for t, n, lang, data in items:
        buf = ctypes.create_string_buffer(data, len(data))
        if not k.UpdateResourceW(u, arg(t), arg(n), lang, buf, len(data)):
            raise RuntimeError('UpdateResource failed for %r %r (error %d)' % (t, n, ctypes.get_last_error()))
    if not k.EndUpdateResourceW(u, False):
        raise RuntimeError('EndUpdateResource failed (error %d)' % ctypes.get_last_error())
    return len(items)


if __name__ == '__main__':
    main()
