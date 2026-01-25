#!/usr/bin/env python3
import os
import tkinter as tk
from tkinter import ttk, messagebox, simpledialog, filedialog, colorchooser
from pathlib import Path

from .core import ThemeEditor, get_theme_dir, load_enums


class ThemeEditorGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Izotrox Theme Editor")
        self.root.geometry("800x600")
        
        self.editor = None
        self.current_file = None
        
        self.create_menu()
        self.create_widgets()
        self.create_statusbar()
        
        self.modified = False
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        
        self.theme_dir = get_theme_dir()
        self.status_var.set("Ready")

    def on_closing(self):
        if self.modified:
            if not messagebox.askyesno("Unsaved Changes", "You have unsaved changes. Exit anyway?"):
                return
        self.root.destroy()

    def create_menu(self):
        menubar = tk.Menu(self.root)
        
        file_menu = tk.Menu(menubar, tearoff=0)
        file_menu.add_command(label="Open...", command=self.open_theme)
        file_menu.add_command(label="Save", command=self.save_theme)
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self.on_closing)
        menubar.add_cascade(label="File", menu=file_menu)
        
        edit_menu = tk.Menu(menubar, tearoff=0)
        edit_menu.add_command(label="Add Section", command=self.add_section)
        edit_menu.add_command(label="Remove Section", command=self.remove_section)
        edit_menu.add_separator()
        edit_menu.add_command(label="Add Key", command=self.add_key)
        edit_menu.add_command(label="Remove Key", command=self.remove_key)
        menubar.add_cascade(label="Edit", menu=edit_menu)
        
        help_menu = tk.Menu(menubar, tearoff=0)
        help_menu.add_command(label="About", command=self.show_about)
        menubar.add_cascade(label="Help", menu=help_menu)
        
        self.root.config(menu=menubar)

    def create_widgets(self):
        self.tree_frame = ttk.Frame(self.root)
        self.tree_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        self.tree = ttk.Treeview(self.tree_frame, columns=("value"), show="tree headings")
        self.tree.heading("#0", text="Name")
        self.tree.heading("value", text="Value")
        self.tree.column("#0", width=250, minwidth=100)
        self.tree.column("value", width=400, minwidth=200)
        
        self.scrollbar = ttk.Scrollbar(self.tree_frame, orient=tk.VERTICAL, command=self.tree.yview)
        self.tree.configure(yscroll=self.scrollbar.set)
        
        self.tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self.scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        self.tree.bind("<Double-1>", self.on_double_click)

    def create_statusbar(self):
        self.status_var = tk.StringVar()
        self.statusbar = ttk.Label(self.root, textvariable=self.status_var, relief=tk.SUNKEN, anchor=tk.W)
        self.statusbar.pack(side=tk.BOTTOM, fill=tk.X)

    def open_theme(self):
        initial_dir = self.theme_dir if self.theme_dir.exists() else os.getcwd()
        filename = filedialog.askopenfilename(
            initialdir=initial_dir,
            title="Select Theme File",
            filetypes=(("INI files", "*.ini"), ("All files", "*.*"))
        )
        if filename:
            self.load_file(filename)

    def load_file(self, filename):
        try:
            self.editor = ThemeEditor(filename)
            self.current_file = filename
            self.modified = False
            self.refresh_tree()
            self.status_var.set(f"Loaded: {filename}")
            self.root.title(f"Izotrox Theme Editor - {os.path.basename(filename)}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to load file: {e}")

    def refresh_tree(self):
        for item in self.tree.get_children():
            self.tree.delete(item)
        if not self.editor:
            return
        sorted_sections = sorted(self.editor.sections.keys())
        for section in sorted_sections:
            section_id = self.tree.insert("", "end", text=section, open=True, tags=("section",))
            lines = self.editor.sections[section]
            for line in lines:
                if '=' in line:
                    parts = line.split('=', 1)
                    key = parts[0].strip()
                    val = parts[1].strip()
                    self.tree.insert(section_id, "end", text=key, values=(val,), tags=("key",))

    def set_modified(self, val=True):
        self.modified = val
        if val:
            self.status_var.set(f"Changes pending... {os.path.basename(self.current_file) if self.current_file else ''}")
        else:
            self.status_var.set(f"Ready: {os.path.basename(self.current_file) if self.current_file else ''}")

    def on_double_click(self, event):
        selected = self.tree.selection()
        if not selected: return
        item_id = selected[0]
        item_tags = self.tree.item(item_id, "tags")
        if "key" in item_tags:
            self.edit_value(item_id)

    def select_enum(self, parent):
        enum_dialog = tk.Toplevel(parent)
        enum_dialog.title("Choose From Enum")
        enum_dialog.geometry("400x450")
        enum_dialog.resizable(False, False)
        enum_dialog.transient(parent)
        enum_dialog.wait_visibility()
        enum_dialog.grab_set()

        enums = load_enums()

        tree_frame = ttk.Frame(enum_dialog)
        tree_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        tree = ttk.Treeview(tree_frame, show="tree")
        tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        scrollbar = ttk.Scrollbar(tree_frame, orient=tk.VERTICAL, command=tree.yview)
        tree.configure(yscroll=scrollbar.set)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)

        for enum_type, values in enums.items():
            parent_node = tree.insert("", "end", text=enum_type, open=True)
            for v in values:
                tree.insert(parent_node, "end", text=v)

        result = [None]

        def on_ok():
            selected = tree.selection()
            if selected:
                item = tree.item(selected[0])
                if tree.parent(selected[0]): 
                    result[0] = item["text"]
                    enum_dialog.destroy()

        def on_cancel():
            enum_dialog.destroy()

        btn_frame = tk.Frame(enum_dialog)
        btn_frame.pack(pady=10)
        tk.Button(btn_frame, text="OK", command=on_ok, width=10).pack(side=tk.LEFT, padx=5)
        tk.Button(btn_frame, text="Cancel", command=on_cancel, width=10).pack(side=tk.LEFT, padx=5)

        enum_dialog.wait_window()
        return result[0]

    def ask_value(self, title, initial_value):
        dialog = tk.Toplevel(self.root)
        dialog.title(title)
        dialog.geometry("400x200")
        dialog.resizable(False, False)
        dialog.transient(self.root)
        dialog.wait_visibility()
        dialog.grab_set()

        tk.Label(dialog, text="Value:").pack(pady=(10, 0), padx=10, anchor=tk.W)
        entry = tk.Entry(dialog)
        entry.insert(0, initial_value)
        entry.pack(fill=tk.X, padx=10, pady=5)

        btn_row1 = tk.Frame(dialog)
        btn_row1.pack(fill=tk.X, padx=10, pady=2)

        def choose_file():
            f = filedialog.askopenfilename(parent=dialog)
            if f:
                try:
                    rel = os.path.relpath(f, os.getcwd())
                    entry.delete(0, tk.END)
                    entry.insert(0, rel)
                except:
                    entry.delete(0, tk.END)
                    entry.insert(0, f)

        def paste_val():
            try:
                val = self.root.clipboard_get()
                entry.delete(0, tk.END)
                entry.insert(0, val)
            except:
                pass

        tk.Button(btn_row1, text="Choose File", command=choose_file).pack(side=tk.LEFT, padx=(0, 5))
        tk.Button(btn_row1, text="Paste...", command=paste_val).pack(side=tk.LEFT)

        def choose_enum():
            val = self.select_enum(dialog)
            if val:
                entry.delete(0, tk.END)
                entry.insert(0, val)

        tk.Button(dialog, text="Choose From Enum", command=choose_enum).pack(padx=10, pady=5, anchor=tk.W)

        result = [None]

        def on_save():
            result[0] = entry.get()
            dialog.destroy()

        def on_cancel():
            dialog.destroy()

        bottom_btns = tk.Frame(dialog)
        bottom_btns.pack(side=tk.BOTTOM, pady=10)
        tk.Button(bottom_btns, text="Save", command=on_save, width=10).pack(side=tk.LEFT, padx=5)
        tk.Button(bottom_btns, text="Cancel", command=on_cancel, width=10).pack(side=tk.LEFT, padx=5)

        dialog.wait_window()
        return result[0]

    def edit_value(self, item_id):
        if not self.editor: return
        key = self.tree.item(item_id, "text")
        parent_id = self.tree.parent(item_id)
        section = self.tree.item(parent_id, "text")
        current_val = self.tree.item(item_id, "values")[0]
        
        new_val = None
        if section in ["Colors", "ColorVariants"]:
            dialog = tk.Toplevel(self.root)
            dialog.title("Edit Color")
            dialog.geometry("300x150")
            dialog.resizable(False, False)
            dialog.transient(self.root)
            dialog.update_idletasks()
            tk.Label(dialog, text="Choose an option").pack(pady=10)
            choice_var = tk.StringVar(value="none")
            btn_frame = tk.Frame(dialog)
            btn_frame.pack(pady=5, fill=tk.X, padx=20)
            def use_picker():
                choice_var.set("picker")
                dialog.destroy()
            def use_text():
                choice_var.set("text")
                dialog.destroy()
            tk.Button(btn_frame, text="Color Picker", command=use_picker).pack(fill=tk.X, pady=2)
            tk.Button(btn_frame, text="Value Edit", command=use_text).pack(fill=tk.X, pady=2)
            dialog.wait_visibility()
            dialog.grab_set()
            self.root.wait_window(dialog)
            choice = choice_var.get()
            if choice == "picker":
                initial = None
                try:
                    parts = [int(x.strip()) for x in current_val.split(',')]
                    if len(parts) >= 3:
                        initial = f"#{parts[0]:02x}{parts[1]:02x}{parts[2]:02x}"
                except:
                    pass
                color = colorchooser.askcolor(initialcolor=initial, title=f"Choose color for {key}")
                if color[0]:
                    r, g, b = int(color[0][0]), int(color[0][1]), int(color[0][2])
                    alpha = ""
                    try:
                        parts = [x.strip() for x in current_val.split(',')]
                        if len(parts) == 4:
                            alpha = f", {parts[3]}"
                    except:
                        pass
                    new_val = f"{r}, {g}, {b}{alpha}"
            elif choice == "text":
                new_val = self.ask_value(f"Edit {key}", current_val)
        else:
            new_val = self.ask_value(f"Edit {key}", current_val)
        
        if new_val is not None:
            self.editor.add_key(section, key, new_val)
            self.set_modified(True)
            self.refresh_tree()

    def add_section(self):
        if not self.editor: return
        name = simpledialog.askstring("Add Section", "Enter section name:", parent=self.root)
        if name:
            if name in self.editor.sections:
                messagebox.showerror("Error", "Section already exists")
                return
            self.editor.add_section(name)
            self.set_modified(True)
            self.refresh_tree()

    def remove_section(self):
        if not self.editor: return
        selected = self.tree.selection()
        if not selected: return
        item_id = selected[0]
        name = self.tree.item(item_id, "text")
        tags = self.tree.item(item_id, "tags")
        if "section" in tags:
            if messagebox.askyesno("Confirm", f"Remove section '{name}' and all its keys?"):
                self.editor.remove_section(name)
                self.set_modified(True)
                self.refresh_tree()
        else:
            messagebox.showinfo("Info", "Please select a section to remove")

    def add_key(self):
        if not self.editor: return
        selected = self.tree.selection()
        section_name = None
        if selected:
            item_id = selected[0]
            tags = self.tree.item(item_id, "tags")
            if "section" in tags:
                section_name = self.tree.item(item_id, "text")
            elif "key" in tags:
                parent_id = self.tree.parent(item_id)
                section_name = self.tree.item(parent_id, "text")
        if not section_name:
            sections = sorted(self.editor.sections.keys())
            if not sections:
                messagebox.showerror("Error", "No sections available")
                return
            messagebox.showwarning("Warning", "Please select a section or key to add a new key to that section.")
            return
        key = simpledialog.askstring("Add Key", f"Enter key name for [{section_name}]:", parent=self.root)
        if not key: return
        val = self.ask_value(f"Add Key to [{section_name}]", "")
        if val is not None:
            self.editor.add_key(section_name, key, val)
            self.set_modified(True)
            self.refresh_tree()

    def remove_key(self):
        if not self.editor: return
        selected = self.tree.selection()
        if not selected: return
        item_id = selected[0]
        tags = self.tree.item(item_id, "tags")
        if "key" in tags:
            key = self.tree.item(item_id, "text")
            parent_id = self.tree.parent(item_id)
            section = self.tree.item(parent_id, "text")
            if messagebox.askyesno("Confirm", f"Remove key '{key}' from [{section}]?"):
                self.editor.remove_key(section, key)
                self.set_modified(True)
                self.refresh_tree()
        else:
            messagebox.showinfo("Info", "Please select a key to remove")

    def save_theme(self):
        if not self.editor: return
        try:
            self.editor.save()
            self.set_modified(False)
            messagebox.showinfo("Success", "Theme saved successfully!")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to save: {e}")

    def show_about(self):
        about_dialog = tk.Toplevel(self.root)
        about_dialog.title("About")
        about_dialog.geometry("300x150")
        about_dialog.resizable(False, False)
        about_dialog.transient(self.root)
        
        tk.Label(about_dialog, text="Izotrox Theme Editor", font=("TkDefaultFont", 10)).pack(pady=(20, 5))
        tk.Label(about_dialog, text="Copyright (c) theonlyasdk 2026", font=("TkDefaultFont", 9)).pack(pady=5)
        
        tk.Button(about_dialog, text="OK", command=about_dialog.destroy, width=10).pack(pady=20)


def run_gui():
    root = tk.Tk()
    app = ThemeEditorGUI(root)
    root.mainloop()
