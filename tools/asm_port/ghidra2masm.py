"""ghidra2masm.py - convert a Ghidra listing (as printed by our listing dumps: 'addr MNEMONIC operands')
into MSVC inline-assembly text for an instruction-level port (naked function). Branch targets become
labels L_<addr>. Absolute data addresses are reported so they can be replaced by tagged constants;
the converter refuses to emit them silently.

usage: python tools/asm_port/ghidra2masm.py LISTING.txt START_ADDR [--map 0xADDR=Name,...] [--out FILE]
  --map   absolute data addresses and call targets -> C++ names (tagged constants / ported functions);
          any absolute reference NOT mapped is an error.
"""
import io
import os
import re
import sys

IMAGE_LO, IMAGE_HI = 0x00400000, 0x007C9000  # Recoil.exe image (ImageBase, ImageBase + SizeOfImage)


def conv_operand(op):
    op = op.strip()
    op = re.sub(r"\bfloat ptr\b", "dword ptr", op)
    op = re.sub(r"\bdouble ptr\b", "qword ptr", op)
    op = re.sub(r"\bST(\d)\b", r"st(\1)", op)
    op = re.sub(r"\+ -0x", "- 0x", op)
    op = re.sub(r"\bE([A-Z]{2})\b", lambda m: "e" + m.group(1).lower(), op)
    return op


EXE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "00_original", "game_install", "Recoil.exe")  # any cwd
_exe = None


def exe_bytes(va, n):
    """n bytes of the original image at va, from the file (initialised data only)."""
    global _exe
    if _exe is None:
        import struct
        if not os.path.exists(EXE):  # cloud sessions (03_re/CLOUD.md): no .text bytes, so no switch tables
            raise SystemExit("0x%08x: needs Recoil.exe (switch table bytes)" % va)
        b = open(EXE, "rb").read()
        pe = struct.unpack_from("<I", b, 0x3C)[0]
        nsec = struct.unpack_from("<H", b, pe + 6)[0]
        opt = struct.unpack_from("<H", b, pe + 20)[0]
        _exe = (b, [struct.unpack_from("<IIII", b, pe + 24 + opt + i * 40 + 8) for i in range(nsec)])
    b, secs = _exe
    rva = va - IMAGE_LO
    for vs, v, rs, ro in secs:
        if v <= rva and rva + n <= v + rs:
            return b[rva - v + ro: rva - v + ro + n]
    raise SystemExit("0x%08x..+%d is not file-backed" % (va, n))


def find_switches(body):
    """Compiler switch dispatches: CMP idx,N / JA default / [XOR r,r / MOV r8,byte ptr [idx + BT]] /
    JMP dword ptr [r*0x4 + JT], tables inside the original .text. Returns {line of the JMP: info}.
    The tables are read from the executable; N comes from the bounds check, so the case set is exact."""
    import struct
    sw = {}
    for i, (addr, ins) in enumerate(body):
        m = re.fullmatch(r"JMP dword ptr \[(E\w\w)\*0x4 \+ 0x([0-9a-f]{6,8})\]", ins)
        if not m:
            continue
        jreg, jt = m.group(1), int(m.group(2), 16)
        byte_line, bt, breg, idx, bound = None, None, None, None, None
        ja = None
        for k in range(i - 1, max(i - 8, -1), -1):
            ins_k = body[k][1]
            mb = re.fullmatch(r"MOV (\w\w),byte ptr \[(E\w\w) \+ 0x([0-9a-f]{6,8})\]", ins_k)
            if mb:
                byte_line, breg, idx, bt = k, mb.group(1), mb.group(2), int(mb.group(3), 16)
                continue
            if ja is None and ins_k.startswith("JA "):
                ja = k
                continue
            mc = re.fullmatch(r"CMP (E\w\w),0x([0-9a-f]+)", ins_k)
            if mc and ja is not None:
                # between the CMP and the JA only flag-neutral instructions may stand (MOV / LEA), and none of
                # them may write the index register
                between = [body[q][1] for q in range(k + 1, ja)]
                # flag-neutral: MOV / LEA / PUSH, and x87 loads and stores (FLD / FST / FSTP / FILD / FXCH; the
                # compare forms set EFLAGS or the FPU flags and are not allowed); none may write the index register
                idxw = ("MOV %s," % mc.group(1), "LEA %s," % mc.group(1), "POP %s" % mc.group(1))
                if all(re.match(r"(MOV|LEA|PUSH|FLD|FILD|FST|FSTP|FISTP|FXCH|FSUB|FMUL|FADD|FDIV)\b", x) and not x.startswith(idxw)
                       for x in between):
                    if idx is None:
                        idx = mc.group(1)
                    if mc.group(1) != idx:
                        raise SystemExit("switch at %s: bounds check on %s, index %s" % (addr, mc.group(1), idx))
                    bound = int(mc.group(2), 16)
                    break
        if bound is None:
            raise SystemExit("switch at %s: no CMP/JA bounds check found" % addr)
        if bt is None:
            if idx != jreg:
                raise SystemExit("switch at %s: index %s but jump register %s" % (addr, idx, jreg))
            slots = list(range(bound + 1))
            sel = None
        else:
            sel = list(exe_bytes(bt, bound + 1))
            slots = sel
        jtab = [struct.unpack_from("<I", exe_bytes(jt + 4 * s, 4))[0] for s in range(max(slots) + 1)]
        sw[i] = {"idx": idx, "bound": bound, "sel": sel, "breg": breg, "jt": jtab, "byte_line": byte_line,
                 "targets": sorted(set(jtab[s] for s in slots))}
    return sw


def emit_switch(sw, addr):
    """Compare chain equivalent to the table dispatch (registers identical; flags are the chain's)."""
    idx = sw["idx"].lower()
    lines = []
    for k in range(sw["bound"] + 1):
        if sw["sel"] is None:
            lines += ["        cmp %s, %d" % (idx, k), "        je L_%x" % sw["jt"][k]]
        else:
            skip = "Lsw_%s_%d" % (addr.lstrip("0"), k)
            lines += ["        cmp %s, %d" % (idx, k), "        jne %s" % skip,
                      "        mov %s, %d" % (sw["breg"], sw["sel"][k]), "        jmp L_%x" % sw["jt"][sw["sel"][k]],
                      "    %s:" % skip]
    lines.append("        int 3  // unreachable: the bounds check above excludes other indices")
    return lines


def main():
    path, start = sys.argv[1], sys.argv[2].lower().replace("0x", "").lstrip("0")
    mapping = {}
    if "--map" in sys.argv:
        spec = sys.argv[sys.argv.index("--map") + 1]
        if spec.startswith("@"):  # map in a file: too many entries for the Windows command line (32 KB)
            spec = open(spec[1:], encoding="utf-8").read().strip()
        for kv in spec.split(","):
            k, v = kv.split("=")
            mapping[int(k, 16)] = v
    outpath = sys.argv[sys.argv.index("--out") + 1] if "--out" in sys.argv else None
    # integers that only look like addresses (value inside the image range): 03_re/ledger/imm_constants.csv, one row per
    # function and value with the evidence; everything else in the image range stays an address (an unmapped one refuses the function)
    int_consts = set()
    cpath = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "03_re", "ledger", "imm_constants.csv")
    if os.path.exists(cpath):
        import csv
        for row in csv.DictReader(open(cpath, encoding="utf-8")):
            if int(row["function"], 16) == int(start, 16):
                int_consts.add(int(row["value"], 16))
    lines = open(path, encoding="utf-8").read().splitlines()
    body, on = [], False
    for ln in lines:
        if ln.startswith("###"):
            if on:
                break
            # the header's ADDRESS field must equal START: a substring test also matched an earlier chunk whose NAME embeds the
            # address (### 00414660 thunk_FUN_00414590 for START 414590), porting the thunk's body under the target's name (found
            # 2026-09-30 by the boot probe: Hud_BuildNameMessage_00414590 became `jmp Hud_BuildNameMessage_00414590`)
            f = ln.split()
            on = len(f) > 1 and f[1].lower().lstrip("0") == start
            continue
        if on and ln.strip():
            body.append(ln.split(" ", 1))
    targets = set()
    for addr, ins in body:
        m = re.match(r"J\w+ 0x([0-9a-f]+)", ins)
        if m:
            targets.add(m.group(1).lstrip("0"))
    body_addrs = {int(a, 16) for a, _ in body}
    switches = find_switches(body)
    for sw in switches.values():
        targets.update("%x" % t for t in sw["targets"])
    # a code address INSIDE this function used as a value (a C++ catch block's MOV EAX,<continuation>; RET) becomes a label too
    for addr, ins in body:
        if re.match(r"(CALL|J\w+) ", ins):
            continue
        for mm in re.finditer(r"0x([0-9a-f]{6,8})", ins.split(" ", 1)[1] if " " in ins else ""):
            v = int(mm.group(1), 16)
            if v in body_addrs and v not in mapping and v not in int_consts:
                targets.add("%x" % v)
    # --publish ADDR=DEST;...: store the address of the label at ADDR into DEST before the first instruction (tools/gen_eh_frames.py:
    # the catch blocks of a C++ try are labels inside this naked port, and the generated HandlerType table must hold their addresses)
    publish = []
    if "--publish" in sys.argv:
        for kv in sys.argv[sys.argv.index("--publish") + 1].split(";"):
            if kv.strip():
                k, v = kv.split("=", 1)
                publish.append((int(k, 16), v))
                if int(k, 16) not in body_addrs:
                    sys.exit("--publish: 0x%x is not an instruction of this function" % int(k, 16))
                targets.add("%x" % int(k, 16))
    skip = {sw["byte_line"] for sw in switches.values() if sw["byte_line"] is not None}
    out, absolute = [], []
    for k, dest in publish:
        out.append("        mov dword ptr [%s], offset L_%x" % (dest, k))
    for n_line, (addr, ins) in enumerate(body):
        if n_line in skip:  # the byte-table load is folded into the switch's compare chain below
            a = addr.lstrip("0")
            for t in targets:
                if t == ("%x" % int(addr, 16)):
                    out.append("    L_%s:" % t)
            continue
        if n_line in switches:
            for t in targets:
                if t == ("%x" % int(addr, 16)):
                    out.append("    L_%s:" % t)
            out.extend(emit_switch(switches[n_line], addr))
            continue
        a = addr.lstrip("0")
        full = ("%x" % int(addr, 16)).lstrip("0")
        for t in targets:
            if t.endswith(full) or full.endswith(t):
                out.append("    L_%s:" % t)
        parts = ins.split(" ", 1)
        mn = parts[0].lower()
        ops = parts[1] if len(parts) > 1 else ""
        # String instructions: Ghidra writes the prefix as a suffix (MOVSD.REP) and spells out the implicit
        # operands (ES:EDI,ESI). Emit the prefix and drop the operands.
        sm = re.fullmatch(r"(movs|stos|lods|scas|cmps)([bwd])(\.rep|\.repe|\.repne)?", mn)
        if sm:
            prefix = {None: "", ".rep": "rep ", ".repe": "repe ", ".repne": "repne "}[sm.group(3)]
            out.append("        %s%s%s" % (prefix, sm.group(1), sm.group(2)))
            continue
        if "." in mn:
            sys.exit("unhandled mnemonic suffix: %s %s" % (addr, ins))
        m = re.match(r"0x([0-9a-f]+)", ops)
        if mn.startswith("j") and m:
            v = int(m.group(1), 16)
            if v not in body_addrs:  # a tail jump into another function: its port, like a CALL target
                if v in mapping:
                    out.append("        %s %s" % (mn, mapping[v]))
                else:
                    absolute.append((addr, ins))
                continue
            out.append("        %s L_%s" % (mn, m.group(1).lstrip("0")))
            continue
        def sub_abs(mm):
            v = int(mm.group(1), 16)
            if v in mapping:
                return "[" + mapping[v] + "]"
            absolute.append((addr, ins))
            return mm.group(0)
        ops = re.sub(r"\[0x([0-9a-f]{6,8})\]", sub_abs, ops)
        # Any other value inside the original image is an address too: an immediate (PUSH 0x53d758 ->
        # `offset Name`) or an indexed displacement ([EAX*0x4 + 0x4d2000] -> [EAX*0x4 + Name]). Mapped ones are
        # substituted; unmapped ones are an error like any other absolute reference.
        if not (mn == "call" or mn.startswith("j")):
            def sub_imm(mm):
                v = int(mm.group(1), 16)
                if not IMAGE_LO <= v < IMAGE_HI or v in int_consts:
                    return mm.group(0)
                if v in body_addrs and v not in mapping and ops[:mm.start()].count("[") == ops[:mm.start()].count("]"):
                    return "offset L_%x" % v  # own code address as a value (labelled above)
                if v not in mapping:
                    absolute.append((addr, ins))
                    return mm.group(0)
                inside = ops[:mm.start()].count("[") > ops[:mm.start()].count("]")
                if mapping[v].startswith("dword ptr ["):
                    # the immediate is the address of an import thunk (JMP [slot]): pushing it as a function pointer is
                    # pushing the slot's value; only PUSH has that form (push dword ptr [slot]), anything else is refused
                    if mn == "push" and not inside and ops.strip() == mm.group(0):
                        return mapping[v]
                    absolute.append((addr, ins))
                    return mm.group(0)
                return mapping[v] if inside else "offset " + mapping[v]
            ops = re.sub(r"\b0x([0-9a-f]{6,8})\b", sub_imm, ops)
        mc = re.match(r"0x([0-9a-f]{6,8})$", ops.strip())
        if mn == "call" and mc:
            v = int(mc.group(1), 16)
            if v in mapping:
                out.append("        call %s" % mapping[v])
                continue
            absolute.append((addr, ins))
        # x87 register forms written without operands by Ghidra
        if mn in ("faddp", "fmulp", "fsubp", "fsubrp", "fdivp", "fdivrp") and not ops:
            ops = "ST1,ST0"
        if mn in ("faddp", "fmulp", "fsubp", "fsubrp", "fdivp", "fdivrp") and re.fullmatch(r"ST\d", ops.strip()):
            ops = ops.strip() + ",ST0"
        if mn == "fxch" and not ops:
            ops = "ST1"
        if mn in ("fmul", "fadd", "fsub", "fsubr", "fdiv", "fdivr") and re.fullmatch(r"ST\d", ops.strip()):
            ops = "ST0," + ops.strip()
        conv = ",".join(conv_operand(o) for o in ops.split(",")) if ops else ""
        # Ghidra leaves a memory operand's size implicit when a register operand gives it ("MOV [0x4e1398],EAX").
        # Once the address becomes a symbol (e.g. a byte-typed data image), MASM would take the symbol's type, so
        # state the size explicitly from the register's width.
        parts2 = [p.strip() for p in conv.split(",")] if conv else []
        if len(parts2) == 2 and mn not in ("lea",):
            widths = {"dword": re.compile(r"^e[a-ds][xip]$|^e[sb]p$|^e[sd]i$", re.I), "word": re.compile(r"^[a-d]x$|^[sd]i$|^[sb]p$", re.I),
                      "byte": re.compile(r"^[a-d][lh]$", re.I)}
            for mi, ri in ((0, 1), (1, 0)):
                if parts2[mi].startswith("[") and "ptr" not in parts2[mi]:
                    for size, rx in widths.items():
                        if rx.match(parts2[ri]):
                            parts2[mi] = "%s ptr %s" % (size, parts2[mi])
                            break
            conv = ",".join(parts2)
        out.append("        %s%s" % (mn, (" " + conv.replace(",", ", ")) if conv else ""))
    if absolute:
        sys.stderr.write("UNMAPPED ABSOLUTE REFERENCES (map them with --map):\n")
        for a, i in absolute:
            sys.stderr.write("  %s %s\n" % (a, i))
        sys.exit(1)
    text = "\n".join(out) + "\n"
    if outpath:
        io.open(outpath, "w", encoding="utf-8", newline="").write(text)
    else:
        sys.stdout.write(text)


if __name__ == "__main__":
    main()
