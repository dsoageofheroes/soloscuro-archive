-- This selects and runs the appropiate test from the command line argument
local testRunner = {}
local narrateTester = require 'test/narrateTester'
local charTester =    require 'test/charTester'
local testToRun = nil

testFunctions = {
    ["narr"] = narrateTester.run,
    ["char"] = charTester.run
}

function testRunner.init(args)
    num_args = 0
    for _ in pairs(args) do num_args = num_args + 1 end

    for i=3,num_args do
        parseArg(arg[i-1])
    end

    if not (testToRun == nil) then
        print ("Need to run test:" .. testToRun)
        testFunctions[testToRun]()
        os.exit(0)
    end
end

local nextArgIsDebug = false
function parseArg(arg)
    if nextArgIsDebug then
        testToRun = arg
        nextArgIsDebug = false
    end
    if arg == "-d" then
        nextArgIsDebug = true
    end
end

return testRunner
