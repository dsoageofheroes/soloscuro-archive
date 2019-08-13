luaunit = require 'luaunit'
ds = require 'libds'

-- Always call first to initialize the gff system
ds.gff_init()

-- It is recommended to load all the DS1 gffs
ds.gff_load_directory("ds1")

function test1()
    luaunit.assertEquals(1 + 1, 2)
end

ds.gff_cleanup()

os.exit( luaunit.LuaUnit.run() )
