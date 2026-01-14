function init()
    screen_w = Screen.width()
    screen_h = Screen.height()
    
    main_layout = Layout.create(LAYOUT_LINEAR, 0, 0, screen_w, screen_h, 0)
    
    local lbl = Label.create(0, 0, screen_w - 20, WRAP_CONTENT, "This is the second app!")
    Label.set_multiline(lbl, true)
    Label.set_color(lbl, Color.from_rgb(255, 255, 0))
    Layout.add_label(main_layout, lbl)
    
    local btn_back = Button.create(0, 0, 200, 50, "Go Back", Color.from_rgb(100, 100, 100))
    Button.on_click(btn_back, function()
        Launcher.start()
    end)
    Layout.add_button(main_layout, btn_back)

    App.set_view(main_layout, 0)
end

init()
