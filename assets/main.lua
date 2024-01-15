
clickHandlers = {}

function onStartClicked()
    PlaySound("mixkit-select-click-1109.wav")
    destroyStartScreen()
end


function addClickHandler(rect, func)
    reg = { rect=rect, func=func}

    id = #clickHandlers + 1
    clickHandlers[id] = reg
    return id
end

function removeClickHandler(id)
    table.remove(clickHandlers, id)
end

function clearClickHandlers()
    clickHandlers = {}
end

function createStartScreen()
    local s = {
        rect = {{0, 0}, {300, 200}},
        backgroundColor = { 255, 255, 204, 255},
        color = { 0, 255, 0, 255 },
        text = "HELLO WORLD"
    }

    local b = {
        rect = {{400, 300}, {700, 400}},
        backgroundColor = { 255, 0, 0, 255},
        text = "Click to start"
    }
    addClickHandler(b.rect, onStartClicked)

    surfs = {s,b}
    UpdateSurfaces(surfs)
end

function destroyStartScreen()
    
    UpdateSurfaces({})
    clearClickHandlers()

end


function init()
    createStartScreen()
end

function handleClick(mouseX, mouseY)
    for _, reg in ipairs(clickHandlers) do
        if mouseX > reg.rect[1][1] and mouseX < reg.rect[2][1] and mouseY > reg.rect[1][2] and mouseY < reg.rect[2][2] then
            reg.func()
        end
    end
    
    -- if (mouseX > 400 and mouseX < 700 and mouseY > 300 and mouseY < 400) then
    --     onStartClicked()
    -- end

end

function update()

    if (inputState.ButtonConfirmDown) then
        handleClick(inputState.MouseX, inputState.MouseY)
    end

end
