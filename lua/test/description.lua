pos = 0
level = 1

function idle()
    soloscuro.debug_set_desc(0, level, pos)
end

function keydown(num)
    --print (num)
    soloscuro.debug_set_desc(0, level, pos)
    if (num == DSLK_UP) then
        level = level + 1
    end
    if (num == DSLK_DOWN) then
        level = level - 1
    end
    if (num == DSLK_LEFT) then
        pos = pos - 1
    end
    if (num == DSLK_RIGHT) then
        pos = pos + 1
    end
end

function keyup()
end

function init()
    soloscuro.load_charsave(0, 3) -- load character from charsave.gff
    --soloscuro.load_charsave(1, 16) -- load character from charsave.gff
    --soloscuro.set_ignore_repeat(true) -- set to false if you want repeats
    --e = soloscuro.load_player(0)
    --print (e.name)
    soloscuro.load_window("description")
end
