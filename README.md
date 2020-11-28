# ADVANCED CARTOGRAPHY 0.9.1
A Noita mod that creates a map of the world as you play through it.

![banner](examples/banner_compressed.jpg)

## System Requirements

- Windows Vista or newer, 64 bit
- Extra hard drive space to store images and maps (must be same drive you have Noita on)
- 4 GB or more of RAM
- Willingness to play in windowed mode (full windowed, where you can see the borders)
- Everything else you need to play Noita (in particular, a good CPU)

## Disclaimer

I have no idea what I'm doing. I learned what a pointer is two weeks ago, and had never used Lua, C++, or C prior to this project. There are many bugs, (known ones being listed further down). It is extremely unpolished. Do not expect greatness. Do not expect competence. Embrace pain, you will be experiencing a lot of it soon.

## Donations

Help me upgrade my gaming toaster:

- Bitcoin: bc1qkljd7xp00h89zp8p8a0nqw3hctsqmzsmqtss0p
- Etherium: 0x22AeD69E6683F11Ac9bD672f86F0044E875D5f6D

## Usage

- Get Noita
- Download mod (latest release build here: [LINK])
- Put mod (mod folder should have the name 'advanced_cartography') in your mods folder in Noita
    - Path is normally "..\Steam\steamapps\common\Noita\mods\advanced-cartography"
- Open Noita, in "Mods" select "Enable Unsafe Mods" and then enable the AC mod (click on it in the menu)
- Play the game in windowed mode (fullscreen-windowed only works sometimes). AC will take screenshots as you play.
- In the mod root folder (advanced_cartography), there is an application called 'Stitcher'. Run that.
    - It will open a command prompt which will guide you through the process of stitching the screenshots into a full map.
    - If you keep Stitcher open while you play, you will be able to redraw and review the map during the game, allowing you to check for unexplored areas and find items you left behind.
- Screenshots are saved in the 'raws' folder, drawn maps are stored in the 'maps' folder.
- Screenshots are automatically scaled to the virtual dimensions of the game (not the resolution of the window, the actual number of pixels that Noita is calculating and displaying). This is done to maintain consistent coordinates and keep file sizes manageable.
- Worlds are sorted by seed, and automatically tagged when they are Daily Runs or Daily Practice runs
- When you die, AC will take a screenshot of the final frame of your life
- If you enter a New Game +, a new seed folder is created with the appropriate tag, and the old one remains the same

## Planned Features

- Mod settings which allow you to control size of screenshot deadzone, behavior for multiple runs on the same seed, etc. 
- Advanced stitch function, integration with mod GUI (stitch and view maps from the pause menu, instead of an external mode)
- Multithreaded screenshot compression

## Known Bugs

Each of these already has a ticket in [issues](https://github.com/d0ng1er/advanced_cartography/issues).

- Teleporting corrupts screenshot coordinates
- Polymorphing gets counted as death (current fix is to restart the game after polymorphing to continue capturing, then combine the batch folders from each world into one of the seed folders when you want to stitch)

## Special Thanks

- Inspiration for this project: [Dadido3/noita-mapcap](https://github.com/Dadido3/noita-mapcap)
