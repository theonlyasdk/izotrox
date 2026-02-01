#!/usr/bin/env python3
import os
import json
import tkinter as tk
from tkinter import ttk, messagebox, simpledialog
from pathlib import Path


class SchemaEditor:
    def __init__(self, parent, schema_path):
        self.parent = parent
        self.schema_path = schema_path
        self.schema = {}
        self.modified = False
        
        self.window = tk.Toplevel(parent)
        self.window.title("Schema Editor")
        self.window.geometry("700x500")
        self.window.minsize(700, 500)
        self.window.transient(parent)
        
        self.create_widgets()
        self.load_schema()
        self.refresh_tree()
        
        self.window.protocol("WM_DELETE_WINDOW", self.on_closing)
    
    def on_closing(self):
        if self.modified:
            response = messagebox.askyesnocancel("Unsaved Changes", "Do you want to save changes before closing?")
            if response is None:
                return
            if response:
                self.save_schema()
        self.window.destroy()
    
    def create_widgets(self):
        toolbar = ttk.Frame(self.window)
        toolbar.pack(fill=tk.X, padx=5, pady=5)
        
        tk.Button(toolbar, text="Add Section", command=self.add_section, width=12).pack(side=tk.LEFT, padx=2)
        tk.Button(toolbar, text="Remove Section", command=self.remove_section, width=12).pack(side=tk.LEFT, padx=2)
        tk.Button(toolbar, text="Add Key", command=self.add_key, width=12).pack(side=tk.LEFT, padx=2)
        tk.Button(toolbar, text="Remove Key", command=self.remove_key, width=12).pack(side=tk.LEFT, padx=2)
        tk.Button(toolbar, text="Save", command=self.save_schema, width=12).pack(side=tk.RIGHT, padx=2)
        
        tree_frame = ttk.Frame(self.window)
        tree_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        self.tree = ttk.Treeview(tree_frame, columns=("type",), show="tree headings")
        self.tree.heading("#0", text="Name")
        self.tree.heading("type", text="Type")
        self.tree.column("#0", width=400)
        self.tree.column("type", width=200)
        
        scrollbar = ttk.Scrollbar(tree_frame, orient=tk.VERTICAL, command=self.tree.yview)
        self.tree.configure(yscroll=scrollbar.set)
        
        self.tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        self.tree.bind("<Double-1>", self.on_double_click)
        
        status_frame = ttk.Frame(self.window)
        status_frame.pack(fill=tk.X, padx=5, pady=5)
        
        self.status_var = tk.StringVar()
        self.status_label = ttk.Label(status_frame, textvariable=self.status_var, relief=tk.SUNKEN, anchor=tk.W)
        self.status_label.pack(fill=tk.X)
        self.status_var.set(f"Schema: {self.schema_path}")
    
    def load_schema(self):
        try:
            if os.path.exists(self.schema_path):
                with open(self.schema_path, 'r') as f:
                    self.schema = json.load(f)
            else:
                self.schema = {}
            self.modified = False
        except Exception as e:
            messagebox.showerror("Error", f"Failed to load schema: {e}")
            self.schema = {}
    
    def save_schema(self):
        try:
            os.makedirs(os.path.dirname(self.schema_path), exist_ok=True)
            with open(self.schema_path, 'w') as f:
                json.dump(self.schema, f, indent=2)
            self.modified = False
            self.status_var.set(f"Schema saved: {self.schema_path}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to save schema: {e}")
    
    def refresh_tree(self):
        for item in self.tree.get_children():
            self.tree.delete(item)
        
        for section_name in sorted(self.schema.keys()):
            section_id = self.tree.insert("", "end", text=section_name, values=("",), tags=("section",), open=True)
            keys = self.schema[section_name]
            for key_name in sorted(keys.keys()):
                key_type = keys[key_name]
                self.tree.insert(section_id, "end", text=key_name, values=(key_type,), tags=("key",))

    
    def on_double_click(self, event):
        selected = self.tree.selection()
        if not selected:
            return
        item_id = selected[0]
        tags = self.tree.item(item_id, "tags")
        if "key" in tags:
            self.edit_key_type(item_id)
    
    def edit_key_type(self, item_id):
        key_name = self.tree.item(item_id, "text")
        parent_id = self.tree.parent(item_id)
        section_name = self.tree.item(parent_id, "text")
        current_type = self.tree.item(item_id, "values")[0]
        
        type_dialog = tk.Toplevel(self.window)
        type_dialog.title("Edit Key Type")
        type_dialog.geometry("350x280")
        type_dialog.minsize(350, 280)
        type_dialog.resizable(False, False)
        type_dialog.transient(self.window)
        type_dialog.wait_visibility()
        type_dialog.grab_set()
        
        tk.Label(type_dialog, text=f"Select type for key '{key_name}':").pack(pady=(15, 10))
        
        listbox_frame = ttk.Frame(type_dialog)
        listbox_frame.pack(fill=tk.BOTH, expand=True, padx=20, pady=5)
        
        types_list = tk.Listbox(listbox_frame)
        types_list.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        scrollbar = ttk.Scrollbar(listbox_frame, orient=tk.VERTICAL, command=types_list.yview)
        types_list.configure(yscrollcommand=scrollbar.set)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        available_types = ["color", "int", "float", "string", "bool", "path"]
        for type_name in available_types:
            types_list.insert(tk.END, type_name)
            if type_name == current_type:
                types_list.selection_set(available_types.index(type_name))
        
        result = [None]
        
        def on_ok():
            selection = types_list.curselection()
            if selection:
                result[0] = types_list.get(selection[0])
            type_dialog.destroy()
        
        def on_cancel():
            type_dialog.destroy()
        
        def on_double_click_type(event):
            on_ok()
        
        types_list.bind("<Double-Button-1>", on_double_click_type)
        
        btn_frame = tk.Frame(type_dialog)
        btn_frame.pack(pady=10)
        tk.Button(btn_frame, text="OK", command=on_ok, width=10, height=2).pack(side=tk.LEFT, padx=5)
        tk.Button(btn_frame, text="Cancel", command=on_cancel, width=10, height=2).pack(side=tk.LEFT, padx=5)
        
        type_dialog.wait_window()
        
        if result[0]:
            self.schema[section_name][key_name] = result[0]
            self.modified = True
            self.refresh_tree()
    
    def add_section(self):
        section_name = simpledialog.askstring("Add Section", "Enter section name:", parent=self.window)
        if not section_name:
            return
        if section_name in self.schema:
            messagebox.showerror("Error", f"Section '{section_name}' already exists")
            return
        self.schema[section_name] = {}
        self.modified = True
        self.refresh_tree()
    
    def remove_section(self):
        selected = self.tree.selection()
        if not selected:
            return
        item_id = selected[0]
        tags = self.tree.item(item_id, "tags")
        if "section" not in tags:
            messagebox.showinfo("Info", "Please select a section to remove")
            return
        section_name = self.tree.item(item_id, "text")
        if messagebox.askyesno("Confirm", f"Remove section '{section_name}' and all its keys?"):
            del self.schema[section_name]
            self.modified = True
            self.refresh_tree()
    
    def add_key(self):
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
            messagebox.showinfo("Info", "Please select a section or key to add a new key")
            return
        
        key_name = simpledialog.askstring("Add Key", f"Enter key name for [{section_name}]:", parent=self.window)
        if not key_name:
            return
        if key_name in self.schema[section_name]:
            messagebox.showerror("Error", f"Key '{key_name}' already exists in section '{section_name}'")
            return
        
        self.schema[section_name][key_name] = "string"
        self.modified = True
        self.refresh_tree()
    
    def remove_key(self):
        selected = self.tree.selection()
        if not selected:
            return
        item_id = selected[0]
        tags = self.tree.item(item_id, "tags")
        if "key" not in tags:
            messagebox.showinfo("Info", "Please select a key to remove")
            return
        key_name = self.tree.item(item_id, "text")
        parent_id = self.tree.parent(item_id)
        section_name = self.tree.item(parent_id, "text")
        if messagebox.askyesno("Confirm", f"Remove key '{key_name}' from [{section_name}]?"):
            del self.schema[section_name][key_name]
            self.modified = True
            self.refresh_tree()


def open_schema_editor(parent, schema_path):
    SchemaEditor(parent, schema_path)
