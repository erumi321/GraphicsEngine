--[[CreateRectangle(200, 200, 400, 400, 1.0, 1.0, 1.0);
CreateRectangle(200, 600, 400, 400, 1.0, 0.0, 0.0);
CreateRectangle(600, 200, 400, 400, 0.0, 1.0, 0.0);
CreateRectangle(600, 600, 400, 400, 0.0, 0.0, 1.0);]]--

CreateButton(600, 600, 400, 400, "testFunction", 0.0, 0.0, 1.0);
CreateButton(200, 200, 400, 400, "testFunction2", 0.0, 0.0, 1.0);

local i = 0

function testFunction(btn)
	MyPrint(btn.x);
end

function testFunction2(btn)
	MyPrint(btn.y);
end
