-- This loads the C library and binds is to a 'ds' object
ds = require 'libds'
-- Don't change for now
gff_filename = "resource.gff"

function init_current_file()
    -- Returns how many different types of data are stored in the gff file (EX: image, text music, etc...)
    numTypes = ds.gff_get_number_of_types(gff_file)

    -- We start at the first entry
    current_type_index = 0

    -- Find what the ID of the first type.  We use this to reference which type of data we are working with.
    type_id = ds.gff_get_type_id(gff_file, current_type_index)

    -- Get a list of all the resource ids associated with a files' type
    current_resource_id_list = ds.gff_get_id_list(gff_file, type_id)

    -- Start at the first id (Note that the first id, may not be zero!)
    current_resource_id_index = 1

    -- Hold the Image (so we don't reload EVERY time)
    current_image = 0
end

function love.load()
  love.graphics.setDefaultFilter('nearest', 'nearest')
  
    if #arg < 2 then
        print ("Usage: <love2d binary> " .. arg[1] .. "<path to darksun 1 directory>")
        love.event.quit(0)
    end

    -- Always call first to initialize the gff system
    ds.gff_init()

    -- Then load the game directory, right now we are limited to 256 GFF files.
    print('Initialized, loading directory:' .. arg[2])
    ds.gff_load_directory(arg[2])

    -- Lets select the gff_filename from on top.
    gff_file = ds.gff_find_index(gff_filename)
    if (gff_file == -1) then -- Not a DS1 directory
        gff_file = ds.gff_find_index("resflop.gff")
    end
    print('Resource File selected: ' .. gff_file)

    -- intialize the current gff file
    init_current_file()

    -- Hold the current selected palette
    current_palette = 0

    -- Get the list of files in the directory.  Start with displaying 1.
    file_list = ds.gff_list()
    file_index = 1 -- Lua is 1 referenced =(

    music = love.audio.newSource("074.voc", "static")
    music:play()
end

function love.keypressed( key )
    if key == "escape" then
        ds.gff_close(gff_file)
        love.event.quit(0)
    end
end

function love.update(dt)
end

function love.draw()
dirwork = love.filesystem.getWorkingDirectory ()
    love.graphics.print("dir: " .. dirwork, 10, 10)
    love.graphics.print("up/down: change the type selected in the GFF. ", 10, 550)
end
