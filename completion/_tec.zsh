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
        'help:Show help for commands'
        'init:Init directory structure'
        'add:Add a new task to project'
        'cat:Concatenate task unit values'
        'desk:Manage and show desks'
        'ls:List project tasks'
        'mv:Move (rename) tasks'
        'prev:Switch to previous task'
        'rm:Remove task from project'
        'set:Set task unit values'
        'cd:Switch to task'
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
            help_topics=(${(k)_tec_subcommands} ${(k)_tec_board_subcommands} ${(k)_tec_column_subcommands} ${(k)_tec_project_subcommands})
            _describe 'help topic' help_topics
            ;;
    esac
}

_tec_add() {
    _arguments \
        '(-b)'{-b,--board}'[board name]:board:_tec_boards' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-switch}'[do not switch to task]' \
        '(-p)'{-p,--project}'[project name]:project:_tec_projects' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '(-N)'{-N,--no-switch-dir}'[neither switch to task nor to task directory]' \
        '*:task ID:'
}

_tec_rm() {
    _arguments \
        '(-b)'{-b,--board}'[board name]:board:_tec_boards' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-p)'{-p,--project}'[project name]:project:_tec_projects' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '(-y)'{-y,--yes}'[remove without confirmation]' \
        '*:task ID:_tec_tasks'
}

_tec_ls() {
    _arguments \
        '(-a)'{-a,--all}'[list all tasks]' \
        '(-b)'{-b,--board}'[board name]:board:_tec_boards' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '(-v)'{-v,--verbose}'[show more verbose output]' \
        '(-t)'{-t,--toggles}'[show only toggle switches]' \
        '(-H)'{-H,--headers}'[show headers]' \
        '*:project:_tec_projects'
}

_tec_mv() {
    _arguments \
        '(-d)'{-d,--destination}'[destination project]:project:_tec_projects' \
        '(-f)'{-f,--force}'[overwrite destination task]' \
        '(-s)'{-s,--source}'[source project]:project:_tec_projects' \
        '(-T)'{-T,--tasks}'[treat all IDs as source to move to project]' \
        '*:task ID:_tec_tasks'
}

_tec_prev() {
    _arguments \
        '(-b)'{-b,--board}'[board name]:board:_tec_boards' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-p)'{-p,--project}'[project name]:project:_tec_projects' \
        '(-q)'{-q,--quiet}'[do not write to stderr]'
}

_tec_set() {
    _arguments \
        '(-b)'{-b,--board}'[board name]:board:_tec_boards' \
        '(-d)'{-d,--description}'[task description]' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-p)'{-p,--project}'[project name]:project:_tec_projects' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '(-t)'{-t,--type}'[task type]:type:(task bugfix feature hotfix)' \
        '(-P)'{-P,--priority}'[task priority]:priority:(lowest low mid high highest)' \
        '*:task ID:_tec_tasks'
}

_tec_cat() {
    _arguments \
        '(-b)'{-b,--board}'[board name]:board:_tec_boards' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-k)'{-k,--key}'[key to show]:key' \
        '(-p)'{-p,--project}'[project name]:project:_tec_projects' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:task ID:_tec_tasks'
}

_tec_cd() {
    _arguments \
        '(-b)'{-b,--board}'[board name]:board:_tec_boards' \
        '(-n)'{-n,--no-update}'[do not update toggles]' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-p)'{-p,--project}'[project name]:project:_tec_projects' \
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
                'del:Delete desk with all tasks'
                'list:List desks'
                'move:Move (rename) desk'
                'prev:Switch to previous desk'
                'set:Set desk values'
                'show:Show desk info'
                'sync:Switch to or synchronize (with) desk'
            )
            _describe -t commands 'desk subcommand' desk_subcommands
            ;;
        args)
            case $line[1] in
                add) _tec_desk_add ;;
                del) _tec_desk_del ;;
                list) _tec_desk_list ;;
                move) _tec_desk_move ;;
                prev) _tec_desk_prev ;;
                set) _tec_desk_set ;;
                show) _tec_desk_show ;;
                sync) _tec_desk_sync ;;
            esac
            ;;
    esac
}

_tec_desk_add() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-switch}'[do not switch to new board]' \
        '(-p)'{-p,--project}'[project name]:project:_tec_projects' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:board name:'
}

_tec_desk_del() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-confirm}'[delete without confirmation]' \
        '(-p)'{-p,--project}'[project name]:project:_tec_projects' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:board name:_tec_boards'
}

_tec_desk_list() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        ':project:_tec_projects'
}

_tec_desk_move() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-p)'{-p,--project}'[project name]:project:_tec_projects' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        ':source board:_tec_boards' \
        ':destination board:'
}

_tec_desk_prev() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-p)'{-p,--project}'[project name]:project:_tec_projects' \
        '(-q)'{-q,--quiet}'[do not write to stderr]'
}

_tec_desk_set() {
    _arguments \
        '(-d)'{-d,--description}'[board description]' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-p)'{-p,--project}'[project name]:project:_tec_projects' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:board name:_tec_boards'
}

_tec_desk_show() {
    _arguments \
        '(-b)'{-b,--board}'[board name]:board:_tec_boards' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-k)'{-k,--key}'[key to show]:key' \
        '(-p)'{-p,--project}'[project name]:project:_tec_projects' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:board name:_tec_boards'
}

_tec_desk_sync() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-switch}'[do not switch to board]' \
        '(-p)'{-p,--project}'[project name]:project:_tec_projects' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:board name:_tec_boards'
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
                'list:List columns'
                'move:Move task to column'
            )
            _describe -t commands 'column subcommand' column_subcommands
            ;;
        args)
            case $line[1] in
                list) _tec_column_list ;;
                move) _tec_column_move ;;
            esac
            ;;
    esac
}

_tec_column_list() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]'
}

_tec_column_move() {
    _arguments \
        '(-b)'{-b,--board}'[board name]:board:_tec_boards' \
        '(-c)'{-c,--column}'[column to move to]:column:_tec_columns' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '(-p)'{-p,--project}'[project name]:project:_tec_projects' \
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
                'add:Add a new project'
                'del:Delete project with all boards and tasks'
                'ls:List projects'
                'prev:Switch to previous project'
                'rename:Rename project'
                'set:Set project values'
                'show:Show project info'
                'sync:Switch to or synchronize (with) project'
            )
            _describe -t commands 'project subcommand' env_subcommands
            ;;
        args)
            case $line[1] in
                add) _tec_env_add ;;
                del) _tec_env_del ;;
                ls) _tec_env_ls ;;
                prev) _tec_env_prev ;;
                rename) _tec_env_rename ;;
                set) _tec_env_set ;;
                show) _tec_env_show ;;
                sync) _tec_env_sync ;;
            esac
            ;;
    esac
}

_tec_env_add() {
    _arguments \
        '(-b)'{-b,--board}'[board name]:board:_tec_boards' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-switch}'[do not switch to new environment]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:project name:'
}

_tec_env_del() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-confirm}'[delete without confirmation]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:project name:_tec_projects'
}

_tec_env_ls() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '(-v)'{-v,--verbose}'[show more info]'
}

_tec_env_prev() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]'
}

_tec_env_rename() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        ':source project:_tec_projects' \
        ':destination project:'
}

_tec_env_set() {
    _arguments \
        '(-d)'{-d,--description}'[project description]' \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:project name:_tec_projects'
}

_tec_env_show() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:project name:_tec_projects'
}

_tec_env_sync() {
    _arguments \
        '(-h)'{-h,--help}'[show help and exit]' \
        '(-n)'{-n,--no-switch}'[do not switch to project]' \
        '(-q)'{-q,--quiet}'[do not write to stderr]' \
        '*:project name:_tec_projects'
}

# Helper functions
_tec_projects() {
    local -a projects
    # This would ideally query the actual projects, but for completion we'll just provide a stub
    projects=('project1' 'project2' 'project3')
    _describe 'project' projects
}

_tec_boards() {
    local -a boards
    # Similarly, this would query the actual boards
    boards=('board1' 'board2' 'board3')
    _describe 'board' boards
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
                prev) _tec_prev ;;
                rm) _tec_rm ;;
                set) _tec_set ;;
                column) _tec_column ;;
            esac
            ;;
    esac
}

_tec "$@"
