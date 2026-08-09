#compdef tec

# TODO:
# 1. Add support for toggles in options -C, -D, -H, etc

local -a subcommands global_opts plugins

global_opts=(
    '(-C)'{-C,--color}'[enable colors]:toggle:(on off)'
    '(-D)'{-D,--debug}'[enable debug mode]:toggle:(on off)'
    '(-h)'{-h,--help}'[show help and exit]'
    '(-F)'{-F,--config}'[path to config file]:directory:_files'
    '(-H)'{-H,--hooks}'[enable hooks]:toggle:(on off)'
    '(-P)'{-P,--plugins}'[plugins directory]:directory:_files'
    '(-T)'{-T,--tasks}'[tasks directory]:directory:_files'
    '(-v)'{-v,--version}'[show version and exit]'
)

_subcommands() {
    subcommands=(
        'add:Add a new task to environment'
        'cat:Concatenate task unit values'
        'cd:Switch to task'
        'cfg:Manage and show configs'
        'desk:Manage and show desks'
        'env:Manage and show environments'
        'help:Show help for commands'
        'init:Init directory structure'
        'ls:List environment tasks'
        'mv:Move (rename) tasks'
        'rm:Remove task from environment'
        'set:Set task unit values'
        'version:Display version information'
    )
}

_tec_help() {
    _arguments \
        '(-d)'{-d,--desc}'[output short description for each topic]' \
        '(-l)'{-l,--list}'[output only description for all commands]' \
        '(-s)'{-s,--synopsis}'[output only a short usage synopsis]' \
        ':command:->command'

    case $state in
        command)
            local -a help_topics
            help_topics=(
                add cat cd cfg desk env help init ls mv rm set
                cfg-get cfg-ls
                desk-add desk-cat desk-cd desk-ls desk-mv desk-rm desk-set
                env-add env-cat env-cd env-ls env-rename env-rm env-set
            )
            _describe 'help topic' help_topics
            ;;
    esac
}

_tec_add() {
    _arguments \
        '(-d)'{-d,--desk}'[desk name]:desk:_tec_desks' \
        '(-D)'{-D,--description}'[provide description (generated if not provided)]' \
        '(-e)'{-e,--env}'[environment name]:env:_tec_envs' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-switch}'[do not switch to task]' \
        '(-N)'{-N,--no-switch-dir}'[neither switch to task nor to task directory]' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        '*:task ID:'
}

_tec_rm() {
    _arguments \
        '(-d)'{-d,--desk}'[desk name]:desk:_tec_desks' \
        '(-e)'{-e,--env}'[environment name]:env:_tec_envs' \
        '(-f)'{-f,--force}'[never prompt]' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-i)'{-i,--interactive}'[prompt before every removal]' \
        '(-I)'{-I,--once}'[prompt once before removing more than one task ID]' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        '(-v)'{-v,--verbose}'[explain what is being done]' \
        '*:task ID:_tec_tasks'
}

_tec_ls() {
    _arguments \
        '(-a)'{-a,--all}'[list all tasks (including done)]' \
        '(-d)'{-d,--desk}'[desk name]:desk:_tec_desks' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-H)'{-H,--headers}'[show headers]' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        '(-t)'{-t,--toggles}'[show ONLY toggle switches (current and previous)]' \
        '(-v)'{-v,--verbose}'[under development: show more verbose output]' \
        '*:environment:_tec_envs'
}

_tec_mv() {
    _arguments \
        '(-f)'{-f,--force}'[overwrite destination task (under development)]' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-t)'{-t,--target}'[move all tasks to target desk (under development)]' \
        ':source:->source' \
        ':destination:'

    case $state in
        source)
            _alternative \
                'envs:environment:_tec_envs' \
                'desks:desk:_tec_desks' \
                'tasks:task:_tec_tasks'
            ;;
    esac
}

_tec_set() {
    _arguments \
        '(-d)'{-d,--desk}'[desk name]:desk:_tec_desks' \
        '(-D)'{-D,--description}'[task description]' \
        '(-e)'{-e,--env}'[environment name]:env:_tec_envs' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-i)'{-i,--interactive}'[set unit values in interactive mode (under development)]' \
        '(-P)'{-P,--priority}'[task priority]:priority:(lowest low mid high highest)' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        '(-T)'{-T,--type}'[task type]:type:(task bugfix feature hotfix)' \
        '*:task ID:_tec_tasks'
}

_tec_cat() {
    _arguments \
        '(-d)'{-d,--desk}'[desk name]:desk:_tec_desks' \
        '(-e)'{-e,--env}'[environment name]:env:_tec_envs' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-k)'{-k,--key}'[key to show (builtin or plugin)]:key' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        '*:task ID:_tec_tasks'
}

_tec_cd() {
    _arguments \
        '(-d)'{-d,--desk}'[desk name]:desk:_tec_desks' \
        '(-e)'{-e,--env}'[environment name]:env:_tec_envs' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-update}'[do not update toggles]' \
        '(-N)'{-N,--no-switch-dir}'[neither update toggles nor switch to task directory]' \
        '(-p)'{-p,--path}'[switch into PATH inside the task directory]:path:_tec_cd_path' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        '*:task ID:_tec_tasks'
}

_tec_cfg() {
    local curcontext="$curcontext" state line
    typeset -A opt_args

    _arguments -C \
        '(-h)'{-h,--help}'[show help and exit]' \
        ':subcommand:->subcmd' \
        '*:: :->args'

    case $state in
        subcmd)
            local -a cfg_subcommands
            cfg_subcommands=(
                'get:Get config values'
                'ls:List config values'
                'set:Set config values (under development)'
                'revert:Revert config values (under development)'
                'save:Save config values into file (under development)'
            )
            _describe -t commands 'cfg subcommand' cfg_subcommands
            ;;
        args)
            case $line[1] in
                get) _tec_cfg_get ;;
                ls) _tec_cfg_ls ;;
                set) _tec_cfg_set ;;
                revert) _tec_cfg_revert ;;
                save) _tec_cfg_save ;;
            esac
            ;;
    esac
}

_tec_cfg_get() {
    _arguments \
        '*:config value:'
}

_tec_cfg_ls() {
    _arguments
}

_tec_cfg_set() {
    _arguments \
        ':config key:' \
        ':config value:'
}

_tec_cfg_revert() {
    _arguments \
        ':config key:'
}

_tec_cfg_save() {
    _arguments
}

# Desk subcommands
_tec_desk() {
    local curcontext="$curcontext" state line
    typeset -A opt_args

    _arguments -C \
        '(-h)'{-h,--help}'[show help and exit]' \
        ':subcommand:->subcmd' \
        '*:: :->args'

    case $state in
        subcmd)
            local -a desk_subcommands
            desk_subcommands=(
                'add:Add a new desk'
                'cat:Concatenate desks info'
                'cd:Switch to desk'
                'ls:List desks'
                'mv:Move (rename) desks'
                'rm:Remove desk with all tasks'
                'set:Set desks values'
            )
            _describe -t commands 'desk subcommand' desk_subcommands
            ;;
        args)
            case $line[1] in
                add) _tec_desk_add ;;
                cat) _tec_desk_cat ;;
                cd) _tec_desk_cd ;;
                ls) _tec_desk_ls ;;
                mv) _tec_desk_mv ;;
                rm) _tec_desk_rm ;;
                set) _tec_desk_set ;;
            esac
            ;;
    esac
}

_tec_desk_add() {
    _arguments \
        '(-e)'{-e,--env}'[environment name]:env:_tec_envs' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-switch}'[do not switch to newly created desk]' \
        '(-N)'{-N,--no-switch-dir}'[neither switch to task nor to desk directory]' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        '*:desk name:'
}

_tec_desk_rm() {
    _arguments \
        '(-e)'{-e,--env}'[environment name]:env:_tec_envs' \
        '(-f)'{-f,--force}'[never prompt]' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-i)'{-i,--interactive}'[prompt before every removal]' \
        '(-I)'{-I,--once}'[prompt once before removing more than one task ID]' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        '(-v)'{-v,--verbose}'[explain what is being done]' \
        '*:desk name:_tec_desks'
}

_tec_desk_ls() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        ':environment:_tec_envs'
}

_tec_desk_mv() {
    _arguments \
        '(-e)'{-e,--env}'[environment name]:env:_tec_envs' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        ':source desk:_tec_desks' \
        ':destination desk:'
}

_tec_desk_set() {
    _arguments \
        '(-D)'{-D,--description}'[task description]' \
        '(-e)'{-e,--env}'[environment name]:env:_tec_envs' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        '*:desk name:_tec_desks'
}

_tec_desk_cat() {
    _arguments \
        '(-d)'{-d,--desk}'[desk name]:desk:_tec_desks' \
        '(-e)'{-e,--env}'[environment name]:env:_tec_envs' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-k)'{-k,--key}'[key to show (builtin or plugin)]:key' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        '*:desk name:_tec_desks'
}

_tec_desk_cd() {
    _arguments \
        '(-e)'{-e,--env}'[environment name]:env:_tec_envs' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-switch}'[do not switch to task]' \
        '(-N)'{-N,--no-switch-dir}'[neither switch to task nor to desk directory]' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        '*:desk name:_tec_desks'
}

# Environments subcommands
_tec_env() {
    local curcontext="$curcontext" state line
    typeset -A opt_args

    _arguments -C \
        '(-h)'{-h,--help}'[show help and exit]' \
        ':subcommand:->subcmd' \
        '*:: :->args'

    case $state in
        subcmd)
            local -a env_subcommands
            env_subcommands=(
                'add:Add a new environment'
                'cat:Concatenate environment info'
                'cd:Switch to environment'
                'ls:List environments'
                'rename:Rename environment'
                'rm:Remove environment with all desks and tasks'
                'set:Set environment values'
            )
            _describe -t commands 'env subcommand' env_subcommands
            ;;
        args)
            case $line[1] in
                add) _tec_env_add ;;
                cat) _tec_env_cat ;;
                cd) _tec_env_cd ;;
                ls) _tec_env_ls ;;
                rename) _tec_env_rename ;;
                rm) _tec_env_rm ;;
                set) _tec_env_set ;;
            esac
            ;;
    esac
}

_tec_env_add() {
    _arguments \
        '(-d)'{-d,--desk}'[desk name (default is "desk")]:desk:_tec_desks' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-switch}'[do not switch to newly created environment]' \
        '(-N)'{-N,--no-switch-dir}'[neither switch to task nor to environment directory]' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        '*:environment name:'
}

_tec_env_rm() {
    _arguments \
        '(-f)'{-f,--force}'[never prompt]' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-i)'{-i,--interactive}'[prompt before every removal]' \
        '(-I)'{-I,--once}'[prompt once before removing more than one task ID]' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        '(-v)'{-v,--verbose}'[explain what is being done]' \
        '*:environment name:_tec_envs'
}

_tec_env_ls() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        '(-v)'{-v,--verbose}'[under development: show more info]'
}

_tec_env_rename() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        ':source environment:_tec_envs' \
        ':destination environment:'
}

_tec_env_set() {
    _arguments \
        '(-d)'{-d,--desk}'[desk name]:desk:_tec_desks' \
        '(-D)'{-D,--description}'[environment description]' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-i)'{-i,--interactive}'[set unit values in interactive mode (under development)]' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        '*:environment name:_tec_envs'
}

_tec_env_cat() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        '*:environment name:_tec_envs'
}

_tec_env_cd() {
    _arguments \
        '(-d)'{-d,--desk}'[desk name]:desk:_tec_desks' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-switch}'[do not switch to environment]' \
        '(-N)'{-N,--no-switch-dir}'[neither switch to task nor to environment directory]' \
        '(-q)'{-q,--quiet}'[do not write anything to standard error output]' \
        '*:environment name:_tec_envs'
}

# Plugin commands
#
# Discover installed plugins the same way tec itself resolves them
# (tec_cli_is_plugin() in cli/tec.c): a plugin named NAME lives at
# "<pgnbase>/NAME/NAME". Descriptions come from each plugin's own
# one-line "desc" file (see gmux/find/link/conv's plugin directories),
# falling back to a generic label if it's missing.
_tec_plugins() {
    local pgnbase name descfile desc

    plugins=()
    pgnbase=$(_tec cfg get pgnbase 2>/dev/null)
    [[ -n $pgnbase && -d $pgnbase ]] || return

    for name in $pgnbase/*(-/N:t); do
        [[ -e $pgnbase/$name/$name ]] || continue
        descfile=$pgnbase/$name/desc
        desc="tec plugin"
        [[ -f $descfile ]] && desc=$(<$descfile)
        plugins+=("$name:$desc")
    done
}

# gmux: tec's git-multiplexer plugin. Subcommands/options/descriptions
# from gmux's own `helpmsg` dict and argparse setup (gmux/gmux).
_tec_gmux() {
    local curcontext="$curcontext" state line
    typeset -A opt_args
    local -a gmux_opts

    gmux_opts=(
        '(-e --env)'{-e,--env}'[environment name]:env:_tec_envs'
        '(-d --desk)'{-d,--desk}'[desk name]:desk:_tec_desks'
        '(-T --taskbase)'{-T,--taskbase}'[task directory base]:directory:_files -/'
        '(-P --pgnbase)'{-P,--pgnbase}'[plugin directory base]:directory:_files -/'
        '(-m --message)'{-m,--message}'[commit message]:message:'
        '(-v --verbose)'{-v,--verbose}'[verbose mode]'
        '(-h --help)'{-h,--help}'[show help and exit]'
    )

    _arguments -C -s \
        $gmux_opts \
        ':subcommand:->subcmd' \
        '*:: :->args'

    case $state in
        subcmd)
            local -a gmux_subcommands
            gmux_subcommands=(
                'commit:Commit changes on task branch'
                'remove:Remove gmux unit values'
                'rsync:Update task branch with remote'
                'show:Show gmux unit values'
                'sync:Update task branch'
                'update:Merge task branch into default'
                'help:Show help message'
            )
            _describe -t commands 'gmux subcommand' gmux_subcommands
            ;;
        args)
            _arguments -s \
                $gmux_opts \
                '*:task ID:_tec_tasks'
            ;;
    esac
}

# conv: tec's workflow/kanban plugin (see tec/pgn/gmux/conv).
_tec_conv() {
    local curcontext="$curcontext" state line
    typeset -A opt_args
    local -a conv_opts

    conv_opts=(
        '(-T --taskbase)'{-T,--taskbase}'[tec taskbase directory]:directory:_files -/'
        '(-P --pgnbase)'{-P,--pgnbase}'[tec plugin directory]:directory:_files -/'
        '(-e --env)'{-e,--env}'[environment name]:env:_tec_envs'
        '(-d --desk)'{-d,--desk}'[desk name]:desk:_tec_desks'
        '--desk-path[desk directory, bypassing -T/-e/-d resolution]:directory:_files -/'
        '(-h --help)'{-h,--help}'[show help and exit]'
    )

    _arguments -C -s \
        $conv_opts \
        ':subcommand:->subcmd' \
        '*:: :->args'

    case $state in
        subcmd)
            local -a conv_subcommands
            conv_subcommands=(
                'ls:List tasks in a desk, grouped by workflow stage'
                'mv:Move a task to a workflow stage'
                'next:Move a task to its next workflow stage'
                'columns:Show the stages in the current workflow'
                'help:Show help message'
            )
            _describe -t commands 'conv subcommand' conv_subcommands
            ;;
        args)
            case $line[1] in
                ls)
                    _arguments -s \
                        $conv_opts \
                        '(-s --stage)'{-s,--stage}'[only list tasks in this stage]:stage:_conv_stages'
                    ;;
                mv)
                    _arguments -s \
                        $conv_opts \
                        ':task ID:_tec_tasks' \
                        ':stage:_conv_stages'
                    ;;
                next)
                    _arguments -s \
                        $conv_opts \
                        ':task ID:_tec_tasks'
                    ;;
                columns|help) ;;
            esac
            ;;
    esac
}

# List conv's configured workflow stages, in order, for `-s`/`mv`
# completion. Goes through `_tec conv columns` (tec's own plugin
# dispatch) rather than assuming a bare `conv` on $PATH, forwarding
# whatever -T/-e/-d/--desk-path have already been typed so far.
_conv_stages() {
    local -a stages convargs
    local taskbase="${opt_args[-T]:-$opt_args[--taskbase]}"
    local env="${opt_args[-e]:-$opt_args[--env]}"
    local desk="${opt_args[-d]:-$opt_args[--desk]}"
    local deskpath="${opt_args[--desk-path]}"

    [[ -n $taskbase ]] && convargs+=(-T $taskbase)
    [[ -n $env ]] && convargs+=(-e $env)
    [[ -n $desk ]] && convargs+=(-d $desk)
    [[ -n $deskpath ]] && convargs+=(--desk-path $deskpath)

    stages=(${(f)"$(_tec conv columns $convargs 2>/dev/null | sed -n 's/^[0-9]\+\. //p')"})
    _describe 'workflow stage' stages
}

# Helper functions
#
# Query the real `_tec` binary (not the `tec` shell wrapper, which may
# `cd` on commands like `add`/`cd`/`mv` - none of the lookups below use
# those, but going straight to the binary keeps this side-effect free
# and independent of whether the wrapper is sourced) and turn its
# aligned "ID  description" listing into "ID:description" pairs that
# _describe can render with annotations.
_tec_list() {
    _tec "$@" 2>/dev/null | awk '{
        desc = ""
        for (i = 2; i <= NF; i++) desc = desc (i > 2 ? " " : "") $i
        print $1 ":" desc
    }'
}

_tec_envs() {
    local -a envs
    envs=(${(f)"$(_tec_list env ls)"})
    _describe 'environment' envs
}

_tec_desks() {
    local -a desks
    local env="${opt_args[-e]:-$opt_args[--env]}"
    desks=(${(f)"$(_tec_list desk ls ${env:+$env})"})
    _describe 'desk' desks
}

_tec_tasks() {
    local -a tasks lsargs
    local desk="${opt_args[-d]:-$opt_args[--desk]}"
    local env="${opt_args[-e]:-$opt_args[--env]}"

    [[ -n $desk ]] && lsargs+=(-d $desk)
    [[ -n $env ]] && lsargs+=($env)

    tasks=(${(f)"$(_tec_list ls $lsargs)"})
    _describe 'task ID' tasks
}

# Read the "curr" value out of a tec toggle file ($DIR/.tec/toggles,
# "key : val" lines - see lib/unit.c's UNIT_FMT). Reading the file
# directly instead of shelling out keeps this free of hook_action side
# effects: `_tec cd` always runs the cd hook (e.g. gmux switching
# branches) regardless of -n/-N, so it can't be used just to resolve
# the current env/desk/task for completion purposes.
_tec_toggle_curr() {
    [[ -f $1 ]] && awk -F: '
        { gsub(/^[ \t]+|[ \t]+$/, "", $1) }
        $1 == "curr" { gsub(/^[ \t]+|[ \t]+$/, "", $2); print $2; exit }
    ' "$1"
}

# Resolve the on-disk directory of the target task, honoring any -e/-d
# already typed and falling back to the current env/desk/task toggles
# otherwise - the same defaulting `tec cd` itself applies at runtime.
_tec_task_dir() {
    local taskbase env desk task

    taskbase=$(_tec cfg get taskbase 2>/dev/null)
    [[ -n $taskbase ]] || return 1

    env="${opt_args[-e]:-$opt_args[--env]}"
    [[ -n $env ]] || env=$(_tec_toggle_curr "$taskbase/.tec/toggles")
    [[ -n $env ]] || return 1

    desk="${opt_args[-d]:-$opt_args[--desk]}"
    [[ -n $desk ]] || desk=$(_tec_toggle_curr "$taskbase/$env/.tec/toggles")
    [[ -n $desk ]] || return 1

    task=$(_tec_toggle_curr "$taskbase/$env/$desk/.tec/toggles")
    [[ -n $task ]] || return 1

    print -r -- "$taskbase/$env/$desk/$task"
}

# Completion for cd's -p PATH: directories inside the resolved task
# dir, falling back to plain directory completion if it can't be
# resolved (e.g. no current task set yet).
_tec_cd_path() {
    local dir=$(_tec_task_dir)

    if [[ -n $dir && -d $dir ]]; then
        _path_files -W $dir -/
    else
        _files -/
    fi
}

# Main completion function
_tec_comp() {
    local curcontext="$curcontext" state line
    typeset -A opt_args

    _arguments -C -s \
        $global_opts \
        ':subcommand:->subcmd' \
        '*:: :->args'

    case $state in
        subcmd)
            _subcommands
            _tec_plugins
            _describe -t commands 'tec command' subcommands
            _describe -t plugins 'tec plugin' plugins
            ;;
        args)
            # If no subcommand was provided (just options), we're done
            if (( $#line == 0 )); then
                return
            fi

            case $line[1] in
                help) _tec_help ;;
                init) ;;
                add) _tec_add ;;
                cat) _tec_cat ;;
                cd) _tec_cd ;;
                cfg) _tec_cfg ;;
                desk) _tec_desk ;;
                env) _tec_env ;;
                ls) _tec_ls ;;
                mv) _tec_mv ;;
                rm) _tec_rm ;;
                set) _tec_set ;;
                version) ;;
                gmux) _tec_gmux ;;
                conv) _tec_conv ;;
                *)
                    _tec_plugins
                    if (( ${plugins[(I)$line[1]:*]} )); then
                        # Known plugin with no dedicated completer: fall
                        # back to plain file completion for its args
                        # rather than erroring out.
                        _files
                    else
                        _message "unknown subcommand: $line[1]"
                    fi
                    ;;
            esac
            ;;
    esac
}

_tec_comp "$@"
