-- Configuration file for x48ng

-- `config_dir` is relative to $HOME or absolute
config_dir = ".config/x48ng"

-- Pathes are either relative to `config_dir` or absolute
rom = "rom"
ram = "ram"
state = "state"
port1 = "port1"
port2 = "port2"

pseudo_terminal = false

serial = false

serial_line = "/dev/ttyS0"

verbose = false

debugger = false

throttle = false


--------------------
-- User Interface --
--------------------

frontend = "tui" -- possible values: "x11", "sdl", "tui"
hide_chrome = false

fullscreen = false


netbook = false


mono = false

gray = false


x11_visual = "default"

font_small = "-*-fixed-bold-r-normal-*-14-*-*-*-*-*-iso8859-1"
font_medium = "-*-fixed-bold-r-normal-*-15-*-*-*-*-*-iso8859-1"
font_large = "-*-fixed-medium-r-normal-*-20-*-*-*-*-*-iso8859-1"
font_devices = "-*-fixed-medium-r-normal-*-12-*-*-*-*-*-iso8859-1"
