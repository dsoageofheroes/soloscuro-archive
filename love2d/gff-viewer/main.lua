-- This loads the C library and binds is to a 'ds' object
--require 'dsl/objects'
ds = require 'libds'
-- Don't change for now
gff_filename = "resource.gff"

RMAP_TYPE = 1346456914
GMAP_TYPE = 1346456903
TILE_TYPE = 1162627412
ETAB_TYPE = 1111577669
RDFF_TYPE = 1179010130
BMP_TYPE  = 542133570
MONR_TYPE = 1380863821
FONT_TYPE = 1414418246
MAS_TYPE =  542327117
GPL_TYPE =  541872199
PORT_TYPE = 1414680400

function narrate_open(action, text, index)
end

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

    -- Hold the Music data and handle to the music player
    midi_data = 0
    music = 0

    -- Grab the RMAP data, if it exists.
    ds.load_map(gff_file);
    rmap_id_list = ds.gff_get_id_list(gff_file, RMAP_TYPE);
    if (table.getn(rmap_id_list) > 0) then
        res_id = rmap_id_list[current_resource_id_index] -- ASSUME only one RMAP!
    end

    -- Grab ALL the tiles in case we are a region file.
    tiles = {}
    get_tiles()

    -- Set the start location for the RMAP
    rmapstartx = 0
    rmapstarty = 0

    -- Set the number of possible objects
    num_objects = 0
    cobject = 0

    -- Zero out the font
    font = nil

    -- Current animation script
    scmd = nil
    -- Each script is really a "script file" with 36 internal scripts
    -- scmd_number will depeict which of the 36 are in use
    -- 0 is the default script of the script entry/file.
    scmd_number = 25
    scmd_flipx = 1 -- If we need to flip along x
    scmd_flipy = 1 -- if we need to flip along y
    scmd_xoffset = 0
    scmd_yoffset = 0 -- any offsets needed by script.

    --f = DS1Combat()
    --print ("f.id = " .. f.id)
    --print ("f.hp = " .. f.hp)
    --f.hp = 12
    --gpl_file = ds.gff_find_index("gpldata.gff")
    --ds.mas_execute(gpl_file, 42)
    --ds.gpl_execute(gpl_file, 3, 1716)
    --ds.gpl_execute(gpl_file, 4, 1) -- I never got this in the real game, did you?
    --ds.gpl_execute(gpl_file, 5, 2348)
    --LOOK CHECKS
    --ds.gpl_execute(gpl_file, 5, 6792)
    --ds.gpl_execute(gpl_file, 5, 6832)
    --NO WORKY
    --ds.gpl_execute(gpl_file, 2, 1)
    --ds.gpl_execute(gpl_file, 5, 54)
    --ds.gpl_execute(gpl_file, 5, 74)
    --ds.gpl_execute(gpl_file, 5, 94)
    --ds.gpl_execute(gpl_file, 5, 114)
    --ds.gpl_execute(gpl_file, 3, 1854)
    --ds.gpl_execute(gpl_file, 5, 931)
    --ds.gpl_execute(gpl_file, 5, 772)
    --ds.gpl_execute(gpl_file, 5, 6530)
end

function get_font() 
    yellow_color = 0xFFFF00FF -- R, G, B, A
    black_color = 0x000000FF -- R, G, B, A
    if (font == nil) then
        font = {}
        font_count = ds.font_count(gff_file)
        for i=0,255 do
            font[i] = {}
            img, w, h = ds.create_font_img(gff_file, i, yellow_color, black_color);
            current_image = nil
            if (not (w == 0)) then
                imageData = love.image.newImageData(w, h, "rgba8", img)
                current_image = love.graphics.newImage( imageData )
            end
            font[i]["img"] = current_image;
            font[i]["w"] = w;
            font[i]["h"] = h;
        end
    end
end

function get_tiles() 
    tile_id_list = ds.gff_get_id_list(gff_file, TILE_TYPE);
    for k,tile_id in pairs(tile_id_list) do
        data = ds.get_frame_rgba_with_palette(gff_file, TILE_TYPE, tile_id, 0, current_palette)
        if (data ~=  0) then -- always check that it got the image!
            width = ds.get_frame_width(gff_file, TILE_TYPE, tile_id, 0)
            height = ds.get_frame_height(gff_file, TILE_TYPE, tile_id, 0)
            imageData = love.image.newImageData(width, height, "rgba8", data)
            tiles[tile_id] = love.graphics.newImage( imageData )
        end
    end
end

-- No precondition, draws with blocks (aka, don't move there!)
function draw_rmap_with_blocks()
    rmapx = 0   -- where on the screen to draw the first tile, x position
    rmapy = 130 -- where on the screen to draw the first tile, y positione
    if (table.getn(tiles) > 0) then -- make sure we have tiles!
        for i=rmapstartx,127 do
            for j=rmapstarty,97 do
                nextx = rmapx + 16*(i-rmapstartx) -- tiles are 16x16
                nexty = rmapy + 16*(j-rmapstarty) -- tiles are 16x16
                tile_id = ds.get_tile_id(gff_file, j, i);
                love.graphics.draw(tiles[tile_id], nextx, nexty, 0, 2, 2.4)
                -- Blocks in DS1
                if (ds.map_is_block(gff_file, j, i)) then
                    love.graphics.setColor(255,0,0);
                    love.graphics.rectangle("line", nextx, nexty, 16, 16 )
                    love.graphics.setColor(255,255,255);
                end
                -- I believe these come when items are in the map.
                if (ds.map_is_actor(gff_file, j, i)) then
                    love.graphics.setColor(0,255,0);
                    love.graphics.rectangle("line", nextx + 1, nexty + 1, 14, 14 )
                    love.graphics.setColor(255,255,255);
                end
                -- I believe these are DSO items, but I could be wrong...
                if (ds.map_is_danger(gff_file, j, i)) then
                    love.graphics.setColor(0,0,255);
                    love.graphics.rectangle("line", nextx + 2, nexty + 2, 12, 12 )
                    love.graphics.setColor(255,255,255);
                end
            end
        end
    end
end

-- No precondition required.
function draw_rmap_simplier()
    rmapx = 0   -- where on the screen to draw the first tile, x position
    rmapy = 130 -- where on the screen to draw the first tile, y positione
    if (table.getn(tiles) > 0) then -- make sure we have tiles!
        for i=rmapstartx,127 do
            for j=rmapstarty,97 do
                nextx = rmapx + 16*(i-rmapstartx) -- tiles are 16x16
                nexty = rmapy + 16*(j-rmapstarty) -- tiles are 16x16
                tile_id = ds.get_tile_id(gff_file, j, i);
                love.graphics.draw(tiles[tile_id], nextx, nexty, 0, 2, 2.4)
            end
        end
    end
end

-- PRECONDITION: rmap_id_list has the ids of the rmap!
function draw_rmap()
    rmapx = 0
    rmapy = 130
    if (table.getn(rmap_id_list) > 0) then
        res_id = rmap_id_list[current_resource_id_index] -- ASSUME only one RMAP!
        rmap = ds.gff_get_data_as_text(gff_file, RMAP_TYPE, res_id)
        for i=rmapstartx,126 do
            for j=rmapstarty,96 do -- This may need to be 97!
                nextx = rmapx + 16*(i-rmapstartx) -- tiles are 16x16
                nexty = rmapy + 16*(j-rmapstarty) -- tiles are 16x16
                tile_id = string.byte(rmap, i+j*128+1)
                love.graphics.draw(tiles[tile_id], nextx, nexty, 0, 2, 2.4)
            end
        end
    end
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
end

function love.keypressed( key )
    if key == "down" then
        if (not (music == 0)) then
            music:stop()
        end
        cframe = 0
        current_image = 0
        current_resource_id_index = 1
        current_type_index = (current_type_index + 1) % numTypes
        type_id = ds.gff_get_type_id(gff_file, current_type_index)
        current_resource_id_list = ds.gff_get_id_list(gff_file, type_id)
        midi_data = 0
        scmd = nil
        scmd_xoffset = 0
        scmd_yoffset = 0
    end
    if key == "up" then
        if (not (music == 0)) then
            music:stop()
        end
        cframe = 0
        current_image = 0
        current_resource_id_index = 1
        current_type_index = (current_type_index - 1) % numTypes
        type_id = ds.gff_get_type_id(gff_file, current_type_index)
        current_resource_id_list = ds.gff_get_id_list(gff_file, type_id)
        midi_data = 0
        scmd = nil
        scmd_xoffset = 0
        scmd_yoffset = 0
    end
    if key == "right" then
        if (not (music == 0)) then
            music:stop()
        end
        cframe = 0
        current_image = 0
        -- 1 referenced arrays are so dumb...
        if (current_resource_id_index == table.getn(current_resource_id_list)) then
            current_resource_id_index = 1
        else
            current_resource_id_index = current_resource_id_index + 1
        end
        midi_data = 0
        -- with 0 referenced it would be: current_resource_id_index = (current_resource_id_index + 1) % table.getn(current_resource_id_list)
        cobject = cobject + 1
        cobject = cobject % num_objects
        scmd = nil
        scmd_xoffset = 0
        scmd_yoffset = 0
    end
    if key == "left" then
        if (not (music == 0)) then
            music:stop()
        end
        cframe = 0
        current_image = 0
        if (current_resource_id_index == 1) then
            current_resource_id_index = table.getn(current_resource_id_list)
        else
            current_resource_id_index = current_resource_id_index - 1
        end
        midi_data = 0
        cobject = cobject - 1
        cobject = cobject % num_objects
        scmd = nil
        scmd_xoffset = 0
        scmd_yoffset = 0
    end
    if key == "f" then
        cframe = cframe + 1
        current_image = 0
    end
    if key == "p" or key == "o" then
        num_palettes = ds.gff_get_number_of_palettes()
        if key == "p" then
            current_palette = (current_palette + 1) % num_palettes
        else
            current_palette = (current_palette - 1 + num_palettes) % num_palettes
        end
        current_image = 0
        tiles = {}
        get_tiles()
        if (not (midi_data == 0)) then
            if (not (music == 0)) then
                music:stop()
            end
            music_data = love.filesystem.newFileData(midi_data, "_.mid")
            music_decoder = love.sound.newDecoder(music_data)
            music = love.audio.newSource(music_decoder, "stream")
            music:play()
        end
    end
    if key == "w" then
        ds.gff_write_raw_bytes(gff_file, type_id, current_resource_id_list[current_resource_id_index], "data.dat")
    end
    if key == "escape" then
        ds.gff_close(gff_file)
        love.event.quit(0)
    end
    if key == "pageup" then
        file_index = (file_index - 1) % (table.getn(file_list) + 1)
        if (file_index == 0) then file_index = table.getn(file_list) end
        gff_file = ds.gff_find_index(file_list[file_index]) -- gff_file index *MAY* or *MAYNOT* be = file_index
        init_current_file()
        current_resource_id_index = 1
        current_type_index = 0
    end
    if key == "pagedown" then
        file_index = (file_index + 1) % table.getn(file_list)
        if (file_index == 0) then file_index = table.getn(file_list) end
        gff_file = ds.gff_find_index(file_list[file_index]) -- gff_file index *MAY* or *MAYNOT* be = file_index
        init_current_file()
        current_resource_id_index = 1
        current_type_index = 0
    end
    -- controls for the map!
    if key == "e" then
        rmapstarty = rmapstarty - 1
        if (rmapstarty < 0) then rmapstarty = 0 end
    end
    if key == "d" then
        rmapstarty = rmapstarty + 1
        if (rmapstarty < 0) then rmapstarty = 0 end
    end
    if key == "s" then
        rmapstartx = rmapstartx - 1
        if (rmapstartx < 0) then rmapstartx = 0 end
    end
    if key == "f" then
        rmapstartx = rmapstartx + 1
        if (rmapstartx < 0) then rmapstartx = 0 end
    end
    if key == "a" then
        get_next_scmd() 
    end
end

-- Searches for the next SCMD and set it to execute.
-- If none is found, default to scmd 0.
function get_next_scmd() 
    scmd_number = scmd_number + 1
    if (scmd_number > 35) then
        scmd_number = 0
        scmd = nil
        return
    end
end

function love.update(dt)
    if (not(scmd == nil)) then
        --print("scmd_len = " .. scmd_len)
        if (scmd_len > 0) then
            scmd_delay = scmd_delay - (60 * dt)
            --print("scmd_delay = " .. scmd_delay)
            --print("scmd_idx = " .. scmd_idx)
            if (scmd_delay < 0) then
                --print("frame_idx = " .. ds.scmd_bmp_idx(scmd, scmd_idx))
                cframe = ds.scmd_bmp_idx(scmd, scmd_idx);
                current_image = 0
                scmd_delay = ds.scmd_delay(scmd, scmd_idx)
                if (ds.scmd_last(scmd, scmd_idx)) then
                    scmd = nil
                else
                    if (scmd_delay == 0) then
                        scmd_delay = 16 -- default delay if none is specified in the script.
                    end
                    if (ds.scmd_xmirror(scmd, scmd_idx)) then scmd_flipx = -1 else scmd_flipx = 1 end
                    if (ds.scmd_ymirror(scmd, scmd_idx)) then scmd_flipy = -1 else scmd_flipy = 1 end
                    scmd_xoffset = scmd_xoffset + ds.scmd_xoffset(scmd, scmd_idx)
                    scmd_yoffset = scmd_yoffset + ds.scmd_yoffset(scmd, scmd_idx)
                    scmd_idx = scmd_idx + 1
                end
            end
        end
    end
end

function love.draw()
    res_id = current_resource_id_list[current_resource_id_index] -- just save what our current resource id is.
    -- Get the id of the current type selected.
    type_id = ds.gff_get_type_id(gff_file, current_type_index)
    -- Get the name of the current type id.
    type_name = ds.get_gff_type_name(type_id);
    love.graphics.print("File: " .. file_list[file_index], 10, 10)
    love.graphics.print("Type: " .. current_type_index .. " of " .. (numTypes - 1), 10, 30)
    love.graphics.print("Name: " .. type_name .. " (".. type_id .. ")", 10, 50)
    if (res_id == nil) then
        return;
    end
    love.graphics.print("Current ID: " .. res_id , 10, 70)
    love.graphics.print("Data: ", 10, 90)
    love.graphics.print("Palette index: " .. current_palette .. " press o/p to cycle", 410, 30)

    type_displayed = false; -- Lua doesn't have a switch statement =(
    if (type_id == 1313427539 or type_id == 1415071060) then
        -- With text we can grab the text and print
        msg = ds.gff_get_data_as_text(gff_file, type_id, res_id)
        love.graphics.print(msg, 10, 110)
        type_displayed = true;
    end
    if (type_id == BMP_TYPE or type_id == 1313817417 or type_id == 1162627412) then
        num_frames = ds.get_frame_count(gff_file, type_id, res_id)
        cframe = cframe % num_frames
        width = ds.get_frame_width(gff_file, type_id, res_id, cframe)
        height = ds.get_frame_height(gff_file, type_id, res_id, cframe)
        love.graphics.print("Number of Frames: " .. num_frames, 10, 110)
        love.graphics.print("Frame [" .. cframe .. "]: " .. width .. " x " .. height, 10, 130)
        love.graphics.print("Press 'f' to cycle through frames if there are more than one.", 10, 150)
        if (current_image == 0) then
            data = ds.get_frame_rgba_with_palette(gff_file, type_id, res_id, cframe, current_palette)
            if (data ~=  0) then -- always check that it got the image!
                imageData = love.image.newImageData(width, height, "rgba8", data)
                current_image = love.graphics.newImage( imageData )
            end
        end
        if (current_image ~= 0) then -- always check the image is available!
            love.graphics.draw(current_image, 0, 170, 0, 2, 2.4)
        else
            love.graphics.print("Unable to convert image.  Inform the developer.", 10, 150)
        end
        type_displayed = true;
    end
    if (type_id == 1363497804 or type_id == 1363497799) then
        if (midi_data == 0) then
            midi_data = ds.get_chunk_as_midi(gff_file, type_id, res_id)
        end
        love.graphics.print("Press p to play music.", 10, 170)
    end
    if (type_id == RMAP_TYPE) then
        love.graphics.print("Use e, s, d, f to scroll by tile.", 10, 110)
        draw_rmap_simplier()
        type_displayed = true
    end
    if (type_id == GMAP_TYPE) then
        love.graphics.print("Use e, s, d, f to scroll by tile.", 10, 110)
        draw_rmap_with_blocks()
        type_displayed = true
    end
    if (type_id == ETAB_TYPE) then
        num_objects = ds.map_get_number_of_objects(gff_file, res_id)
        num_frames = ds.map_get_object_frame_count(gff_file, res_id, cobject)
        cframe = cframe % num_frames
        love.graphics.print("current object # " .. cobject .. " of " .. num_objects, 10, 110)
        love.graphics.print("use right/left arrows to cycle through", 10, 130)
        love.graphics.print("Number of Frames: " .. num_frames, 10, 150)
        love.graphics.print("Frame [" .. cframe .. "]: " .. width .. " x " .. height .. " Animation #" .. scmd_number, 10, 170)
        love.graphics.print("Press 'f' to cycle through frames if there are more than one.  Press 'a' to cycle through animations", 10, 190)
        if (current_image == 0) then
            data, width, height = ds.map_get_object_bmp(gff_file, res_id, cobject, cframe)
            if (data ~=  0) then -- always check that it got the image!
                imageData = love.image.newImageData(width, height, "rgba8", data)
                current_image = love.graphics.newImage( imageData )
            end
        end
        if (current_image ~= 0) then -- always check the image is available!
            love.graphics.draw(current_image, 50 + scmd_xoffset, 230 + scmd_yoffset, 0, scmd_flipx * 2, scmd_flipy * 2.4)
        else
            love.graphics.print("Unable to convert image.  Inform the developer.", 10, 230)
        end
        x, y, z = ds.map_get_object_location(gff_file, res_id, cobject)
        love.graphics.print("location (" .. x .. ", " .. y .. ", " .. z ..")", 10, 210)
        type_displayed = true;
        if (scmd == nil) then
            scmd_xoffset = 0
            scmd_yoffset = 0
            scmd = ds.map_get_object_scmd(gff_file, res_id, cobject, scmd_number)
            if (not (scmd == 0))  then
                scmd_len = ds.scmd_len(scmd)
                if (scmd_len > 0) then
                    scmd_idx = 0
                    scmd_delay = ds.scmd_delay(scmd, scmd_idx)
                    if (scmd_delay == 0) then scmd_delay = 16 end
                    if (ds.scmd_xmirror(scmd, scmd_idx)) then scmd_flipx = -1 else scmd_flipx = 1 end
                    if (ds.scmd_ymirror(scmd, scmd_idx)) then scmd_flipy = -1 else scmd_flipy = 1 end
                end
            else
                scmd = nil
            end
        end
    end
    if (type_id == RDFF_TYPE) then
        object = ds.object_inspect(gff_file, res_id)
        if (object == nil) then return end
        if (object["type"] == 1) then
            print_ds1_combat(object)
            type_displayed = true;
        end
        if (object["type"] == 2) then
            print_ds1_item(object)
            type_displayed = true;
        end
    end
    if (type_id == MONR_TYPE) then
        monster = ds.load_monster(2, 2)
        if (monster == nil) then return end
        love.graphics.print("id = " .. monster["id"] .. ", level = " .. monster["level"], 10, 150)
    end
    if (type_id == FONT_TYPE) then
        get_font() 
        xpos = 0;
        ypos = 130;
        for i=0,255 do
            if (not (font[i]["img"] == nil)) then
                love.graphics.draw(font[i]["img"], xpos, ypos, 0, 2, 2.4)
                type_displayed = true
            end
            xpos = xpos + 20
            if (xpos > 320) then
                xpos = 0;
                ypos = ypos + 25
            end
        end
    end
    --if (type_id == MAS_TYPE) then
        --ds.mas_print(gff_file, res_id)
    --end
    --if (type_id == GPL_TYPE) then
        --ds.gpl_print(gff_file, res_id)
    --end
    if (type_id == PORT_TYPE) then
        if (current_image == 0) then
            --data = ds.get_frame_rgba_with_palette(gff_file, type_id, res_id, cframe, current_palette)
            data, width, height = ds.get_portrait(res_id)
            if (data ~=  0) then -- always check that it got the image!
                imageData = love.image.newImageData(width, height, "rgba8", data)
                current_image = love.graphics.newImage( imageData )
            end
        end
        if (current_image ~= 0) then -- always check the image is available!
            love.graphics.draw(current_image, 0, 170, 0, 2, 2.4)
        else
            love.graphics.print("Unable to convert image.  Inform the developer.", 10, 150)
        end
        type_displayed = true;
    end
    if (not type_displayed) then
        love.graphics.print("Unable to display.", 10, 110)
    end
    love.graphics.print("up/down: change the type selected in the GFF. ", 10, 550)
    love.graphics.print("left/right: change the current selected data in the GFF. ", 10, 570)
end

function print_ds1_combat(object)
    love.graphics.print("DarkSun1 Combat Object: " .. object["name"], 10, 110)
    love.graphics.print("HP: " .. object["hp"], 10, 130)
    love.graphics.print("Psi: " .. object["psi"], 10, 150)
    love.graphics.print("game id: " .. object["id"], 10, 170)
    love.graphics.print("AC: " .. object["ac"], 10, 190)
    love.graphics.print("Movement: " .. object["move"], 10, 210)
    love.graphics.print("THAC0: " .. object["thac0"], 10, 230)
    love.graphics.print("str: " .. object["str"], 10, 250)
    love.graphics.print("dex: " .. object["dex"], 10, 270)
    love.graphics.print("con: " .. object["con"], 10, 290)
    love.graphics.print("int: " .. object["int"], 10, 310)
    love.graphics.print("wis: " .. object["wis"], 10, 330)
    love.graphics.print("cha: " .. object["cha"], 10, 350)
    --if (current_image ~= 0) then -- always check the image is available!
        --love.graphics.draw(current_image, 50, 130, 0, 2, 2.4)
    --end
end

function print_ds1_item(object)
    love.graphics.print("DarkSun1 item Object: " .. object["name_index"], 10, 110)
    love.graphics.print("id: " .. object["id"], 10, 130)
    love.graphics.print("quantity: " .. object["quantity"], 10, 150)
    love.graphics.print("value: " .. object["value"], 10, 170)
    love.graphics.print("icon: " .. object["icon"], 10, 190)
    love.graphics.print("charges: " .. object["charges"], 10, 210)
    love.graphics.print("special: " .. object["special"], 10, 230)
    love.graphics.print("slot: " .. object["slot"], 10, 250)
    love.graphics.print("bmp_id: " .. object["bmp_id"], 10, 270)
    love.graphics.print("script_id: " .. object["script_id"], 10, 290)
    --if (current_image ~= 0) then -- always check the image is available!
        --love.graphics.draw(current_image, 50, 130, 0, 2, 2.4)
    --end
end
