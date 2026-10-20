"""recoil_setup.py - the player Setup window (two steps). The work is done by setup_core.py.

Step 1  Requirements: dgVoodoo2 (link to the download page; the player selects the zip they downloaded).
Step 2  Your Recoil: the player selects their copy (installed folder, zip, CD folder, .iso, .bin/.cue, .nrg) and an install folder;
        Setup checks the version, then installs. Run: python tools/setup/recoil_setup.py (or Setup.exe in the players' download).
"""
import os, sys, threading, webbrowser
import tkinter as tk
from tkinter import filedialog, messagebox, ttk

import setup_core as core

TITLE = 'Recoil Redux Setup'


class Wizard(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title(TITLE)
        self.geometry('640x460')
        self.resizable(False, False)
        self.dgv = tk.StringVar()
        self.src = tk.StringVar()
        self.dest = tk.StringVar(value=os.path.join(os.environ.get('USERPROFILE', 'C:\\'), 'Games', 'Recoil Redux'))
        self.shortcut = tk.BooleanVar(value=True)
        self.source = None
        self.body = ttk.Frame(self, padding=16)
        self.body.pack(fill='both', expand=True)
        nav = ttk.Frame(self, padding=(16, 0, 16, 16))
        nav.pack(fill='x')
        self.back = ttk.Button(nav, text='< Back', command=self.step1)
        self.next = ttk.Button(nav, text='Next >', command=self.step2)
        self.next.pack(side='right')
        self.back.pack(side='right', padx=8)
        self.step1()

    def clear(self):
        for w in self.body.winfo_children():
            w.destroy()

    def heading(self, text, sub):
        ttk.Label(self.body, text=text, font=('Segoe UI', 13, 'bold')).pack(anchor='w')
        ttk.Label(self.body, text=sub, wraplength=600, justify='left').pack(anchor='w', pady=(4, 12))

    def picker(self, var, label, browse):
        row = ttk.Frame(self.body)
        row.pack(fill='x', pady=4)
        ttk.Label(row, text=label, width=16).pack(side='left')
        ttk.Entry(row, textvariable=var).pack(side='left', fill='x', expand=True)
        ttk.Button(row, text='Browse...', command=browse).pack(side='left', padx=(6, 0))

    # ---------------------------------------------------------------- step 1
    def step1(self):
        self.clear()
        self.heading('Step 1 of 2 - Requirements',
                     'Recoil uses DirectDraw, which modern Windows needs help with. The remake uses dgVoodoo2 for this '
                     '(free). Download the dgVoodoo2 zip, then select it here - Setup takes what it needs from it.')
        link = ttk.Label(self.body, text='Download dgVoodoo2: ' + core.DGV_LINK, foreground='#0645ad', cursor='hand2')
        link.pack(anchor='w')
        link.bind('<Button-1>', lambda e: webbrowser.open(core.DGV_LINK))
        ttk.Label(self.body, text='(tested with versions %s - the file is named like dgVoodoo2_87_5.zip)' % core.DGV_TESTED).pack(anchor='w', pady=(0, 12))
        self.picker(self.dgv, 'dgVoodoo2 zip:', lambda: self.dgv.set(
            filedialog.askopenfilename(title='Select the dgVoodoo2 zip', filetypes=[('zip', '*.zip')]) or self.dgv.get()))
        self.status1 = ttk.Label(self.body, text='', wraplength=600, justify='left')
        self.status1.pack(anchor='w', pady=12)
        self.back.state(['disabled'])
        self.next.configure(text='Next >', command=self.step2, state='normal')

    # ---------------------------------------------------------------- step 2
    def step2(self):
        try:
            core.dgvoodoo_members(self.dgv.get())
        except core.SetupError as e:
            self.status1.configure(text=str(e), foreground='#b00')
            return
        self.clear()
        self.heading('Step 2 of 2 - Your copy of Recoil',
                     'Select your own copy of Recoil: the installed game folder, a zip of it, the CD (or a folder copied from '
                     'it), or a disc image (.iso, .bin/.cue, .nrg). It must be the 1999-01-29 build, as on the "Recoil Classic" '
                     'CD. A full disc image (.nrg or .bin+.cue) also gives the CD music.')
        self.picker(self.src, 'Recoil copy:', self.pick_source)
        ttk.Button(self.body, text='Select a folder instead...', command=lambda: self.set_source(
            filedialog.askdirectory(title='Select the Recoil folder or CD'))).pack(anchor='e')
        self.info = ttk.Label(self.body, text='', wraplength=600, justify='left')
        self.info.pack(anchor='w', pady=8)
        self.picker(self.dest, 'Install to:', lambda: self.dest.set(filedialog.askdirectory(title='Install folder') or self.dest.get()))
        ttk.Checkbutton(self.body, text='Create a desktop shortcut', variable=self.shortcut).pack(anchor='w', pady=4)
        self.bar = ttk.Progressbar(self.body, length=600)
        self.bar.pack(fill='x', pady=(8, 4))
        self.log = tk.Text(self.body, height=6, width=80, state='disabled', font=('Consolas', 9))
        self.log.pack(fill='both', expand=True)
        self.back.state(['!disabled'])
        self.next.configure(text='Install', command=self.start, state='disabled')

    def pick_source(self):
        self.set_source(filedialog.askopenfilename(title='Select your Recoil copy', filetypes=[
            ('Recoil copy', '*.zip *.iso *.bin *.cue *.nrg *.exe'), ('all files', '*.*')]))

    def set_source(self, path):
        if not path:
            return
        if path.lower().endswith('recoil.exe'):
            path = os.path.dirname(path)
        self.src.set(path)
        self.info.configure(text='checking...', foreground='')
        self.update_idletasks()
        try:
            self.source, lines = core.check_source(path)
            self.info.configure(text='\n'.join(lines), foreground='#060')
            self.next.configure(state='normal')
        except core.SetupError as e:
            self.source = None
            self.info.configure(text=str(e), foreground='#b00')
            self.next.configure(state='disabled')

    # ---------------------------------------------------------------- install
    def write(self, msg):
        self.after(0, self._write, msg)

    def _write(self, msg):
        self.log.configure(state='normal')
        self.log.insert('end', msg + '\n')
        self.log.see('end')
        self.log.configure(state='disabled')

    def start(self):
        dest = self.dest.get()
        if os.path.isdir(dest) and os.listdir(dest) and not messagebox.askyesno(TITLE, '%s is not empty. Install into it anyway?' % dest):
            return
        self.next.state(['disabled'])
        self.back.state(['disabled'])
        threading.Thread(target=self.run, args=(dest,), daemon=True).start()

    def run(self, dest):
        try:
            core.install(self.src.get(), self.dgv.get(), dest, shortcut=self.shortcut.get(), log=self.write,
                         progress=lambda i, n: self.after(0, lambda: self.bar.configure(maximum=n, value=i)))
            self.after(0, lambda: (messagebox.showinfo(TITLE, 'Recoil Redux is installed.'), self.next.configure(
                text='Close', command=self.destroy, state='normal')))
        except Exception as e:   # SetupError or an I/O failure: report it, allow another try
            self.write('FAILED: %s' % e)
            self.after(0, lambda: (self.next.state(['!disabled']), self.back.state(['!disabled'])))


if __name__ == '__main__':
    Wizard().mainloop()
