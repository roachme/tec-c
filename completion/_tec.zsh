#compdef tec

# TODO:
# 1. Add support to suggest objects (env, desk, task ID)
# 2. Add support for toggles in options -C, -D, -H, etc

local -a subcommands global_opts

global_opts=(
    '(-C)'{-C,--color}'[enable colors]:toggle:(on off)'
    '(-D)'{-D,--debug}'[enable debug mode]:toggle:(on off)'
    '(-h)'{-h,--help}'[show help and exit]'
    '(-F)'{-F,--config}'[path to config file]:directory:_files'
    '(-H)'{-H,--hooks}'[enable hooks]:toggle:(on off)'
    '(-P)'{-P,--plugins}'[plugins directory]:directory:_files'
    '(-T)'{-T,--tasks}'[tasks directory]:directory:_files'
    '(-V)'{-V,--version}'[show version and exit]'
)

_subcommands() {
    subcommands=(
        'add:Add a new task to env'
        'cat:Concatenate task unit values'
        'cd:Switch to task'
        'cfg:Manage and show configs'
        'desk:Manage and show desks'
        'help:Show help for commands'
        'init:Init directory structure'
        'ls:List env tasks'
        'mv:Move (rename) tasks'
        'rm:Remove task from env'
        'set:Set task unit values'
        'column:Manage and show columns'
        'env:Manage and show environments'
    )
}

_tec_help() {
    _arguments \
        '(-d)'{-d,--desc}'[output short description for each topic]' \
        '(-s)'{-s,--synopsis}'[output only a short usage synopsis]' \
        ':command:->command'

    case $state in
        command)
            local -a help_topics
            help_topics=(${(k)_tec_subcommands} ${(k)_tec_desk_subcommands} ${(k)_tec_column_subcommands} ${(k)_tec_env_subcommands})
            _describe 'help topic' help_topics
            ;;
    esac
}

_tec_add() {
    _arguments \
        '(-b)'{-b,--desk}'[desk name]:desk:_tec_desks' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-switch}'[do not switch to task]' \
        '(-p)'{-p,--env}'[env name]:env:_tec_envs' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '(-N)'{-N,--no-switch-dir}'[neither switch to task nor to task directory]' \
        '*:task ID:'
}

_tec_rm() {
    _arguments \
        '(-b)'{-b,--desk}'[desk name]:desk:_tec_desks' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-p)'{-p,--env}'[env name]:env:_tec_envs' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '(-y)'{-y,--yes}'[remove without confirmation]' \
        '*:task ID:_tec_tasks'
}

_tec_ls() {
    _arguments \
        '(-a)'{-a,--all}'[list all tasks]' \
        '(-b)'{-b,--desk}'[desk name]:desk:_tec_desks' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '(-v)'{-v,--verbose}'[show more verbose output]' \
        '(-t)'{-t,--toggles}'[show only toggle switches]' \
        '(-H)'{-H,--headers}'[show headers]' \
        '*:env:_tec_envs'
}

_tec_mv() {
    _arguments \
        '(-d)'{-d,--destination}'[destination env]:env:_tec_envs' \
        '(-f)'{-f,--force}'[overwrite destination task]' \
        '(-s)'{-s,--source}'[source env]:env:_tec_envs' \
        '(-T)'{-T,--tasks}'[treat all IDs as source to move to env]' \
        '*:task ID:_tec_tasks'
}

_tec_set() {
    _arguments \
        '(-b)'{-b,--desk}'[desk name]:desk:_tec_desks' \
        '(-d)'{-d,--description}'[task description]' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-p)'{-p,--env}'[env name]:env:_tec_envs' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '(-t)'{-t,--type}'[task type]:type:(task bugfix feature hotfix)' \
        '(-P)'{-P,--priority}'[task priority]:priority:(lowest low mid high highest)' \
        '*:task ID:_tec_tasks'
}

_tec_cat() {
    _arguments \
        '(-b)'{-b,--desk}'[desk name]:desk:_tec_desks' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-k)'{-k,--key}'[key to show]:key' \
        '(-p)'{-p,--env}'[env name]:env:_tec_envs' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:task ID:_tec_tasks'
}

_tec_cd() {
    _arguments \
        '(-b)'{-b,--desk}'[desk name]:desk:_tec_desks' \
        '(-n)'{-n,--no-update}'[do not update toggles]' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-p)'{-p,--env}'[env name]:env:_tec_envs' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '(-N)'{-N,--no-switch-dir}'[neither update toggles nor switch to task directory]' \
        '*:task ID:_tec_tasks'
}

# Board subcommands
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
                'cat:Concatenate desk info'
                'cd:Switch to desk'
                'ls:List desks'
                'mv:Move (rename) desk'
                'rm:Remove desk with all tasks'
                'set:Set desk values'
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
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-switch}'[do not switch to new desk]' \
        '(-p)'{-p,--env}'[env name]:env:_tec_envs' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:desk name:'
}

_tec_desk_rm() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-confirm}'[remove without confirmation]' \
        '(-p)'{-p,--env}'[env name]:env:_tec_envs' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:desk name:_tec_desks'
}

_tec_desk_ls() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        ':env:_tec_envs'
}

_tec_desk_mv() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-p)'{-p,--env}'[env name]:env:_tec_envs' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        ':source desk:_tec_desks' \
        ':destination desk:'
}

_tec_desk_set() {
    _arguments \
        '(-d)'{-d,--description}'[desk description]' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-p)'{-p,--env}'[env name]:env:_tec_envs' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:desk name:_tec_desks'
}

_tec_desk_cat() {
    _arguments \
        '(-b)'{-b,--desk}'[desk name]:desk:_tec_desks' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-k)'{-k,--key}'[key to show]:key' \
        '(-p)'{-p,--env}'[env name]:env:_tec_envs' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:desk name:_tec_desks'
}

_tec_desk_cd() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-switch}'[do not switch to desk]' \
        '(-p)'{-p,--env}'[env name]:env:_tec_envs' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:desk name:_tec_desks'
}

# Column subcommands
_tec_column() {
    local curcontext="$curcontext" state line
    typeset -A opt_args

    _arguments -C \
        '(-h)'{-h,--help}'[show help and exit]' \
        ':subcommand:->subcmd' \
        '*:: :->args'

    case $state in
        subcmd)
            local -a column_subcommands
            column_subcommands=(
                'ls:List columns'
                'mv:Move task to column'
            )
            _describe -t commands 'column subcommand' column_subcommands
            ;;
        args)
            case $line[1] in
                ls) _tec_column_ls ;;
                move) _tec_column_mv ;;
            esac
            ;;
    esac
}

_tec_column_ls() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]'
}

_tec_column_mv() {
    _arguments \
        '(-b)'{-b,--desk}'[desk name]:desk:_tec_desks' \
        '(-c)'{-c,--column}'[column to move to]:column:_tec_columns' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '(-p)'{-p,--env}'[env name]:env:_tec_envs' \
        '*:task ID:_tec_tasks'
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
                'add:Add a new env'
                'cat:Concatenate env info'
                'cd:Switch to environment'
                'ls:List envs'
                'rename:Rename env'
                'rm:Remove env with all desks and tasks'
                'set:Set env values'
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
        '(-b)'{-b,--desk}'[desk name]:desk:_tec_desks' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-switch}'[do not switch to new environment]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:env name:'
}

_tec_env_rm() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-confirm}'[remove without confirmation]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:env name:_tec_envs'
}

_tec_env_ls() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '(-v)'{-v,--verbose}'[show more info]'
}

_tec_env_rename() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        ':source env:_tec_envs' \
        ':destination env:'
}

_tec_env_set() {
    _arguments \
        '(-d)'{-d,--description}'[env description]' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:env name:_tec_envs'
}

_tec_env_cat() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:env name:_tec_envs'
}

_tec_env_cd() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-switch}'[do not switch to env]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:env name:_tec_envs'
}

# Helper functions
_tec_envs() {
    local -a envs
    # This would ideally query the actual envs, but for completion we'll just provide a stub
    envs=('env1' 'env2' 'env3')
    _describe 'env' envs
}

_tec_desks() {
    local -a desks
    # Similarly, this would query the actual desks
    desks=('desk1' 'desk2' 'desk3')
    _describe 'desk' desks
}

_tec_columns() {
    local -a columns
    columns=('todo' 'in-progress' 'done' 'review')
    _describe 'column' columns
}

_tec_tasks() {
    local -a tasks
    # This would query the actual tasks
    tasks=('task1' 'task2' 'task3')
    _describe 'task' tasks
}

# Main completion function
_tec() {
    local curcontext="$curcontext" state line
    typeset -A opt_args

    _arguments -C -s \
        $global_opts \
        ':subcommand:->subcmd' \
        '*:: :->args'

    case $state in
        subcmd)
            _subcommands
            _describe -t commands 'tec command' subcommands
            ;;
        args)
            case $line[1] in
                help) _tec_help ;;
                init) ;;
                add) _tec_add ;;
                cat) _tec_cat ;;
                cd) _tec_cd ;;
                env) _tec_env ;;
                desk) _tec_desk ;;
                ls) _tec_ls ;;
                mv) _tec_mv ;;
                rm) _tec_rm ;;
                set) _tec_set ;;
                column) _tec_column ;;
            esac
            ;;
    esac
}

_tec "$@"
