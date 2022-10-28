local spngt = require "spngt"

--This script is used to find faster-than-default encode settings

---@class spngt_experiment
---@field filename string
---@field params spngt_params
---@field results spngt_result
---@field size_increase number
---@field speedup number
local spngt_experiment = {}

---@class spngt_stats
---@field params spngt_params
---@field speedup_best number
---@field speedup_worst number
---@field speedup_avg number
---@field size_increase_best number
---@field size_increase_worst number
---@field size_increase_avg number
local spngt_stats = {}

local zlib_strategies =
{
    spngt.Z_FILTERED,
    spngt.Z_DEFAULT_STRATEGY,
    spngt.Z_HUFFMAN_ONLY,
    spngt.Z_RLE,
    spngt.Z_FIXED
}

local zlib_strategy_name =
{
    [spngt.Z_DEFAULT_STRATEGY] = "Z_DEFAULT_STRATEGY",
    [spngt.Z_FILTERED] = "Z_FILTERED",
    [spngt.Z_HUFFMAN_ONLY] = "Z_HUFFMAN_ONLY",
    [spngt.Z_RLE] = "Z_RLE",
    [spngt.Z_FIXED] = "Z_FIXED",
}

local filter_choices =
{
    spngt.FILTER_CHOICE_ALL,
    spngt.DISABLE_FILTERING,
    --[[spngt.FILTER_CHOICE_NONE,
    spngt.FILTER_CHOICE_SUB,
    spngt.FILTER_CHOICE_UP,
    spngt.FILTER_CHOICE_AVG,
    spngt.FILTER_CHOICE_PAETH]]--
--Testing all filter combinations is overkill
}

---@param filter_choice integer
---@return string
local function filter_choice_str(filter_choice)
    local str = ""

    if filter_choice == spngt.FILTER_CHOICE_ALL then return "ALL" end
    if filter_choice == spngt.DISABLE_FILTERING then return "DISABLED" end

    if filter_choice & spngt.FILTER_CHOICE_NONE ~= 0 then str = str .. "NONE " end
    if filter_choice & spngt.FILTER_CHOICE_SUB ~= 0 then str = str .. "SUB " end
    if filter_choice & spngt.FILTER_CHOICE_UP ~= 0 then str = str .. "UP " end
    if filter_choice & spngt.FILTER_CHOICE_AVG ~= 0 then str = str .. "AVG " end
    if filter_choice & spngt.FILTER_CHOICE_PAETH ~= 0 then str = str .. "PAETH " end

    return str
end

---Reset params to default value
---@param p spngt_params
local function reset_params(p)
    if p == nil then error("invalid argument", 2) end

    --p.override_defaults = false
    p.compression_level = 6 --for zlib this is Z_DEFAULT_COMPRESSION
    p.zlib_strategy = spngt.Z_FILTERED
    p.filter_choice = spngt.FILTER_CHOICE_ALL
end

---Generate new params table
---@param compression_level integer
---@param zlib_strategy integer
---@param filter_choice integer
---@return spngt_params
local function new_params(compression_level, zlib_strategy, filter_choice)

    ---@type spngt_params
    local p =
    {
        --override_defaults = true,
        compression_level = compression_level,
        zlib_strategy = zlib_strategy,
        filter_choice = filter_choice,
    }

    return p
end

---@param p spngt_params
local function param_str(p)
    local str = tostring(p.compression_level) .. ","
    str = str .. zlib_strategy_name[p.zlib_strategy] .. ","
    str = str .. filter_choice_str(p.filter_choice)

    return str
end

---@param p spngt_params[]
local function print_param_list(p)
    for i=1, #p do
        print(param_str(p[i]))
    end
end

---@param a spngt_result
---@param b spngt_result
local function compare_results_time(a, b)
    if a.time < b.time then
        return true
    else
        return false
    end
end

---@param a spngt_experiment
---@param b spngt_experiment
local function compare_speedup(a, b)
    if a.speedup > b.speedup then
        return true
    else
        return false
    end
end

---Compare params
---@param a spngt_params
---@param b spngt_params
---@return boolean
local function param_eq(a, b)
    if a == nil or b == nil then error("invalid argument", 2) end

    for k, v in pairs(a) do
        if b[k] ~= nil then
            if b[k] ~= v then
                return false
            end
        end
    end
    return true
end

local basename = arg[1]
local csv_filename = basename .. ".csv"
local f = io.open(csv_filename, "w+")

assert(f)

---The last two columns are a ratio between the default settings
f:write("name,compression_level,zlib_strategy,filter_choice,encoded_size,encode_time,size_increase,speedup\n")


--[[
    For each file start with the default settings,
    then potentially faster settings in decreasing order of compression level.

    The default compression level for zlib is 6 (https://github.com/madler/zlib/blob/21767c654d31d2dccdde4330529775c6c5fd5389/deflate.c#L283).
    mem_level is ignored, it's 8 by default and setting it to the maximum
    value of 9 makes no measurable difference.
    Compression strategy and filtering defaults depends on the image type

    Future benchmarks could include testing with zlib-ng to see
    whether the conclusions change or if it's just an overall speed improvement
    across the board.
]]--

local function add_params(param_list, compression_level, zlib_strategy, filter_choice)
    param_list[#param_list+1] = new_params(compression_level, zlib_strategy, filter_choice)
end

---Generates and returns an encode param list based on ihdr
---@param ihdr spngt_ihdr
---@return spngt_params[]
local function gen_params(ihdr)
    if ihdr == nil then error("invalid argument", 2) end

    ---@type spngt_params[]
    local param_list = {}

    for i=1, #filter_choices do
        for strategy=1, #zlib_strategies  do
            for compression_level=6, 1, -1 do
                add_params(param_list, compression_level, zlib_strategies[strategy], filter_choices[i])
            end
        end
    end

    --Fastest, largest file size
    add_params(param_list, 0, spngt.Z_DEFAULT_STRATEGY, spngt.DISABLE_FILTERING)

    --Defaults are different for indexed color and <8-bit images
    if ihdr.color_type == 3 or ihdr.bit_depth < 8 then
        --speedup and size_increase values are derived from the first (=default) configuration, it has to be first in the list
        local default_idx = 0
        for i=1, #param_list do
            if param_list[i].compression_level == 6 and
               param_list[i].zlib_strategy == spngt.Z_DEFAULT_STRATEGY and
               param_list[i].filter_choice == spngt.DISABLE_FILTERING then
                default_idx = i
            end
        end
        assert(default_idx ~= 0)
        local tmp = param_list[default_idx]
        table.remove(param_list, default_idx)
        table.insert(param_list, 1, tmp)
    end

    return param_list
end


---@type spngt_experiment[]
local experiments = {}

--arg[0] = main.lua, arg[1] = test_name, arg[2] = file1.png, arg[3] = file2.png ...
local n_files = #arg - 2

for i=2, n_files do
    local dir, filename = arg[i]:match("(.*/)(.*)")
    local png = spngt.prefetch(arg[i])
    local ihdr = png:get_ihdr()
    local param_list = gen_params(ihdr)

    ---@type spngt_experiment[]
    local file_experiments = {}

    for j=1, #param_list do

        local params = param_list[j]
        local res, msg = png:encode_benchmark(params)

        if(res == nil) then
            f:write("ERROR:", msg .. "\n")
            error(msg)
        end

        file_experiments[j] =
        {
            filename = filename,
            params = params,
            results = res
        }

        file_experiments[j].size_increase = res.png_size / file_experiments[1].results.png_size
        file_experiments[j].speedup = file_experiments[1].results.time / res.time
    end

    local results_default = file_experiments[1].results

    --Sort results by speedup before printing
    table.sort(file_experiments, compare_speedup) --Not always helpful, comment out if not needed


    for j=1, #param_list do

        local params = file_experiments[j].params
        local res = file_experiments[j].results

        f:write(filename .. ",")
        f:write(params.compression_level .. ",")
        f:write(zlib_strategy_name[params.zlib_strategy] .. ",")
        f:write(filter_choice_str(params.filter_choice) .. ",")
        f:write(res.png_size .. ",")
        f:write(res.time // 1000 .. ",")
        local size_increase =  res.png_size / results_default.png_size
        local speedup = results_default.time / res.time
        f:write(string.format("%.2f", size_increase) .. ",")
        f:write(string.format("%.2f", speedup))
        f:write("\n")

        experiments[#experiments+1] = file_experiments[j]
    end

    png:discard()
end

print("wrote results of " .. n_files .. " images to " .. csv_filename)

f:flush()
f:close()

local params_filename = basename .. "_params.csv"
local pf = io.open(params_filename, "w+")
assert(pf)

pf:write("compression_level,zlib_strategy,filter_choice,speedup_avg,size_increase_avg,speedup_worst,speedup_best,size_increase_worst,size_increase_best\n")

table.sort(experiments, compare_speedup)

---@type integer[]
local top = {}
local top_n = #experiments

top[1] = 1

---@type spngt_stats
local stats = {}

for i=1, #experiments  do

    for j=1, #top do
        if param_eq(experiments[top[j]].params, experiments[i].params) then
           goto duplicate_param
        end
    end

    top[#top+1] = i

    ::duplicate_param::

    if #top == top_n then
        break
    end
end


for i=1, #top do
    local speedup_best = experiments[i].speedup
    local speedup_worst = experiments[i].speedup
    local speedup_avg = experiments[i].speedup
    local param_instances = 1

    local size_increase_best = experiments[i].size_increase
    local size_increase_worst = experiments[i].size_increase
    local size_increase_avg = experiments[i].size_increase

    for j=i+1, #experiments do
        if param_eq(experiments[i].params, experiments[j].params) then

            param_instances = param_instances + 1

            speedup_avg = speedup_avg + experiments[j].speedup
            size_increase_avg = size_increase_avg + experiments[j].size_increase

            if speedup_worst > experiments[j].speedup then
                speedup_worst = experiments[j].speedup
            end

            if size_increase_worst < experiments[j].size_increase then
                size_increase_worst = experiments[j].size_increase
            end

            if size_increase_best > experiments[j].size_increase then
                size_increase_best = experiments[j].size_increase
            end
        end
    end

    speedup_avg = speedup_avg / param_instances
    size_increase_avg = size_increase_avg / param_instances

--[[    print("top len: " .. #top .. ", values: ")

    for j=1, #top do
        print(top[j] .. " ")
    end]]--

    pf:write(string.format("%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n" ,
            param_str(experiments[top[i]].params), speedup_avg, size_increase_avg, speedup_worst,
                speedup_best, size_increase_worst, size_increase_best))
end


pf:flush()
pf:close()

print("wrote stats of " .. #top .. " parameter sets to " .. params_filename)
