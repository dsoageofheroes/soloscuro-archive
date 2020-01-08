-- This loads the C library and binds is to a 'ds' object
ds = require 'libds'

ADD_MENU = 0
PORTRAIT = 1
SHOW_TEXT = 2
SHOW_MENU = 3
EDIT_BOX = 4

-- When narrate_open is called it either creates an dialog box OR edits the current existing one.
-- It is VERY common for narrate_open to be called with PORTRAIT and no text, then have SHOW_TEXT later
-- with possibly a menu.
-- NOTE: SHOW_TEXT may be (and usually is) called multiple times on the same dialog.
function narrate_open(action, text, index)
    if (action == ADD_MENU) then
        -- Here you add a menu item (text).
        -- AKA: add an option for the player to select
    end
    if (action == PORTRAIT) then
        -- Here set the portrait by index.
        data, width, height = ds.get_portrait(index)
        if (data ~=  0) then -- always check that it got the image!
            imageData = love.image.newImageData(width, height, "rgba8", data)
            portrait_image = love.graphics.newImage( imageData )
        end
        -- Now portrait_image *should* be the image to display
        -- Also, if text is not empty, then text needs to be displayed as well.
    end
    if (text) then --(action == SHOW_TEXT) then
        -- Here display the message text to the user
        _text = text
    end
    if (action == SHOW_MENU) then
        -- Display the menu
    end
    print("I'm in LUA!");
end

function love.load()
  love.graphics.setDefaultFilter('nearest', 'nearest')
  
    -- Check that we are passed the directory to darksun
    if #arg < 2 then
        print ("Usage: <love2d binary> " .. arg[1] .. "<path to darksun 1 directory>")
        love.event.quit(0)
    end

    -- Always call first to initialize the gff system
    ds.gff_init()

    -- Then load the game directory, right now we are limited to 256 GFF files.
    print('Initialized, loading directory:' .. arg[2])
    ds.gff_load_directory(arg[2])

    gpl_file = ds.gff_find_index("gpldata.gff")

    -- Warning: This currently only works once!  (don't call again for now...)
    ds.mas_execute(gpl_file, 42) -- Change to the arena region

    -- When everythin is seetup the following functions are triggered by some event, but that
    -- isn't implemented right now.  Instead we trigger manually by calling
    -- dsl_execute.

    -- This one creates the first dialog and then exits (prematurely.)
    -- I need to come up with a scheme of how to continue the gpl/dsl script.
    -- So that once you response the conversation can continue.
    ds.dsl_execute(gpl_file, 5, 2348)
    -- Kinda work...
    -- ds.dsl_execute(gpl_file, 4, 1) -- I never got this in the real game, did you?
    --LOOK CHECKS
    --ds.dsl_execute(gpl_file, 5, 6792)
    --ds.dsl_execute(gpl_file, 5, 6832)

    -- The following don't work, so call at your own risk!
    --ds.dsl_execute(gpl_file, 2, 1)
    --ds.dsl_execute(gpl_file, 5, 54)
    --ds.dsl_execute(gpl_file, 5, 74)
    --ds.dsl_execute(gpl_file, 5, 94)
    --ds.dsl_execute(gpl_file, 5, 114)
    --ds.dsl_execute(gpl_file, 3, 1854)
    --ds.dsl_execute(gpl_file, 5, 931)
    --ds.dsl_execute(gpl_file, 5, 772)
    --ds.dsl_execute(gpl_file, 5, 6530)
end

function love.keypressed( key )
    if key == "escape" then
        --ds.gff_close(gff_file)
        love.event.quit(0)
    end
end

function love.update(dt)
end

function love.draw()
    if (portrait_image) then
        love.graphics.draw(portrait_image, 0, 0, 0, 3, 3.6)
    end

    if (_text) then
        love.graphics.print(_text, 50, 300)
    end
    love.graphics.print("Hello", 50, 320)
end
