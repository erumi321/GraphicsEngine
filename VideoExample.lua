--[[CreateRectangle(200, 200, 400, 400, 1.0, 1.0, 1.0);
CreateRectangle(200, 600, 400, 400, 1.0, 0.0, 0.0);
CreateRectangle(600, 200, 400, 400, 0.0, 1.0, 0.0);
CreateRectangle(600, 600, 400, 400, 0.0, 0.0, 1.0);]]--

--CreateRectangle(x, y, width, height, r, g, b)
--CreateButton(x, y, width, height, functionname, r, g, b)
--CreateText(text, x, y, scale, r, g, b)

CreateButton(200, 200, 400, 400, "createText", 1.0, 1.0, 0.0);
CreateText("New Button", 200, 200, 1.0, 1.0, 1.0, 1.0)

function createText(btn)
	CreateText("This was created!", 600, 600, 1.0, 0.0, 0.0)
end