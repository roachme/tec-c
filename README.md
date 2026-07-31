<div align="center">

# Tec

**Terminal project and task manager**

Every task gets a workspace of its own, and one command puts you back inside it.

<img src="https://img.shields.io/badge/version-v0.12.0-4c8eda?style=flat-square" alt="version" />
<img src="https://img.shields.io/badge/written%20in-C-555555?style=flat-square" alt="written in C" />
<img src="https://img.shields.io/badge/platform-Linux-555555?style=flat-square" alt="platform" />
<img src="https://img.shields.io/badge/license-GPLv3-4c8eda?style=flat-square" alt="license" />

</div>

<br />

![command example](docs/tec.gif)

<br />

## Why Tec

Work is rarely a single task. It is a handful of them at once, and the cost is not
the work itself — it is everything around it. Where were those notes. What was that
scratch file called. Where did I leave off.

Tec gives every task a place of its own and takes you there in one command.

| The daily annoyance | What Tec does about it |
| :-- | :-- |
| Notes, sketches and scratch files pile up wherever you happened to be | Every task owns a directory, always laid out the same way |
| Coming back to a task means reconstructing where everything was | ` tec cd ` drops your shell straight into it |
| Switching back and forth costs more than the switch is worth | ` tec cd - ` returns you to the previous task, the way ` cd - ` does |
| Task tools tend to arrive with a workflow and a toolchain attached | Tec brings neither. Add a plugin if you want one, or do not |

Three ideas hold it together:

1. **Structure** — one predictable workspace per task, made of ordinary directories
   and files you can read, grep and back up with the tools you already have.
2. **Speed** — the fewest keystrokes between you and the work. Tec remembers where
   you are, so most commands need no arguments at all.
3. **Restraint** — the core does not care how you work. It has no opinion about git,
   about kanban, about any of it. What it lacks on purpose, plugins can add.

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

From here on the task directory is yours. Keep notes in it, sketch in it, clone
repos into it — Tec only cares that you can always get back.

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

Tec remembers a current and a previous entry at each of the three levels. That is
what makes ` tec cd - ` work, and why ` tec cat ` or ` tec rm ` need nothing
spelled out: they act on wherever you are.

<br />

## Requirements

Tec itself needs a C toolchain and libconfig:

``` bash
sudo apt install -y libconfig-dev
```

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

The two paths in ` base ` are the only settings you have to care about: where tasks
are kept, and where plugins are looked for. Everything else has a working default.

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

## Extending it

Everything above works on its own. Nothing below is required.

Tec deliberately stops at the workspace. It does not manage your repos, does not
track a task through stages, does not search your notes. When you want one of
those, you add a plugin — and when you do not, nothing is nagging you about it.

A plugin is an executable sitting at ` base.pgn/NAME/NAME ` , which Tec runs as
` tec NAME ` . That is the whole contract, so a plugin can be written in
anything: the ones below are shell, Python and Rust.

Hooks then let a plugin ride along with a builtin command. ` bincmd ` is the
builtin that fires the hook, and there are two groups of them:

- **action** — the plugin command runs as a side effect. Adding a task can set up
  whatever else that task needs.
- **cat** — whatever the plugin prints as ` key : value ` lines is merged into the
  output of ` tec cat ` , so a plugin can add units of its own to a task.

A hook only works once the plugin it names is installed; until then the command
that fires it reports a failure. Leave the groups empty until you have one.

### Installing plugins

` nine ` is the plugin manager. To get it:

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
| [conv](https://github.com/roachme/tec-conv.git) | Workflow engine: kanban and custom workflows |

Each of them carries setup notes of its own, so read its README before use.

<br />

## Good to know

- ` tec help ` lists every command; ` tec help COMMAND ` explains one of them.
- A task ID is up to 8 characters long; environment and desk names up to 10.
- ` tec add ` with no ID makes one up for you: 00000001, 00000002, and so on.
- Most commands fall back to the current environment, desk and task, so
  ` tec cat ` , ` tec set -P high ` and ` tec rm ` all work with no arguments.
- Aliases live in ` tec.cfg ` : ` dir = "ls" ` makes ` tec dir ` run ` tec ls ` .
- Colors, hooks and debug output can be flipped per run with ` -C ` , ` -H `
  and ` -D ` , which is handy when a hook misbehaves.

<br />

## License

Released under the [GNU General Public License v3.0](LICENSE).
