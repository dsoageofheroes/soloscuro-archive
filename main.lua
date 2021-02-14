function init()
    ds.load_charsave(0, 9) -- load character from charsave.gff
    ds.load_charsave(1, 10) -- load character from charsave.gff

    ds.load_region(42) -- right now only 42 works (there is some temporary hard-coding.)
    return true;
end
