<div align="center">

# Tec

**Terminal project and task manager**

One workspace per task — notes, sketches, repos and git branches, always in sync.

<img src="https://img.shields.io/badge/version-v0.12.0-4c8eda?style=flat-square" alt="version" />
<img src="https://img.shields.io/badge/written%20in-C-555555?style=flat-square" alt="written in C" />
<img src="https://img.shields.io/badge/platform-Linux-555555?style=flat-square" alt="platform" />
<img src="https://img.shields.io/badge/license-GPLv3-4c8eda?style=flat-square" alt="license" />

</div>

<br />

![command example](docs/tec.gif)

<br />

## Why Tec

Daily work is rarely one task at a time. It is a handful of them, each with its own
repos, branches, notes and half finished thoughts.

| The daily annoyance | What Tec does about it |
| :-- | :-- |
| Cloning repos and cutting a branch in each of them, over and over | Plugins drive a bunch of repos at once |
| Logs, sketches and notes scattered all over the disk | Every task gets its own structured workspace |
| Switching a task means switching context, notes and branches | ` tec cd ` takes you back to a task, ` tec cd - ` to the previous one |

Three ideas hold it together:

1. **Structure** — a predictable workspace for every task.
2. **Speed** — get exactly where you want with the fewest keystrokes.
3. **Automation** — hand the boring parts over to plugins and hooks.

Tec is meant to be bent to your workflow: tune it through a config file, extend it
with ready made or homemade plugins.

<br />

## Requirements

Tec itself needs a C toolchain and libconfig:

``` bash
sudo apt install -y libconfig-dev
```

Plugins may pull in dependencies of their own — see the README of each plugin.

<br />

## Build

``` bash
make
```

<br />

## Installation

**1. Put the binary on your PATH.** Once compiled, move ` _tec ` into one of the
directories listed in ` PATH ` — ` ~/.local/bin ` is a good spot.

**2. Add the shell wrapper** to your shell rc file, be it ` ~/.bashrc ` ,
` ~/.zshrc ` or another one. It is what lets Tec change the directory of
the shell you are sitting in.

``` bash
#!/usr/bin/env bash

function tec()
{
    local tecstatus;
    local pwdfile="/tmp/tecpwd"

    _tec "$@"
    tecstatus="$?"

    test -s "$pwdfile" && cd "$(cat "$pwdfile")" || return "$tecstatus"
    return "$tecstatus"
}
```

**3. Create a config file**, either at ` ~/.tec/tec.cfg ` or at
` ~/.config/tec/tec.cfg ` , and fill it with the content below.

```
base = {
    task = "$HOME/tectask";
    pgn = "$HOME/.local/lib/tec/pgn";
};

options = {
    hook = true;    /* enable hooks */
    color = true;   /* enable colors */
    debug = false;  /* disable debug info */
};

/* list of hooks for all environments */
hooks = {
    show = (
        { bincmd = "show"; pgname = "gmux"; pgncmd = "show" },
    );
    action = (
        { bincmd = "add"; pgname = "gmux"; pgncmd = "cd" },
    );
    list = ();
};

/* Alias can be used for plugin commands as well.
 * Tho for rn there is no support for nested aliases, i.e. alias
 * can be either builtin command or plugin.  */
alias = {
    dir = "ls";
    els = "env ls";
    dcat = "desk cat";
};
```

<br />

## Commands

```
Usage: tec [OPTION]... COMMAND|PLUGIN
    Run 'tec help tec' to get more info.
    Run 'tec help help' to get more info on command.

    System:
      help       - Show help for commands.
      init       - Init directory structure.
      version    - Display version information.

    Basic:
      add        - Add a new task to environment.
      cat        - Concatenate task unit values.
      cd         - Switch to task.
      ls         - List environment tasks.
      mv         - Move (rename) tasks.
      rm         - Remove task from environment.
      set        - Set task unit values.

    Object:
      cfg        - Manage and show configs.
      desk       - Manage and show desks.
      env        - Manage and show environments.

```

<br />

## Basic workflow

``` bash
tec init              # set the directory structure up
tec env add test      # create an environment
tec add test1         # fill it with tasks
tec add test2
tec ls                # list tasks of the current environment
tec cat test1         # show what a task holds
tec cd                # jump to the current task
tec cd -              # jump back to the previous one
```

<br />

## Plugins

Plugins are installed by ` nine ` , the Tec plugin manager. To get it:

``` bash
PGNDIR="$HOME/.local/lib/tec/pgn"
git clone https://github.com/roachme/tec-nine.git "$PGNDIR/nine"
```

Two things to keep in mind, here and for every other plugin:

- the repo directory name carries **no** ` tec- ` prefix;
- ` PGNDIR ` has to match ` base.pgn ` from the ` tec.cfg ` shown above.

A few worth a look:

| Plugin | What it does |
| :-- | :-- |
| [nine](https://github.com/roachme/tec-nine.git) | Tec plugin manager |
| [gmux](https://github.com/roachme/tec-gmux.git) | Manage a bunch of git repos |
| [find](https://github.com/roachme/tec-find.git) | Find stuff in tasks |

Run ` tec-pgn ` for the full list of Tec plugins.

<br />

## Tips

- ` tec help ` lists every command.
- ` tec help COMMAND ` explains one of them.

<br />

## License

Released under the [GNU General Public License v3.0](LICENSE).
