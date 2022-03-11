--MyPrint(text)

--CreateRectangle(x, y, width, height, r, g, b) - returns id
--CreateButton(x, y, width, height, functionName, r, g, b) - returns id
--CreateText(text, x, y, width, scale, r, g, b) - returns id
--CreateTextfield(text, x, y, width, height, scale, r, g, b)

--ModifyRectangle(id, x, y, width, height, r, g, b)
--ModifyButton(id, x, y, width, height, functionName, r, g, b)
--ModifyText(id, text, x, y, width, text_scale, r, g, b)
--ModifyTextfield(id, text, x, y, width, height, text_scale, backR, backG, backB, textR, textG, textB)

--DeleteRectangle(id)
--DeleteButton(id)
--DeleteText(id)

--[[all button functions are passed a button object which is a table with the values
{
	X = x position of button,
	Y = y position of button,
	Width = width of button,
	Height = height of button,
	R = r color,
	G = g color,
	B = b color,
	onClick = onClick field
	ID = id of button,
	Args = All arrgs of button when passed in or modified, returned as table
}
]]--

function createButton(args)
	return CreateButton(args.X, args.Y, args.Width, args.Height, args.onClick, args.R , args.G, args.B, args.Args)
end

function modifyButton(id, args)
	for k,v in pairs(args.Args) do
		MyPrint(k)
	end
	ModifyButton(id, args.X, args.Y, args.Width, args.Height, args.onClick, args.R, args.G, args.B, args.Args)
end

function createText(args)
	local fontSize = args.FontSize / 48
	return CreateText(args.Text, args.X, args.Y, args.Width, fontSize, args.R, args.G, args.B)
end

function createTextfield(args)
	local fontSize = args.FontSize / 48
	return CreateTextfield(args.Text, args.X, args.Y, args.Width, args.Height, fontSize, args.BackingColor.R, args.BackingColor.G, args.BackingColor.B, args.TextColor.R, args.TextColor.G, args.TextColor.B)
end

local btn1Args = {
	X = 200,
	Y = 25,
	Width = 400,
	Height = 50,
	onClick = "OpenDropdown",
	R = 0.4,
	G = 0.4,
	B = 0.4,
	Args = {"hehe"}
}

local btn = createButton(btn1Args)


MyPrint(btn)

local options = {
	{Text = "New"},
	{Text = "Open"},
	{Text = "Close"},
	{Text = "Save"},
}

function OpenDropdown(btn)

	local newBtnArgs = btn;

	newBtnArgs.onClick = "CloseDropdown"
	newBtnArgs.Args.DropdownObjects = {}

	local x = 100;
	local y = 73;
	local height = 40;
	local width = 200;
	local padding = 3;

	for k,v in pairs(options) do
		local rect = CreateRectangle(x, y, width, height, 0.2, 0.2, 0.2)
		local displayText = createText({Text = v.Text, X = x - width / 2, Y = y - height / 2, Width = width, FontSize = 24, R = 1, G = 1, B = 1})

		newBtnArgs.Args.DropdownObjects[k] = {Id = rect, ChildText = {}}

		newBtnArgs.Args.DropdownObjects[k].ChildText = displayText

		y = y + height + padding

	end

	modifyButton(btn.ID, newBtnArgs)

end

function CloseDropdown(btn)
	
	for k,v in pairs(btn.Args.DropdownObjects) do
		DeleteRectangle(v.Id)
		DeleteText(v.ChildText)
	end

	local newBtnArgs = btn;

	newBtnArgs.onClick = "OpenDropdown"
	newBtnArgs.Args.DropdownObjects = {}

	modifyButton(btn.ID, newBtnArgs)
end