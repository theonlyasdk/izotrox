function init()
    screen_w = Screen.width()
    screen_h = Screen.height()
    
    main_layout = Layout.create(LAYOUT_LINEAR, 0, 0, screen_w, screen_h, 0)
    
    local lbl = Label.create(0, 0, screen_w - 20, WRAP_CONTENT, "Hello from Mini App!")
    Label.set_multiline(lbl, true)
    Label.set_color(lbl, Color.from_rgb(0, 255, 0))
    Layout.add_label(main_layout, lbl)
    
    local btn_back = Button.create(0, 0, 200, 50, "Back to Launcher", Color.from_rgb(100, 100, 100))
    -- Assuming Launcher.start() will be implemented
    Button.on_click(btn_back, function()
        Launcher.start()
    end)
    -- Wait, I don't have Button.on_click yet? 
    -- I'll check Button bindings.
    
    Layout.add_button(main_layout, btn_back)

    App.set_view(main_layout, 0)
end

init()
