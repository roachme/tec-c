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

Daily work is rarely a single task. It is usually a handful of them, each with its
own repos, branches, notes and half-finished thoughts.

| The daily annoyance | What Tec does about it |
| :-- | :-- |
| Cloning repos and cutting a branch in each of them, over and over | Plugins drive a bunch of repos at once |
| Logs, sketches and notes scattered all over the disk | Every task gets its own structured workspace |
| Switching a task means switching context, notes and branches | ` tec cd ` takes you back to a task, ` tec cd - ` to the previous one |

Three ideas hold it together:

1. **Structure** — a predictable workspace for every task.
2. **Speed** — get exactly where you want with the fewest keystrokes.
3. **Automation** — hand the boring parts over to plugins and hooks.

Tec is meant to be bent to your workflow: tune it through a config file and extend
it with ready-made or homemade plugins.

<br />

## A quick look

``` console
$ tec init                            # run once: sets the workspace up
$ tec env add work                    # an environment to keep tasks in

$ tec add parser -D "fix the header parser"
$ tec add pr-42 -D "review PR 42"

$ tec ls
parser    fix the header parser
pr-42     review PR 42

$ tec cd parser                       # your shell follows along
$ pwd
/home/you/tectask/work/desk/parser

$ tec cat                             # what is the current task again?
id     : parser
prio   : mid
type   : task
date   : 20260731
desc   : fix the header parser

$ tec set -P high                     # bump its priority
$ tec cd -                            # back to what you were doing before
```

<br />

## How it fits together

Tec keeps three levels, all of them plain directories on disk:

- **Environment** — a context you work in: a job, a side project, a client.
- **Desk** — a board inside an environment where tasks sit. A fresh environment
  comes with one named ` desk ` .
- **Task** — the thing you actually work on. It owns a directory for your notes,
  sketches and repos, plus units such as description, priority and type.

```
~/tectask/
└── work/                environment
    └── desk/            desk
        ├── parser/      task — notes, sketches and repos live here
        └── pr-42/
```

Tec always remembers the current environment, desk and task, which is why most
commands need no arguments at all — and why ` tec cd - ` can drop you straight back
into the previous one.

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

Steps 1 and 2 are what ` make install ` does for you, so run that instead if you
like. Step 3 is on you either way.

**1. Put the binary on your PATH.** The build leaves it at ` build/_tec ` ; move
it into one of the directories listed in ` PATH ` — ` ~/.local/bin ` is a
good spot.

**2. Add the shell wrapper** to your shell rc file, whether that is
` ~/.bashrc ` , ` ~/.zshrc ` or another one. The wrapper is what allows
Tec to change the working directory of your current shell.

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
` ~/.config/tec/tec.cfg ` , and fill it with the content below. Tec reads the
first of the two it finds, or the one passed with ` tec -f PATH ` .

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

/* Hooks run a plugin command on top of a builtin one, where 'bincmd' is
 * the builtin command that fires the hook.  Two groups are supported:
 *   cat    - plugin output is merged into the units 'tec cat' shows;
 *   action - plugin command runs as a side effect of the builtin one.
 * Both are left empty here.  Fill them in once the plugin is installed,
 * otherwise the builtin command fails, e.g.
 *   action = ( { bincmd = "add"; pgname = "gmux"; pgncmd = "cd" }, );
 */
hooks = {
    cat = ();
    action = ();
};

/* Aliases can be used for plugin commands as well.
 * Nested aliases are not supported yet, i.e. an alias can be
 * either a builtin command or a plugin.  */
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
    Run 'tec help help' to get more info on a command.

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

## Plugins

Plugins are installed by ` nine ` , the Tec plugin manager. To get it:

``` bash
PGNDIR="$HOME/.local/lib/tec/pgn"
git clone https://github.com/roachme/tec-nine.git "$PGNDIR/nine"
```

Two things to keep in mind, here and for every other plugin:

- the repo directory name carries **no** ` tec- ` prefix;
- ` PGNDIR ` has to match ` base.pgn ` from the ` tec.cfg ` shown above.

That is the whole mechanism: an executable at ` base.pgn/NAME/NAME ` is run by
` tec NAME ` , and hooks can call it for you.

A few worth a look:

| Plugin | What it does |
| :-- | :-- |
| [nine](https://github.com/roachme/tec-nine.git) | Tec plugin manager |
| [gmux](https://github.com/roachme/tec-gmux.git) | Manage a bunch of git repos |
| [find](https://github.com/roachme/tec-find.git) | Find stuff in tasks |
| [conv](https://github.com/roachme/tec-conv.git) | Workflow engine: kanban and custom workflows |

Each of them carries setup notes of its own, so read its README before use.

<br />

## Good to know

- ` tec help ` lists every command; ` tec help COMMAND ` explains one of them.
- A task ID is up to 8 characters long; environment and desk names up to 10.
- ` tec add ` with no ID makes one up for you: 00000001, 00000002, and so on.
- Most commands fall back to the current environment, desk and task, so
  ` tec cat ` , ` tec set -P high ` and ` tec rm ` all work with no arguments.
- Aliases and hooks live in ` tec.cfg ` . A hook only works once the plugin it
  names is installed — until then the command that fires it reports a failure.

<br />

## License

Released under the [GNU General Public License v3.0](LICENSE).
