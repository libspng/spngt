--EmmyLua annotations for the spngt API
--Only used for intellisense, not used at runtime.

local spngt = {}

---@class spngt_ihdr
---@field width integer
---@field height integer
---@field bit_depth integer
---@field color_type integer
---@field interlace_method integer
local spngt_ihdr = {}

---@class spngt_result
---@field png_size integer
---@field time integer --Encode/decode time
---@field ihdr spngt_ihdr
local spngt_result = {}

---@class spngt_file
local spngt_file = {}

---@class spngt_params
---@field format integer
---@field override_defaults boolean
---@field compression_level integer
---@field window_bits integer
---@field mem_level integer
---@field zlib_strategy integer
---@field filter_choice integer
local spngt_params = {}

---Prefetch PNG file
---@param filename string
---@return spngt_file
function spngt.prefetch(filename)
end

---Get ihdr from PNG
---@return spngt_ihdr
function spngt_file:get_ihdr()
end

---Discard cached file and associated buffers
function spngt_file:discard()
end

---Measure encode time
---@param params spngt_params
---@return spngt_result
function spngt_file:encode_benchmark(params)
end

--spngt_params.zlib_strategy
spngt.Z_FILTERED = 1
spngt.Z_HUFFMAN_ONLY = 2
spngt.Z_RLE = 3
spngt.Z_FIXED = 4
spngt.Z_DEFAULT_STRATEGY = 0

--spngt_params.compression_level
spngt.Z_NO_COMPRESSION = 0
spngt.Z_BEST_SPEED = 1
spngt.Z_BEST_COMPRESSION = 9
spngt.Z_DEFAULT_COMPRESSION = -1

--spngt_params.filter_choice
spngt.DISABLE_FILTERING = 0
spngt.FILTER_CHOICE_NONE = 8
spngt.FILTER_CHOICE_SUB = 16
spngt.FILTER_CHOICE_UP = 32
spngt.FILTER_CHOICE_AVG = 64
spngt.FILTER_CHOICE_PAETH = 128
spngt.FILTER_CHOICE_ALL = (8|16|32|64|128)

spngt.DEFAULT_DECODE_RUNS = 5
spngt.DEFAULT_DECODE_RUNS = 2

return spngt