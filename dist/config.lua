--------------------------------------------------------------------------------
-- Configuration file for x48ng
-- This is a comment
-- `config_dir` is relative to $XDG_CONFIG_HOME/, or $HOME/.config/ or absolute
config_dir = "x48ng"

-- Pathes are either relative to `config_dir` or absolute
rom = "ROMs/gxrom-r"
ram = "ram"
state = "state"
port1 = "port1"
port2 = "port2"

pseudo_terminal = true
serial = false
serial_line = "/dev/ttyS0"

verbose = false
debugger = false
throttle = false

--------------------
-- User Interface --
--------------------
frontend = "sdl2" -- possible values: "x11", "sdl2" "tui", "tui-small", "tui-tiny"
hide_chrome = false
fullscreen = false
scale = 1.000000 -- applies only to sdl2
mono = false
gray = false
leave_shift_keys = false
inhibit_shutdown = true

x11_visual = "default"
netbook = false
font_small = "-*-fixed-bold-r-normal-*-14-*-*-*-*-*-iso8859-1"
font_medium = "-*-fixed-bold-r-normal-*-15-*-*-*-*-*-iso8859-1"
font_large = "-*-fixed-medium-r-normal-*-20-*-*-*-*-*-iso8859-1"
font_devices = "-*-fixed-medium-r-normal-*-12-*-*-*-*-*-iso8859-1"
--------------------------------------------------------------------------------
