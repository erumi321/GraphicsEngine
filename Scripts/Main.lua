--MyPrint(text)

--CreateRectangle(x, y, width, height, r, g, b) - returns id
--CreateButton(x, y, width, height, functionName, r, g, b) - returns id
--CreateText(text, x, y, scale, r, g, b) - returns id

--ModifyRectangle(id, x, y, width, height, r, g, b)
--ModifyButton(id, x, y, width, height, functionName, r, g, b)
--ModifyText(id, text, x, y, scale, r, g, b)

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

	CreateButton(args.X, args.Y, args.Width, args.Height, args.onClick, args.R , args.G, args.B, args.Args)
end

function modifyButton(id, args)
	ModifyButton(id, args.X, args.Y, args.Width, args.Height, args.onClick, args.R, args.G, args.B, args.Args or {})
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
	Args = {}
}

function table_length(t)
    local z = 0
    for i,v in pairs(t) do z = z + 1 end
    return z
end

table_length(btn1Args.Args)


createButton(btn1Args)


local options = {
	"1",
	"2",
	"3"
}

function OpenDropdown(btn)
	local newBtnArgs = btn;

	newBtnArgs.onClick = "CloseDropdown"
	newBtnArgs.Args.DropdownObject = {
	}

	local x = 100;
	local y = 73;
	local height = 40;
	local width = 200;
	local padding = 3;

	for k,v in pairs(options) do
		local rect = CreateRectangle(x, y, width, height, 0.2, 0.2, 0.2)
		y = y + height + padding
		newBtnArgs.Args.DropdownObject[k] = rect
		MyPrint(k .. ": " .. rect)
	end

	modifyButton(btn.ID, newBtnArgs)

end

function CloseDropdown(btn)
	for k,v in pairs(btn.Args.DropdownObject) do
		DeleteRectangle(v)
	end

	local newBtnArgs = btn;

	newBtnArgs.onClick = "OpenDropdown"
	newBtnArgs.Args = {}

	modifyButton(btn.ID, newBtnArgs)
end