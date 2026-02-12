# Coding style

1. Code Readability
Code should be simple, explicit, and readable. Avoid over-engineering.

2. Memory Management
* Smart Pointers: Use strict ownership semantics. Use C++ smart pointers (`std::shared_ptr` and `std::unique_ptr`, though `std::shared_ptr` should be used sparingly) instead of raw pointers. Use `std::weak_ptr` to break cyclic dependencies.

3. Naming Conventions

* Classes & Structs: `PascalCase` (e.g., `Widget`, `StringBuilder`, `TextEditor`).
* Functions & Methods: `snake_case` (e.g., `create_child`, `resize_bitmap`).
* Variables: `snake_case` (e.g., `file_size`, `buffer_index`).
* Member Variables: `m_snake_case` (e.g., `m_size`, `m_parent_widget`). Private members must strictly follow this prefix.
* Getters/Setters:
* Getter: `property_name()` (Noun, no `get_` prefix).
* Setter: `set_property_name(...)` (Verb).
* File Names: `PascalCase.cpp` / `PascalCase.h` matching the class name.
* Event handlers: `on_event()`, never `set_on_event()`. Event handlers must strictly follow `on_` prefix.
* Variable names: Variable names can be elaborate. Avoid name abbreviation unless necessary.

4. Output:
* Debug/Error output: Only use `Izo::Logger::the()` and never `std:cerr` or `std:cout`!!

6. Formatting & Style
* Indentation: 4 spaces.
* Comments: Minimal. Code should be self-documenting.
* License Header: All files must start with the standard Mozilla Public License version 2.0. license header.

Do not run cmake or ninja directly. To compile the project, run make on the root of the project.