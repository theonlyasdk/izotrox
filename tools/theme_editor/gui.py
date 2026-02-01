#!/usr/bin/env python3
import os
import json
import tkinter as tk
from tkinter import ttk, messagebox, simpledialog, filedialog, colorchooser
from pathlib import Path

from .core import ThemeEditor, get_theme_dir, load_enums
from .schema_editor import open_schema_editor



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
        
        self.apply_theme()
        
        self.setup_keybindings()
        
        self.theme_dir = get_theme_dir()
        self.status_var.set("Ready")

    def setup_keybindings(self):
        self.root.bind("<Control-n>", lambda e: self.new_theme_menu())
        self.root.bind("<Control-N>", lambda e: self.new_theme_menu())
        self.root.bind("<Control-o>", lambda e: self.open_theme())
        self.root.bind("<Control-O>", lambda e: self.open_theme())
        self.root.bind("<Control-s>", lambda e: self.save_theme())
        self.root.bind("<Control-S>", lambda e: self.save_theme())
        self.root.bind("<Control-w>", lambda e: self.close_theme())
        self.root.bind("<Control-W>", lambda e: self.close_theme())


    def apply_theme(self):
        try:
            style = ttk.Style(self.root)
            available_themes = style.theme_names()
            
            preferred_themes = ['clam', 'alt', 'default', 'classic']
            
            for theme in preferred_themes:
                if theme in available_themes:
                    style.theme_use(theme)
                    break
        except Exception:
            pass


    def on_closing(self):
        if self.modified:
            if not messagebox.askyesno("Unsaved Changes", "You have unsaved changes. Exit anyway?"):
                return
        self.root.destroy()

    def create_menu(self):
        menubar = tk.Menu(self.root)
        
        file_menu = tk.Menu(menubar, tearoff=0)
        file_menu.add_command(label="New Theme...", command=self.new_theme_menu)
        file_menu.add_command(label="Open...", command=self.open_theme)
        file_menu.add_command(label="Close Theme", command=self.close_theme)
        file_menu.add_separator()
        file_menu.add_command(label="Save", command=self.save_theme)
        file_menu.add_separator()
        file_menu.add_command(label="Preferences", command=self.show_preferences)
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self.on_closing)
        menubar.add_cascade(label="File", menu=file_menu)


        
        self.edit_menu = tk.Menu(menubar, tearoff=0)
        self.edit_menu.add_command(label="Add Section", command=self.add_section)
        self.edit_menu.add_command(label="Remove Section", command=self.remove_section)
        self.edit_menu.add_command(label="Rename Section", command=self.rename_section)
        self.edit_menu.add_separator()
        self.edit_menu.add_command(label="Add Key", command=self.add_key)
        self.edit_menu.add_command(label="Remove Key", command=self.remove_key)
        self.edit_menu.add_command(label="Rename Key", command=self.rename_key)
        self.edit_menu.add_separator()
        self.edit_menu.add_command(label="Move Key to Section", command=self.move_key)
        menubar.add_cascade(label="Edit", menu=self.edit_menu)

        
        self.theme_menu = tk.Menu(menubar, tearoff=0)
        self.theme_menu.add_command(label="Check Errors", command=self.check_errors)
        menubar.add_cascade(label="Theme", menu=self.theme_menu)
        
        help_menu = tk.Menu(menubar, tearoff=0)
        help_menu.add_command(label="About", command=self.show_about)
        menubar.add_cascade(label="Help", menu=help_menu)

        
        self.root.config(menu=menubar)
        
        self.update_menu_state()

    def update_menu_state(self):
        if self.editor:
            for i in range(self.edit_menu.index("end") + 1):
                try:
                    self.edit_menu.entryconfig(i, state=tk.NORMAL)
                except tk.TclError:
                    pass
            for i in range(self.theme_menu.index("end") + 1):
                try:
                    self.theme_menu.entryconfig(i, state=tk.NORMAL)
                except tk.TclError:
                    pass
        else:
            for i in range(self.edit_menu.index("end") + 1):
                try:
                    self.edit_menu.entryconfig(i, state=tk.DISABLED)
                except tk.TclError:
                    pass
            for i in range(self.theme_menu.index("end") + 1):
                try:
                    self.theme_menu.entryconfig(i, state=tk.DISABLED)
                except tk.TclError:
                    pass


    def create_widgets(self):
        self.main_container = ttk.Frame(self.root)
        self.main_container.pack(fill=tk.BOTH, expand=True)
        
        self.welcome_frame = ttk.Frame(self.main_container)
        self.tree_container = ttk.Frame(self.main_container)
        
        self.create_welcome_screen()
        self.create_tree_view()
        
        self.show_welcome_screen()

    def create_welcome_screen(self):
        center_frame = ttk.Frame(self.welcome_frame)
        center_frame.place(relx=0.5, rely=0.5, anchor=tk.CENTER)
        
        open_btn = tk.Button(center_frame, text="Open Theme (.ini)", command=self.open_theme, 
                            width=25, height=2, font=("TkDefaultFont", 11))
        open_btn.pack(pady=(0, 15))
        
        create_btn = tk.Button(center_frame, text="Create Theme (.ini)", command=self.create_theme,
                              width=25, height=2, font=("TkDefaultFont", 11))
        create_btn.pack()

    def create_tree_view(self):
        self.tree_frame = ttk.Frame(self.tree_container)
        self.tree_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        self.tree = ttk.Treeview(self.tree_frame, columns=("value"), show="tree headings", selectmode='extended')
        self.tree.heading("#0", text="Name")
        self.tree.heading("value", text="Value")
        self.tree.column("#0", width=250, minwidth=100)
        self.tree.column("value", width=400, minwidth=200)
        
        self.scrollbar = ttk.Scrollbar(self.tree_frame, orient=tk.VERTICAL, command=self.tree.yview)
        self.tree.configure(yscroll=self.scrollbar.set)
        
        self.tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self.scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        self.tree.bind("<Double-1>", self.on_double_click)

    def show_welcome_screen(self):
        self.tree_container.pack_forget()
        self.welcome_frame.pack(fill=tk.BOTH, expand=True)

    def show_tree_view(self):
        self.welcome_frame.pack_forget()
        self.tree_container.pack(fill=tk.BOTH, expand=True)


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

    def create_theme(self):
        initial_dir = self.theme_dir if self.theme_dir.exists() else os.getcwd()
        
        create_dialog = tk.Toplevel(self.root)
        create_dialog.title("Create New Theme")
        create_dialog.geometry("450x240")
        create_dialog.minsize(450, 240)
        create_dialog.resizable(True, False)
        create_dialog.transient(self.root)
        create_dialog.wait_visibility()
        create_dialog.grab_set()
        
        tk.Label(create_dialog, text="Theme Name:").grid(row=0, column=0, sticky=tk.W, padx=20, pady=(20, 5))
        name_entry = tk.Entry(create_dialog, width=30)
        name_entry.grid(row=0, column=1, columnspan=2, padx=(0, 20), pady=(20, 5), sticky=tk.EW)
        
        tk.Label(create_dialog, text="Author:").grid(row=1, column=0, sticky=tk.W, padx=20, pady=5)
        author_entry = tk.Entry(create_dialog, width=30)
        author_entry.grid(row=1, column=1, columnspan=2, padx=(0, 20), pady=5, sticky=tk.EW)
        
        tk.Label(create_dialog, text="Version:").grid(row=2, column=0, sticky=tk.W, padx=20, pady=5)
        version_entry = tk.Entry(create_dialog, width=30)
        version_entry.insert(0, "1.0.0")
        version_entry.grid(row=2, column=1, columnspan=2, padx=(0, 20), pady=5, sticky=tk.EW)
        
        tk.Label(create_dialog, text="Base Theme:").grid(row=3, column=0, sticky=tk.W, padx=20, pady=5)
        base_theme_var = tk.StringVar(value=str(self.theme_dir / "default.ini"))
        base_theme_entry = tk.Entry(create_dialog, textvariable=base_theme_var, width=30, state='readonly')
        base_theme_entry.grid(row=3, column=1, padx=(0, 5), pady=5, sticky=tk.EW)

        
        def choose_base_theme():
            filename = filedialog.askopenfilename(
                initialdir=self.theme_dir if self.theme_dir.exists() else os.getcwd(),
                title="Select Base Theme",
                filetypes=(("INI files", "*.ini"), ("All files", "*.*")),
                parent=create_dialog
            )
            if filename:
                base_theme_var.set(filename)
        
        choose_btn = tk.Button(create_dialog, text="Choose", command=choose_base_theme, width=8)
        choose_btn.grid(row=3, column=2, padx=(0, 20), pady=5)
        
        create_dialog.grid_columnconfigure(1, weight=1)
        
        result = [None]
        
        def on_create():
            theme_name = name_entry.get().strip()
            author = author_entry.get().strip()
            version = version_entry.get().strip()
            base_theme = base_theme_var.get()
            
            if not theme_name:
                messagebox.showwarning("Warning", "Please enter a theme name", parent=create_dialog)
                return
            
            if not author:
                messagebox.showwarning("Warning", "Please enter an author name", parent=create_dialog)
                return
            
            if not version:
                messagebox.showwarning("Warning", "Please enter a version", parent=create_dialog)
                return
            
            save_path = filedialog.asksaveasfilename(
                initialdir=initial_dir,
                title="Save Theme As",
                defaultextension=".ini",
                filetypes=(("INI files", "*.ini"), ("All files", "*.*")),
                parent=create_dialog
            )
            
            if not save_path:
                return
            
            result[0] = (save_path, theme_name, author, version, base_theme)
            create_dialog.destroy()
        
        def on_cancel():
            create_dialog.destroy()
        
        btn_frame = tk.Frame(create_dialog)
        btn_frame.grid(row=4, column=0, columnspan=3, pady=(15, 10))
        tk.Button(btn_frame, text="Create", command=on_create, width=10).pack(side=tk.LEFT, padx=5)
        tk.Button(btn_frame, text="Cancel", command=on_cancel, width=10).pack(side=tk.LEFT, padx=5)

        
        create_dialog.wait_window()
        
        if result[0]:
            save_path, theme_name, author, version, base_theme = result[0]
            try:
                if os.path.exists(base_theme):
                    self.editor = ThemeEditor(base_theme)
                else:
                    self.editor = ThemeEditor.__new__(ThemeEditor)
                    self.editor.theme_path = save_path
                    self.editor.sections = {}
                
                if "ThemeManifest" not in self.editor.sections:
                    self.editor.sections["ThemeManifest"] = []
                
                self.editor.add_key("ThemeManifest", "Name", theme_name)
                self.editor.add_key("ThemeManifest", "Author", author)
                self.editor.add_key("ThemeManifest", "Version", version)
                
                self.editor.theme_path = save_path
                self.editor.save()
                
                self.current_file = save_path
                self.modified = False
                self.refresh_tree()
                self.show_tree_view()
                self.status_var.set(f"Created: {save_path}")
                self.root.title(f"Izotrox Theme Editor - {os.path.basename(save_path)}")
            except Exception as e:
                messagebox.showerror("Error", f"Failed to create theme: {e}")

    def close_theme(self):
        if not self.editor:
            return
        if self.modified:
            response = messagebox.askyesnocancel("Unsaved Changes", "Do you want to save changes before closing?")
            if response is None:
                return
            if response:
                self.save_theme()
        
        self.editor = None
        self.current_file = None
        self.modified = False
        self.show_welcome_screen()
        self.update_menu_state()
        self.status_var.set("Ready")
        self.root.title("Izotrox Theme Editor")


    def new_theme_menu(self):
        if self.editor and self.modified:
            response = messagebox.askyesnocancel("Unsaved Changes", "Do you want to save changes before creating a new theme?")
            if response is None:
                return
            if response:
                self.save_theme()
        
        self.create_theme()


    def load_file(self, filename):
        try:
            self.editor = ThemeEditor(filename)
            self.current_file = filename
            self.modified = False
            self.refresh_tree()
            self.show_tree_view()
            self.update_menu_state()
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

        def on_double_click(event):
            on_ok()

        tree.bind("<Double-Button-1>", on_double_click)

        btn_frame = tk.Frame(enum_dialog)
        btn_frame.pack(pady=10)
        tk.Button(btn_frame, text="OK", command=on_ok, width=10).pack(side=tk.LEFT, padx=5)
        tk.Button(btn_frame, text="Cancel", command=on_cancel, width=10).pack(side=tk.LEFT, padx=5)


        enum_dialog.wait_window()
        return result[0]

    def get_key_type_from_schema(self, section, key):
        schema = self.load_schema()
        if section in schema and key in schema[section]:
            return schema[section][key]
        return "any"

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
        
        key_type = self.get_key_type_from_schema(section, key)
        
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
                new_val = self.ask_value(f"Edit '{key}' ({key_type})", current_val)
        else:
            new_val = self.ask_value(f"Edit '{key}' ({key_type})", current_val)
        
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
        if not self.editor:
            return
        selected = self.tree.selection()
        if not selected:
            return
        
        sections_to_remove = []
        for item_id in selected:
            tags = self.tree.item(item_id, "tags")
            if "section" in tags:
                sections_to_remove.append(self.tree.item(item_id, "text"))
        
        if not sections_to_remove:
            messagebox.showinfo("Info", "Please select section(s) to remove")
            return
        
        section_list = "\n".join(f"  - {s}" for s in sections_to_remove)
        if messagebox.askyesno("Confirm", f"Remove {len(sections_to_remove)} section(s) and all their keys?\n\n{section_list}"):
            for section_name in sections_to_remove:
                self.editor.remove_section(section_name)
            self.set_modified(True)
            self.refresh_tree()


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
        if not self.editor:
            return
        selected = self.tree.selection()
        if not selected:
            return
        
        keys_to_remove = []
        for item_id in selected:
            tags = self.tree.item(item_id, "tags")
            if "key" in tags:
                key_name = self.tree.item(item_id, "text")
                parent_id = self.tree.parent(item_id)
                section_name = self.tree.item(parent_id, "text")
                keys_to_remove.append((section_name, key_name))
        
        if not keys_to_remove:
            messagebox.showinfo("Info", "Please select key(s) to remove")
            return
        
        key_list = "\n".join(f"  - [{section}] {key}" for section, key in keys_to_remove)
        if messagebox.askyesno("Confirm", f"Remove {len(keys_to_remove)} key(s)?\n\n{key_list}"):
            for section_name, key_name in keys_to_remove:
                self.editor.remove_key(section_name, key_name)
            self.set_modified(True)
            self.refresh_tree()


    def rename_section(self):
        if not self.editor:
            return
        selected = self.tree.selection()
        if not selected:
            messagebox.showinfo("Info", "Please select section(s) to rename")
            return
        
        sections_to_rename = []
        for item_id in selected:
            tags = self.tree.item(item_id, "tags")
            if "section" in tags:
                sections_to_rename.append(self.tree.item(item_id, "text"))
        
        if not sections_to_rename:
            messagebox.showinfo("Info", "Please select section(s) to rename")
            return
        
        if len(sections_to_rename) == 1:
            old_name = sections_to_rename[0]
            
            rename_dialog = tk.Toplevel(self.root)
            rename_dialog.title("Rename Section")
            rename_dialog.geometry("400x120")
            rename_dialog.minsize(400, 120)
            rename_dialog.resizable(True, False)
            rename_dialog.transient(self.root)
            rename_dialog.wait_visibility()
            rename_dialog.grab_set()
            
            tk.Label(rename_dialog, text=f"Enter new name for section '{old_name}':").pack(pady=(15, 5), padx=20, anchor=tk.W)
            name_entry = tk.Entry(rename_dialog)
            name_entry.insert(0, old_name)
            name_entry.pack(fill=tk.X, padx=20, pady=(0, 10))
            name_entry.select_range(0, tk.END)
            name_entry.focus()
            
            result = [None]
            
            def on_ok(event=None):
                result[0] = name_entry.get()
                rename_dialog.destroy()
            
            def on_cancel():
                rename_dialog.destroy()
            
            btn_frame = tk.Frame(rename_dialog)
            btn_frame.pack(pady=(5, 10))
            tk.Button(btn_frame, text="OK", command=on_ok, width=10).pack(side=tk.LEFT, padx=5)
            tk.Button(btn_frame, text="Cancel", command=on_cancel, width=10).pack(side=tk.LEFT, padx=5)
            
            name_entry.bind("<Return>", on_ok)
            
            rename_dialog.wait_window()
            
            new_name = result[0]
            if not new_name or new_name == old_name:
                return
            if new_name in self.editor.sections:
                messagebox.showerror("Error", f"Section '{new_name}' already exists")
                return
            if self.editor.rename_section(old_name, new_name):
                self.set_modified(True)
                self.refresh_tree()
            else:
                messagebox.showerror("Error", "Failed to rename section")
        else:
            batch_dialog = tk.Toplevel(self.root)
            batch_dialog.title("Batch Rename Sections")
            batch_dialog.geometry("400x200")
            batch_dialog.minsize(400, 200)
            batch_dialog.resizable(True, False)
            batch_dialog.transient(self.root)
            batch_dialog.wait_visibility()
            batch_dialog.grab_set()
            
            tk.Label(batch_dialog, text=f"Renaming {len(sections_to_rename)} sections").pack(pady=(15, 10))
            
            tk.Label(batch_dialog, text="Prefix:").pack(anchor=tk.W, padx=20)
            prefix_entry = tk.Entry(batch_dialog)
            prefix_entry.pack(fill=tk.X, padx=20, pady=(0, 10))
            
            tk.Label(batch_dialog, text="Suffix:").pack(anchor=tk.W, padx=20)
            suffix_entry = tk.Entry(batch_dialog)
            suffix_entry.pack(fill=tk.X, padx=20, pady=(0, 10))
            
            result = [False]
            
            def on_apply():
                prefix = prefix_entry.get()
                suffix = suffix_entry.get()
                if not prefix and not suffix:
                    messagebox.showwarning("Warning", "Please enter a prefix or suffix", parent=batch_dialog)
                    return
                
                conflicts = []
                for old_name in sections_to_rename:
                    new_name = f"{prefix}{old_name}{suffix}"
                    if new_name in self.editor.sections and new_name not in sections_to_rename:
                        conflicts.append(new_name)
                
                if conflicts:
                    messagebox.showerror("Error", f"Name conflicts detected:\n" + "\n".join(f"  - {c}" for c in conflicts), parent=batch_dialog)
                    return
                
                result[0] = True
                batch_dialog.destroy()
            
            def on_cancel():
                batch_dialog.destroy()
            
            btn_frame = tk.Frame(batch_dialog)
            btn_frame.pack(pady=(5, 10))
            tk.Button(btn_frame, text="Apply", command=on_apply, width=10).pack(side=tk.LEFT, padx=5)
            tk.Button(btn_frame, text="Cancel", command=on_cancel, width=10).pack(side=tk.LEFT, padx=5)

            
            batch_dialog.wait_window()
            
            if result[0]:
                prefix = prefix_entry.get()
                suffix = suffix_entry.get()
                for old_name in sections_to_rename:
                    new_name = f"{prefix}{old_name}{suffix}"
                    self.editor.rename_section(old_name, new_name)
                self.set_modified(True)
                self.refresh_tree()


    def rename_key(self):
        if not self.editor:
            return
        selected = self.tree.selection()
        if not selected:
            messagebox.showinfo("Info", "Please select key(s) to rename")
            return
        
        keys_to_rename = []
        for item_id in selected:
            tags = self.tree.item(item_id, "tags")
            if "key" in tags:
                key_name = self.tree.item(item_id, "text")
                parent_id = self.tree.parent(item_id)
                section_name = self.tree.item(parent_id, "text")
                keys_to_rename.append((section_name, key_name))
        
        if not keys_to_rename:
            messagebox.showinfo("Info", "Please select key(s) to rename")
            return
        
        if len(keys_to_rename) == 1:
            section_name, old_key_name = keys_to_rename[0]
            
            rename_dialog = tk.Toplevel(self.root)
            rename_dialog.title("Rename Key")
            rename_dialog.geometry("400x120")
            rename_dialog.minsize(400, 120)
            rename_dialog.resizable(True, False)
            rename_dialog.transient(self.root)
            rename_dialog.wait_visibility()
            rename_dialog.grab_set()
            
            tk.Label(rename_dialog, text=f"Enter new name for key '{old_key_name}':").pack(pady=(15, 5), padx=20, anchor=tk.W)
            name_entry = tk.Entry(rename_dialog)
            name_entry.insert(0, old_key_name)
            name_entry.pack(fill=tk.X, padx=20, pady=(0, 10))
            name_entry.select_range(0, tk.END)
            name_entry.focus()
            
            result = [None]
            
            def on_ok(event=None):
                result[0] = name_entry.get()
                rename_dialog.destroy()
            
            def on_cancel():
                rename_dialog.destroy()
            
            btn_frame = tk.Frame(rename_dialog)
            btn_frame.pack(pady=(5, 10))
            tk.Button(btn_frame, text="OK", command=on_ok, width=10).pack(side=tk.LEFT, padx=5)
            tk.Button(btn_frame, text="Cancel", command=on_cancel, width=10).pack(side=tk.LEFT, padx=5)
            
            name_entry.bind("<Return>", on_ok)
            
            rename_dialog.wait_window()
            
            new_key_name = result[0]
            if not new_key_name or new_key_name == old_key_name:
                return
            if '=' in new_key_name:
                messagebox.showerror("Error", "Key name cannot contain '='")
                return
            if self.editor.rename_key(section_name, old_key_name, new_key_name):
                self.set_modified(True)
                self.refresh_tree()
            else:
                messagebox.showerror("Error", "Failed to rename key")
        else:
            batch_dialog = tk.Toplevel(self.root)
            batch_dialog.title("Batch Rename Keys")
            batch_dialog.geometry("400x200")
            batch_dialog.minsize(400, 200)
            batch_dialog.resizable(True, False)
            batch_dialog.transient(self.root)
            batch_dialog.wait_visibility()
            batch_dialog.grab_set()
            
            tk.Label(batch_dialog, text=f"Renaming {len(keys_to_rename)} keys").pack(pady=(15, 10))
            
            tk.Label(batch_dialog, text="Prefix:").pack(anchor=tk.W, padx=20)
            prefix_entry = tk.Entry(batch_dialog)
            prefix_entry.pack(fill=tk.X, padx=20, pady=(0, 10))
            
            tk.Label(batch_dialog, text="Suffix:").pack(anchor=tk.W, padx=20)
            suffix_entry = tk.Entry(batch_dialog)
            suffix_entry.pack(fill=tk.X, padx=20, pady=(0, 10))
            
            result = [False]
            
            def on_apply():
                prefix = prefix_entry.get()
                suffix = suffix_entry.get()
                if not prefix and not suffix:
                    messagebox.showwarning("Warning", "Please enter a prefix or suffix", parent=batch_dialog)
                    return
                if '=' in prefix or '=' in suffix:
                    messagebox.showerror("Error", "Prefix/suffix cannot contain '='", parent=batch_dialog)
                    return
                result[0] = True
                batch_dialog.destroy()
            
            def on_cancel():
                batch_dialog.destroy()
            
            btn_frame = tk.Frame(batch_dialog)
            btn_frame.pack(pady=(5, 10))
            tk.Button(btn_frame, text="Apply", command=on_apply, width=10).pack(side=tk.LEFT, padx=5)
            tk.Button(btn_frame, text="Cancel", command=on_cancel, width=10).pack(side=tk.LEFT, padx=5)

            
            batch_dialog.wait_window()
            
            if result[0]:
                prefix = prefix_entry.get()
                suffix = suffix_entry.get()
                for section_name, old_key_name in keys_to_rename:
                    new_key_name = f"{prefix}{old_key_name}{suffix}"
                    self.editor.rename_key(section_name, old_key_name, new_key_name)
                self.set_modified(True)
                self.refresh_tree()


    def move_key(self):
        if not self.editor:
            return
        selected = self.tree.selection()
        if not selected:
            messagebox.showinfo("Info", "Please select key(s) to move")
            return
        
        keys_to_move = []
        for item_id in selected:
            tags = self.tree.item(item_id, "tags")
            if "key" in tags:
                key_name = self.tree.item(item_id, "text")
                parent_id = self.tree.parent(item_id)
                source_section = self.tree.item(parent_id, "text")
                keys_to_move.append((source_section, key_name))
        
        if not keys_to_move:
            messagebox.showinfo("Info", "Please select key(s) to move")
            return
        
        source_sections = list(set(section for section, _ in keys_to_move))
        available_sections = sorted([s for s in self.editor.sections.keys() if s not in source_sections])
        
        if not available_sections:
            messagebox.showerror("Error", "No other sections available to move the key(s) to")
            return
        
        selection_dialog = tk.Toplevel(self.root)
        selection_dialog.title("Move Keys")
        selection_dialog.geometry("400x350")
        selection_dialog.resizable(False, False)
        selection_dialog.transient(self.root)
        selection_dialog.wait_visibility()
        selection_dialog.grab_set()
        
        if len(keys_to_move) == 1:
            tk.Label(selection_dialog, text=f"Move key '{keys_to_move[0][1]}' from [{keys_to_move[0][0]}] to:").pack(pady=10, padx=10)
        else:
            tk.Label(selection_dialog, text=f"Move {len(keys_to_move)} key(s) to:").pack(pady=10, padx=10)
            key_list_text = "Keys to move:\n" + "\n".join(f"  [{section}] {key}" for section, key in keys_to_move[:10])
            if len(keys_to_move) > 10:
                key_list_text += f"\n  ... and {len(keys_to_move) - 10} more"
            tk.Label(selection_dialog, text=key_list_text, justify=tk.LEFT, font=("TkDefaultFont", 8)).pack(pady=(0, 10), padx=20)
        
        listbox_frame = ttk.Frame(selection_dialog)
        listbox_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)
        
        listbox = tk.Listbox(listbox_frame)
        listbox.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        scrollbar = ttk.Scrollbar(listbox_frame, orient=tk.VERTICAL, command=listbox.yview)
        listbox.configure(yscrollcommand=scrollbar.set)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        for section in available_sections:
            listbox.insert(tk.END, section)
        
        result = [None]
        
        def on_ok():
            selection = listbox.curselection()
            if selection:
                result[0] = listbox.get(selection[0])
                selection_dialog.destroy()
        
        def on_cancel():
            selection_dialog.destroy()
        
        def on_double_click(event):
            on_ok()
        
        listbox.bind("<Double-Button-1>", on_double_click)
        
        btn_frame = tk.Frame(selection_dialog)
        btn_frame.pack(pady=10)
        tk.Button(btn_frame, text="OK", command=on_ok, width=10).pack(side=tk.LEFT, padx=5)
        tk.Button(btn_frame, text="Cancel", command=on_cancel, width=10).pack(side=tk.LEFT, padx=5)
        
        selection_dialog.wait_window()
        
        if result[0]:
            target_section = result[0]
            for source_section, key_name in keys_to_move:
                self.editor.move_key(source_section, key_name, target_section)
            self.set_modified(True)
            self.refresh_tree()


    def validate_color_format(self, value):
        parts = value.split(',')
        if len(parts) < 3 or len(parts) > 4:
            return False
        for part in parts:
            part = part.strip()
            if not part:
                return False
            try:
                num = int(part)
                if num < 0 or num > 255:
                    return False
            except ValueError:
                return False
        return True

    def validate_int_format(self, value):
        try:
            int(value)
            return True
        except ValueError:
            return False

    def validate_float_format(self, value):
        try:
            float(value)
            return True
        except ValueError:
            return False

    def validate_bool_format(self, value):
        return value.lower() in ['true', 'false', '1', '0', 'yes', 'no']

    def validate_string_format(self, value):
        return True

    def validate_path_format(self, value):
        return len(value) > 0

    def get_validator_for_type(self, type_name):
        validators = {
            "color": self.validate_color_format,
            "int": self.validate_int_format,
            "float": self.validate_float_format,
            "bool": self.validate_bool_format,
            "string": self.validate_string_format,
            "path": self.validate_path_format
        }
        return validators.get(type_name, self.validate_string_format)

    def load_schema(self):
        schema_path = self.get_schema_path()
        try:
            if os.path.exists(schema_path):
                with open(schema_path, 'r') as f:
                    return json.load(f)
        except Exception:
            pass
        return {}

    def get_validators_for_section(self, section_name):
        schema = self.load_schema()
        validators = []
        
        if section_name in schema:
            section_schema = schema[section_name]
            validators.append(("schema-based", section_schema))
        
        return validators


    def check_errors(self):
        if not self.editor:
            messagebox.showinfo("Info", "No theme file is currently open")
            return

        error_dialog = tk.Toplevel(self.root)
        error_dialog.title("Check Theme Errors")
        error_dialog.geometry("600x400")
        error_dialog.minsize(600, 400)
        error_dialog.resizable(True, True)
        error_dialog.transient(self.root)
        error_dialog.wait_visibility()
        error_dialog.grab_set()

        tk.Label(error_dialog, text="Checking theme for errors...").pack(pady=(10, 5))

        progress = ttk.Progressbar(error_dialog, mode='determinate', length=560)
        progress.pack(padx=20, pady=5)

        status_label = tk.Label(error_dialog, text="Initializing...")
        status_label.pack(pady=5)

        result_frame = ttk.Frame(error_dialog)
        result_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        result_listbox = tk.Listbox(result_frame, font=("Courier", 9))
        result_listbox.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        result_scrollbar = ttk.Scrollbar(result_frame, orient=tk.VERTICAL, command=result_listbox.yview)
        result_listbox.configure(yscrollcommand=result_scrollbar.set)
        result_scrollbar.pack(side=tk.RIGHT, fill=tk.Y)

        btn_frame = tk.Frame(error_dialog)
        btn_frame.pack(pady=10)

        close_button = tk.Button(btn_frame, text="Close", command=error_dialog.destroy, width=10, state=tk.DISABLED)
        close_button.pack()

        errors = []

        def run_checks():
            total_keys = sum(len(lines) for lines in self.editor.sections.values())
            current_progress = 0

            for section_name, lines in self.editor.sections.items():
                validators = self.get_validators_for_section(section_name)

                for line_idx, line in enumerate(lines):
                    if '=' in line:
                        key = line.split('=', 1)[0].strip()
                        value = line.split('=', 1)[1].strip()

                        status_label.config(text=f"Checking [{section_name}] {key}...")
                        error_dialog.update()

                        for validator_name, section_schema in validators:
                            if validator_name == "schema-based":
                                if key in section_schema:
                                    expected_type = section_schema[key]
                                    validator_func = self.get_validator_for_type(expected_type)
                                    if not validator_func(value):
                                        error_msg = f"[{section_name}] {key} = {value}"
                                        error_detail = f"  Error: Invalid {expected_type} format"
                                        errors.append((error_msg, error_detail))

                    current_progress += 1
                    progress['value'] = (current_progress / total_keys) * 100
                    error_dialog.update()

            progress['value'] = 100
            status_label.config(text="Check complete!")

            if errors:
                result_listbox.insert(tk.END, f"Found {len(errors)} error(s):")
                result_listbox.insert(tk.END, "")
                for error_msg, error_detail in errors:
                    result_listbox.insert(tk.END, error_msg)
                    result_listbox.insert(tk.END, error_detail)
                    result_listbox.insert(tk.END, "")
            else:
                result_listbox.insert(tk.END, "No errors found!")
                result_listbox.insert(tk.END, "")
                result_listbox.insert(tk.END, "Your theme file is valid.")

            close_button.config(state=tk.NORMAL)

        error_dialog.after(100, run_checks)
        error_dialog.wait_window()

    def save_theme(self):
        if not self.editor: return
        try:
            self.editor.save()
            self.set_modified(False)
            messagebox.showinfo("Success", "Theme saved successfully!")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to save: {e}")

    def get_schema_path(self):
        return Path(__file__).parent / "data" / "schema.json"

    def show_preferences(self):
        pref_dialog = tk.Toplevel(self.root)
        pref_dialog.title("Preferences")
        pref_dialog.geometry("650x450")
        pref_dialog.minsize(650, 450)
        pref_dialog.resizable(True, True)
        pref_dialog.transient(self.root)
        
        notebook = ttk.Notebook(pref_dialog)
        notebook.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        general_tab = ttk.Frame(notebook)
        notebook.add(general_tab, text="General")
        
        general_frame = ttk.Frame(general_tab)
        general_frame.pack(fill=tk.BOTH, expand=True, padx=20, pady=20)
        
        tk.Label(general_frame, text="General Settings", font=("TkDefaultFont", 12, "bold")).pack(anchor=tk.W, pady=(0, 10))
        tk.Label(general_frame, text="General preferences and application settings.", 
                wraplength=590, justify=tk.LEFT).pack(anchor=tk.W, pady=(0, 20))
        
        schema_tab = ttk.Frame(notebook)
        notebook.add(schema_tab, text="Schema")
        
        schema_info_frame = ttk.Frame(schema_tab)
        schema_info_frame.pack(fill=tk.BOTH, expand=True, padx=20, pady=20)
        
        tk.Label(schema_info_frame, text="Schema Configuration", font=("TkDefaultFont", 12, "bold")).pack(anchor=tk.W, pady=(0, 10))
        tk.Label(schema_info_frame, text="The schema defines the structure and data types for theme validation.", 
                wraplength=590, justify=tk.LEFT).pack(anchor=tk.W, pady=(0, 20))
        
        schema_path = self.get_schema_path()
        tk.Label(schema_info_frame, text=f"Schema File: {schema_path}", 
                font=("TkDefaultFont", 9)).pack(anchor=tk.W, pady=(0, 20))
        
        def edit_schema():
            open_schema_editor(pref_dialog, str(schema_path))
        
        tk.Button(schema_info_frame, text="Edit Default Schema", command=edit_schema, 
                 width=20, height=2).pack(anchor=tk.W)
        
        btn_frame = tk.Frame(pref_dialog)
        btn_frame.pack(pady=10)
        tk.Button(btn_frame, text="Close", command=pref_dialog.destroy, width=10).pack()

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
