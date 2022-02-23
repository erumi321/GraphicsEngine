--[[CreateRectangle(200, 200, 400, 400, 1.0, 1.0, 1.0);
CreateRectangle(200, 600, 400, 400, 1.0, 0.0, 0.0);
CreateRectangle(600, 200, 400, 400, 0.0, 1.0, 0.0);
CreateRectangle(600, 600, 400, 400, 0.0, 0.0, 1.0);]]--

--CreateRectangle(x, y, width, height, r, g, b) - returns id
--CreateButton(x, y, width, height, functionName, r, g, b) - returns id
--CreateText(text, x, y, scale, r, g, b) - returns id

--ModifyRectangle(id, x, y, width, height, r, g, b)
--ModifyButton(id, x, y, width, height, functionName, r, g, b)
--ModifyText(id, text, x, y, scale, r, g, b)

--[[all button functions are passed a button object which is a table with the values
{
	x = x position of button,
	y = y position of button,
	width = width of button,
	height = height of button,
	ID = id of button
}
]]--

CreateButton(200, 200, 200, 200, "modText", 0.0, 0.0, 1.0)

local text = CreateText("Hey", 200, 200, 1.0, 1.0, 1.0, 0.0)

function modText(btn)
	ModifyText(text, "Ho", 600, 600, 1.0, 1.0, 0.0, 1.0)
	ModifyButton(btn.ID, 600, 600, btn.width, btn.height, "modText2", 0.0, 0.0, 1.0)
end

function modText2(btn)
	ModifyText(text, "Hey", 200, 200, 1.0, 1.0, 1.0, 0.0)
	ModifyButton(btn.ID, 200, 200, btn.width, btn.height, "modText", 0.0, 0.0, 1.0)
end