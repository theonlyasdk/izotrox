local tb = nil
local main_layout = nil

-- Colors
local COLOR_WHITE = Color.from_rgb(255, 255, 255)
local COLOR_GREY = Color.from_rgb(150, 150, 150)
local COLOR_RED = Color.from_rgb(255, 80, 80)
local COLOR_ORANGE = Color.from_rgb(255, 165, 0)

-- Redirect print to layout
function print(...)
    local args = {...}
    local str = ""
    for i, v in ipairs(args) do
        str = str .. tostring(v) .. (i < #args and "\t" or "")
    end
    
    local lbl = Label.create(0, 0, Screen.width() - 20, WRAP_CONTENT, str)
    Label.set_multiline(lbl, true)
    Label.set_color(lbl, COLOR_WHITE)
    Layout.add_label(main_layout, lbl)
end

function show_help()
    print("AnKrypton Lua Help:")
    print("Available Modules:")
    print(" - App: set_view(layout, 0)")
    print(" - Layout: create(type, x, y, w, h, param), add_button, add_textbox, add_label, add_progressbar")
    print(" - Label: create, set_text, set_font_size, set_color, set_multiline, set_scroll_on_overflow")
    print(" - Button: create, set_width, set_height")
    print(" - TextBox: create, set_text, get_text, set_hide_on_finish")
    print(" - ProgressBar: create, set_progress")
    print(" - Canvas: fill_rect, clear, set_pixel, outline_rect, gradient_rect, fill_round_rect")
    print(" - Graphics: intersect_clip")
    print(" - System: show_debug_info(bool)")
    print(" - Screen: width(), height()")
end

function init()
    screen_w = Screen.width()
    screen_h = Screen.height()
    System.debug(true)
    
    main_layout = Layout.create(LAYOUT_LINEAR, 0, 0, screen_w, screen_h, 0)
    
    -- Shell TextBox
    tb = TextBox.create(0, 0, screen_w, 60)
    TextBox.set_width(tb, MATCH_PARENT)
    TextBox.set_placeholder(tb, "Lua Shell: type code...")
    TextBox.set_hide_on_finish(tb, true)
    Layout.add_textbox(main_layout, tb)
    
    show_help()

    TextBox.on_finish(tb, function(text)
        if text == "" then return end
        
        -- Echo command
        local echo = Label.create(0, 0, screen_w - 20, WRAP_CONTENT, "> " .. text)
        Label.set_color(echo, COLOR_GREY)
        Layout.add_label(main_layout, echo)

        if text == "help" then
            show_help()
        elseif text == "launcher" then
            Launcher.start()
        else
            -- Execute
            local fn, err = load(text)
            if not fn then
                fn, err = load("return " .. text)
            end

            if not fn then
                local err_lbl = Label.create(0, 0, screen_w - 20, WRAP_CONTENT, "Compile Error: " .. tostring(err))
                Label.set_color(err_lbl, COLOR_RED)
                Label.set_multiline(err_lbl, true)
                Layout.add_label(main_layout, err_lbl)
            else
                local results = { pcall(fn) }
                local status = results[1]
                if not status then
                    local err_lbl = Label.create(0, 0, screen_w - 20, WRAP_CONTENT, "Runtime Error: " .. tostring(results[2]))
                    Label.set_color(err_lbl, COLOR_RED)
                    Label.set_multiline(err_lbl, true)
                    Layout.add_label(main_layout, err_lbl)
                else
                    for i = 2, #results do
                        print(results[i])
                    end
                end
            end
        end
        
        TextBox.set_text(tb, "")
    end)

    App.set_view(main_layout, 0)
end

function draw()
    Text.draw(20, Screen.height() - 30, "Layout Engine Active", COLOR_ORANGE)
end

init()
