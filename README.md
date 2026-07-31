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

**What the demo shows**, beat by beat:

1. ` tec init ` lays the workspace down. Once, ever.
2. An environment called ` test ` is created and three tasks are dropped into it.
3. ` tec ls ` shows what is on the plate.
4. A plain ` cd ` wanders off home — and ` tec cd ` walks straight back into the
   current task. No path typed, none remembered.
5. ` tec cd - ` flips to the task before it.
6. ` tec rm -f ` deletes the task being stood in, and the shell is carried out of
   the directory that no longer exists instead of being stranded there.

That is the loop the whole tool is built around. The script behind the recording
is ` docs/cmdscript.txt ` if you want to replay it yourself.

<br />

## The idea

Work is rarely a single task. It is a handful of them at once, and most of the pain
is not the work — it is everything around it. Where were those notes. What was that
scratch file called. Where did I leave off.

Tec is an answer to that and nothing more: **give every task a place of its own,
keep every one of those places the same shape, and make getting there cost one
command.** Organized, so everything is where you left it by design rather than by
luck. Optimized, so returning to a task is not a small research project.

What Tec does **not** do is just as deliberate. It has no opinion about git, about
stages and boards, about how you name things or when a task is finished. It will
not make you adopt a methodology to file a note. That restraint is the feature —
the core stays small enough to fit any workflow, and anything genuinely missing can
be bolted on later without the core caring.

| The daily annoyance | What Tec does about it |
| :-- | :-- |
| Notes, sketches and scratch files pile up wherever you happened to be | Every task owns a directory, always laid out the same way |
| Coming back to a task means reconstructing where everything was | ` tec cd ` drops your shell straight into it |
| Switching back and forth costs more than the switch is worth | ` tec cd - ` returns you to the previous task, the way ` cd - ` does |
| Task tools tend to arrive with a workflow and a toolchain attached | Tec brings neither |

It is one C binary with one library dependency, and everything it stores is
ordinary directories and files — readable, greppable and backed up by whatever you
already use.

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

From there the task directory is yours. Keep notes in it, sketch in it, clone repos
into it — Tec only cares that you can always get back.

At every one of the three levels Tec remembers a current and a previous entry. That
is what makes ` tec cd - ` work, and why ` tec cat ` or ` tec rm ` need
nothing spelled out: they act on wherever you already are.

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
Tec to change the working directory of your current shell, which is the whole
trick behind ` tec cd ` .

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

The two paths under ` base ` are the only settings that need your attention: where
tasks are kept, and where plugins are looked for. Everything else has a working
default, and a config holding nothing but ` base ` is perfectly valid.

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

## Day to day

Things that are easy to miss and worth knowing early.

**Adding a task moves you into it.** ` tec add parser ` creates the task and puts
your shell in its directory, ready to work. Pass ` -N ` to create it and stay
where you are.

**Almost nothing needs arguments.** ` tec cat ` , ` tec set -P high ` and
` tec rm ` act on the current task. Name a task only when you mean a
different one.

**` - ` means "the previous one" at every level.** ` tec cd - ` for tasks,
` tec env cd - ` for environments, and the same for desks. Forgotten which is
which? ` tec ls -t ` prints just the current and previous task.

**Deleting what you are standing in is safe.** Remove the task you are inside and
Tec carries your shell out first, so the session is never left in a directory that
was deleted underneath it.

**Tasks carry a little metadata.** A type — ` task ` , ` bugfix ` ,
` feature ` , ` hotfix ` — and a priority from ` lowest ` to
` highest ` , set with ` tec set -T bugfix -P high ` . Anything else you
want to record is a file in the task directory.

**Names are short on purpose.** A task ID is up to 8 characters, environment and
desk names up to 10. Skip the ID entirely and Tec generates one: 00000001, 00000002
and so on. ` tec mv ` renames a task you later think better of.

**Aliases save keystrokes.** ` dir = "ls" ` in the config makes ` tec dir `
run ` tec ls ` ; ` els = "env ls" ` works for subcommands too.

**When something misbehaves**, colors, hooks and debug output can each be flipped
for a single run with ` -C ` , ` -H ` and ` -D ` , as in
` tec -H off ls ` . And ` tec help COMMAND ` explains any command in
full.

<br />

## Plugins

**Plugins are not what Tec is.** Everything above stands on its own, and plenty of
people will never need this section. It is here because a small core has to admit
that some jobs belong to somebody else.

Tec stops at the workspace. It does not drive your repos, does not walk a task
through stages, does not search your notes. When you want one of those, you add a
plugin — and when you do not, nothing nags you about it.

A plugin is an executable sitting at ` base.pgn/NAME/NAME ` , which Tec runs as
` tec NAME ` . That is the entire contract, so a plugin can be written in
anything at all: the ones below are shell, Python and Rust.

Hooks then let a plugin ride along with a builtin command, where ` bincmd `
is the builtin that fires the hook:

- **action** — the plugin command runs as a side effect, so adding or switching a
  task can set up whatever else that task needs.
- **cat** — whatever the plugin prints as ` key : value ` lines is merged into the
  output of ` tec cat ` , letting a plugin add units of its own to a task.

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

## License

Released under the [GNU General Public License v3.0](LICENSE).
