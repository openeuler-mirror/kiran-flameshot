<div align="center">
  <p>
    <h1>
      <a href="https://gitee.com/Bmen/kiran-kiran-flameshot">
        <img src="img/app/kiran128x128.png" alt="Kiran-Kiran-Flameshot" />
      </a>
      <br />
      Kiran-Flameshot
    </h1>
    <h4>Powerful yet simple to use screenshot software.</h4>
  </p>
</div>

## Index

- [Features](#features)
- [Usage](#usage)
  - [CLI configuration](#cli-configuration)
- [Keyboard Shortcuts](#keyboard-shortcuts)
  - [Local](#local)
  - [Global](#global)
    - [On KDE Plasma desktop](#on-kde-plasma-desktop)
- [Considerations](#considerations)
- [Installation](#installation)
- [Compilation](#compilation)
  - [Dependencies](#dependencies)
    - [Compile-time](#compile-time)
    - [Run-time](#run-time)
    - [Optional](#optional)
- [License](#license)

## Features

- Customizable appearance.
- Easy to use.
- In-app screenshot edition.
- DBus interface.
- Upload to Imgur.

## Usage

Example commands:

- Capture with GUI:

    ```shell
    kiran-flameshot gui
    ```

- Capture with GUI with custom save path:

    ```shell
    kiran-flameshot gui -p ~/myStuff/captures
    ```

- Capture with GUI after 2 seconds delay (can be useful to take screenshots of mouse hover tooltips, etc.):

    ```shell
    kiran-flameshot gui -d 2000
    ```

- Fullscreen capture with custom save path (no GUI) and delayed:

    ```shell
    kiran-flameshot full -p ~/myStuff/captures -d 5000
    ```

- Fullscreen capture with custom save path copying to clipboard:

    ```shell
    kiran-flameshot full -c -p ~/myStuff/captures
    ```

- Capture the screen containing the mouse and print the image (bytes) in PNG format:

    ```shell
    kiran-flameshot screen -r
    ```

- Capture the screen number 1 and copy it to the clipboard:

    ```shell
    kiran-flameshot screen -n 1 -c
    ```

In case of doubt choose the first or the second command as shortcut in your favorite desktop environment.

A systray icon will be in your system's panel while Kiran-Flameshot is running.
Do a right click on the tray icon and you'll see some menu items to open the configuration window and the information window.
Check out the information window to see all the available shortcuts in the graphical capture mode.

### CLI configuration

You can use the graphical menu to configure Kiran-Flameshot, but alternatively you can use your terminal or scripts to do so.

- Open the configuration menu:

    ```shell
    kiran-flameshot config
    ```

- Show the initial help message in the capture mode:

    ```shell
    kiran-flameshot config --showhelp true
    ```

- For more information about the available options use the help flag:

    ```shell
    kiran-flameshot config -h
    ```

## Keyboard shortcuts

### Local

These shortcuts are available in GUI mode:

|  Keys                                                                     |  Description                                                   |
|---                                                                        |---                                                             |
| <kbd>←</kbd>, <kbd>↓</kbd>, <kbd>↑</kbd>, <kbd>→</kbd>                    | Move selection 1px                                             |
| <kbd>Shift</kbd> + <kbd>←</kbd>, <kbd>↓</kbd>, <kbd>↑</kbd>, <kbd>→</kbd> | Resize selection 1px                                           |
| <kbd>Esc</kbd>                                                            | Quit capture                                                   |
| <kbd>Ctrl</kbd> + <kbd>C</kbd>                                            | Copy to clipboard                                              |
| <kbd>Ctrl</kbd> + <kbd>S</kbd>                                            | Save selection as a file                                       |
| <kbd>Ctrl</kbd> + <kbd>Z</kbd>                                            | Undo the last modification                                     |
| <kbd>Spacebar</kbd>                                                       | Toggle visibility of sidebar with options of the selected tool, color picker for the drawing color and history menu |
| Right Click                                                               | Show the color wheel                                              |
| Mouse Wheel                                                               | Change the tool's thickness                                    |

<kbd>Shift</kbd> + drag a handler of the selection area: mirror redimension in the opposite handler.

### Global

If you want use Kiran-Flameshot as a default screenshot utility, chances are you want to launch it using the <kbd>Prt Sc</kbd> key. Kiran-Flameshot doesn't yet offer a fully-automated option to do so, but you can configure your system to do so.

#### On KDE Plasma desktop

To make configuration easier, there's a [file](docs/shortcuts-config/kiran-flameshot-shortcuts-kde) in the repository that more or less automates this process. This file will assign the following keys to the following actions by default:

|  Keys                                                  |  Description                                                                       |
|---                                                     |---                                                                                 |
| <kbd>Prt Sc</kbd>                                      | Start the Kiran-Flameshot screenshot tool and take a screenshot                          |
| <kbd>Ctrl</kbd> + <kbd>Prt Sc</kbd>                    | Wait for 3 seconds, then start the Kiran-Flameshot screenshot tool and take a screenshot |
| <kbd>Shift</kbd> + <kbd>Prt Sc</kbd>                   | Take a full-screen (all monitors) screenshot and save it                           |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>Prt Sc</kbd> | Take a full-screen (all monitors) screenshot and copy it to the clipboard          |

If you don't like the defaults, you can change them manually later.

Steps for using the configuration:

1. The configuration file configures shortcuts so that Kiran-Flameshot automatically saves (without opening the save dialog) screenshots to `~/Pictures/Screenshots` folder. Make sure you have that folder by running the following command:

    ```shell
    mkdir -p ~/Pictures/Screenshots
    ```

    (If you don't like the default location, you can skip this step and configure your preferred directory later.)
2. Download the configuration file:

    ```shell
    cd ~/Desktop
    wget https://gitee.com/Bmen/kiran-flameshot/master/docs/shortcuts-config/kiran-flameshot-shortcuts-kde
    ```
3. Go to _System Settings_ → _Shortcuts_ → _Custom Shortcuts_.
4. If there's one, you'll need to disable an entry for Spectacle, the default KDE screenshot utility first because its shortcuts might collide with Kiran-Flameshot's ones; so, just uncheck the _Spectacle_ entry.
5. Click _Edit_ → _Import..._, navigate to the Desktop folder (or wherever you saved the configuration file) and open the configuration file.
6. Now the Kiran-Flameshot entry should appear in the list. Click _Apply_ to apply the changes.
7. If you want to change the defaults, you can expand the entry, select the appropriate action and modify it as you wish; the process is pretty mush self-explanatory.

## Considerations

- Experimental Gnome Wayland and Plasma Wayland support.

- If you are using Gnome you need to install the [Gnome Shell Extension Appindicator](https://github.com/Ubuntu/gnome-shell-extension-appindicator) extension in order to see the systemtray icon.

- In order to speed up the first launch of Kiran-Flameshot (DBus init of the app can be slow), consider starting the application automatically on boot.

- Press <kbd>Enter</kbd> or <kbd>Ctrl</kbd> + <kbd>C</kbd> when you are in a capture mode and you don't have an active selection and the whole desktop will be copied to your clipboard! Pressing <kbd>Ctrl</kbd> + <kbd>S</kbd> will save your capture in a file! Check the [Shortcuts](#shortcuts) for more information.

- Execute the command `kiran-flameshot` without parameters to launch a running instance of the program without taking actions.


## Compilation

To build the application in your system, you'll need to install the dependencies needed for it and Package names might be different for each distribution, see [Dependencies](#dependencies) below for more information.

### Dependencies

#### Compile-time

- Qt >= 5.3
  + Development tools
- GCC >= 4.9.2

#### Run-time

- Qt
  + SVG

#### Optional

- Git
- OpenSSL
- CA Certificates

## License
- The main code is licensed under [GPLv3](LICENSE)
