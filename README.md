<div align="center">
   <!--<img align="center" width="128px" src="crates/gitbutler-tauri/icons/128x128@2x.png" /> -->
	<h1 align="center"><b>Tec</b></h1>
	<p align="center">
        Terminal project and task manager
    <br />
  </p>
</div>

<br/>


![command example](docs/tec.gif)


## ⇁  Problems
During my work I encounter many subtasks I gotta do on a daily basis.
Among them:
1. Clone Git repos to my machine and create branches in multiple repos (usually different for each task)
2. Keep my task logs, sketches, notes and repos for each task somewhere. Structured and separately
3. Often switch between tasks, which involves context, notes, sketches, and especially Git branches

## ⇁  Solution
Tec comes with 3 basic ideas
1. Structure your workspace for each task
2. Get you exactly where you want with the fewest keystrokes
3. Automate stuff that you're tired of doing at work. That's what plugins are for

## ⇁  Structure
- Customize: Adjust the util for your workflow via the config file.
- Extensible: Extend the util with predefined or your own plugins. You can get the full list of Tec plugins in ``` tec-pgn ```.

## ⇁  Dependencies
Plugins might have dependencies. Take a look at the plugin's README.md for more info.
The main program uses some dependencies. To install them run the commands below:
```
sudo apt install -y libconfig-dev
```


## ⇁  Build
Simply run the command below.
``` bash
make
```

## ⇁  Installation
1. Once you have compiled successfully, put the executable ` _tec ` into one of the directories defined in the env variable ` PATH ` . I put it in ` ~/.local/bin ` .
2. Copy the content of tec.sh into your shell rc file. It's ` ~/.bashrc ` , ` .zshrc ` , etc.

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

3. Create a basic Tec config file, either in ` ~/.tec/tec.cfg ` or ` ~/.config/tec/tec.cfg `, and fill it with the content below

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

/* An alias can be used for plugin commands as well.
 * Though for now there is no support for nested aliases, i.e. an alias
 * can be either a builtin command or a plugin.  */
alias = {
    dir = "ls";
    els = "env ls";
    dcat = "desk cat";
};
```

## ⇁  Tec builtin commands
```
Usage: tec [OPTION]... COMMAND|PLUGIN
    Run 'tec help tec' to get more info.
    Run 'tec help help' to get more info on command.

    System:
      help       - Show help for commands.
      init       - Init directory structure.
      version    - Display version information.

    Basic:
      add        - Add a new task to the environment.
      cat        - Concatenate task unit values.
      cd         - Switch to task.
      ls         - List environment tasks.
      mv         - Move (rename) tasks.
      rm         - Remove a task from the environment.
      set        - Set task unit values.

    Object:
      cfg        - Manage and show configs.
      desk       - Manage and show desks.
      env        - Manage and show environments.

```


## ⇁  Plugins

There is a ` nine ` plugin manager to install plugins.

Here are some plugins to check out \
[nine](https://github.com/roachme/tec-nine.git) - Tec plugin manager \
[gmux](https://github.com/roachme/tec-gmux.git) - Manage a bunch of git repos \
[find](https://github.com/roachme/tec-find.git) - Find stuff in tasks


## ⇁  Install plugin manager

To install the plugin manager run the code below
```
PGNDIR="$HOME/.local/lib/tec/pgn"
git clone https://github.com/roachme/tec-nine.git "$PGNDIR/nine"
```
Note: \
1. Make sure the repo dirname has NO prefix ` tec- `. The same goes for any plugin you install \
2. ` "$PGNDIR" ` is the path set in ` tec.cfg `.
   Make sure ` PGNDIR ` is the same as ` base.pgn ` in ` tec.cfg ` shown above.


## ⇁  Basic workflow
1. To initialize the util type in ` tec init `
2. Now you're ready to create an environment: ` tec env add test `
3. Once the task environment is created you can fill it with tasks: ` tec add test1 `
4. Add one more task: ` tec add test2 `
5. List all your tasks in the current environment: ` tec ls `
6. Show the content of a task: ` tec cat test1 `
7. Sync with the current task: ` tec cd `
8. Or quickly switch to the previous task: ` tec cd - `


## ⇁  Tips
1. Use ` tec help ` to get a list of commands.
2. Or ` tec help COMMAND ` to get help on a specified command.
