#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "unit.h"

#define UNIT_DELIM        ":\n"
#define UNIT_FMT          "%s : %s\n"

/**
 * make_node() - allocate a single zero-initialized unit node
 *
 * Return: a pointer to the newly allocated node, with @key, @val and
 * @next all set to NULL, or NULL if malloc failed.
 */
static tec_unit_t *make_node(void)
{
    tec_unit_t *node;

    if ((node = malloc(sizeof(tec_unit_t))) == NULL)
        return NULL;

    node->key = node->val = NULL;
    node->next = NULL;
    return node;
}

/**
 * trim_whitespace_end() - strip trailing whitespace from a string in place
 * @str: NUL-terminated string to trim; modified in place
 *
 * Return: @str, with a new NUL written after its last non-whitespace
 * character.
 */
static char *trim_whitespace_end(char *str)
{
    char *end = str + strlen(str) - 1;

    while (end >= str && isspace(*end))
        end--;
    *++end = '\0';
    return str;
}

/**
 * trim_whitespace_beg() - skip past leading whitespace in a string
 * @str: NUL-terminated string to scan
 *
 * Return: a pointer into @str at the first non-whitespace character
 * (or at the terminating NUL if @str is all whitespace).
 */
static char *trim_whitespace_beg(char *str)
{
    while ((isspace(*str)))
        str++;
    return str;
}

/**
 * trim_whitespace() - strip both leading and trailing whitespace in place
 * @str: NUL-terminated string to trim; modified in place
 *
 * Return: a pointer to the trimmed string, which may point past the
 * start of @str (leading whitespace is skipped, not overwritten).
 */
static char *trim_whitespace(char *str)
{
    str = trim_whitespace_beg(str);
    str = trim_whitespace_end(str);
    return str;
}

/**
 * unit_save() - write a unit list to a file
 * @filename: path of the file to write
 * @units: head of the linked list of units to write, may be NULL
 *
 * Writes each "key : val" pair of @units, one per line, to a temporary
 * file ("@filename.tmp") and then renames it over @filename, so the
 * target file is never left half-written.
 *
 * Return: 0 on success; nonzero if the temporary path would not fit
 * in its buffer, the temporary file could not be opened or closed, or
 * the final rename() failed.
 */
int unit_save(const char *filename, tec_unit_t *units)
{
    FILE *fp;
    char tmpname[PATH_MAX + 5];

    if (snprintf(tmpname, sizeof(tmpname), "%s.tmp", filename) >=
        (int)sizeof(tmpname))
        return 1;

    if ((fp = fopen(tmpname, "w")) == NULL)
        return 1;

    for (tec_unit_t * unit = units; unit; unit = unit->next)
        fprintf(fp, UNIT_FMT, unit->key, unit->val);

    if (fclose(fp) != 0)
        return 1;

    return rename(tmpname, filename);
}

/**
 * unit_parse() - parse a single "key : val" line and add it to a list
 * @units: head of the list to append the parsed unit to, may be NULL
 * @str: line of text to parse, in "key : val" form
 *
 * Ignores @str if it is longer than BUFSIZ, or if a key/value pair
 * cannot be extracted from it (whitespace is trimmed from both key
 * and value).
 *
 * Return: @units with the new key/val node appended if parsing
 * succeeded, otherwise @units unchanged.
 */
tec_unit_t *unit_parse(tec_unit_t *units, const char *str)
{
    char *token;
    char key[BUFSIZ + 1] = { 0 };
    char val[BUFSIZ + 1] = { 0 };
    char buf[BUFSIZ + 1] = { 0 };

    if (strlen(str) > BUFSIZ)
        return units;

    strcpy(buf, str);

    if ((token = strtok(buf, UNIT_DELIM)) != NULL)
        strcpy(key, trim_whitespace(token));
    if ((token = strtok(NULL, UNIT_DELIM)) != NULL)
        strcpy(val, trim_whitespace(token));
    if (key[0] != '\0' && val[0] != '\0')
        units = unit_add(units, key, val);
    return units;
}

/**
 * unit_load() - read and parse a units file into a linked list
 * @filename: path of the units file to read
 *
 * Reads @filename line by line, parsing each with unit_parse().
 *
 * Return: the head of the parsed unit list (NULL if @filename has no
 * valid "key : val" lines), or NULL if @filename could not be opened.
 */
tec_unit_t *unit_load(const char *filename)
{
    FILE *fp;
    char buf[BUFSIZ + 1];
    tec_unit_t *units = NULL;

    if ((fp = fopen(filename, "r")) == NULL)
        return NULL;

    while (fgets(buf, BUFSIZ, fp) != NULL)
        units = unit_parse(units, buf);
    fclose(fp);
    return units;
}

/**
 * unit_add() - append a new key/val node to the end of a unit list
 * @head: head of the list to append to, may be NULL for an empty list
 * @key: key string, copied internally
 * @val: value string, copied internally
 *
 * Return: @head with the new node appended, or the new node itself if
 * @head was NULL. If allocating the node or copying @key/@val fails,
 * @head is returned unchanged and nothing is added.
 */
tec_unit_t *unit_add(tec_unit_t *head, const char *key, const char *val)
{
    tec_unit_t *unit, *tmp;

    if ((unit = make_node()) == NULL)
        return head;

    if (!(unit->key = strdup(key)) || !(unit->val = strdup(val))) {
        free(unit->key);
        free(unit->val);
        free(unit);
        return head;
    } else if ((tmp = head) == NULL)
        return unit;

    while (tmp->next)
        tmp = tmp->next;
    tmp->next = unit;
    return head;
}

/**
 * unit_join() - join two unit lists into one without copying data
 * @head: head of the first list, may be NULL
 * @tail: list to attach after @head's last node
 *
 * Return: @head with @tail linked onto its end, or @tail if @head was
 * NULL.
 */
tec_unit_t *unit_join(tec_unit_t *head, tec_unit_t *tail)
{
    tec_unit_t *units, *tmp;

    if (head == NULL)
        return tail;

    units = head;
    tmp = units;

    while (units->next)
        units = units->next;

    units->next = tail;
    return tmp;
}

/**
 * unit_set() - update the value for a key, or add it if not present
 * @head: head of the unit list to update, may be NULL
 * @key: key to look up
 * @val: new value string, copied internally
 *
 * Searches @head for a node whose key matches @key and replaces its
 * value; if no match is found, appends a new node via unit_add().
 *
 * Return: @head (unchanged pointer) in all cases, except that if @key
 * was not found and @head was NULL, returns the newly created node
 * from unit_add(). If duplicating @val fails for an existing key, the
 * value is left unchanged and @head is still returned.
 */
tec_unit_t *unit_set(tec_unit_t *head, const char *key, const char *val)
{
    char *newval;

    for (tec_unit_t * unit = head; unit != NULL; unit = unit->next) {
        if (strcmp(unit->key, key) == 0) {
            if ((newval = strdup(val)) == NULL)
                return head;
            free(unit->val);
            unit->val = newval;
            return head;
        }
    }
    return unit_add(head, key, val);
}

/* TODO: make return value 'const char *'.  */
/**
 * unit_get() - look up the value stored for a key
 * @head: head of the unit list to search, may be NULL
 * @key: key to look up
 *
 * Return: a pointer to the matching node's value (not a copy), or
 * NULL if no node in @head has a matching key.
 */
char *unit_get(tec_unit_t *head, const char *key)
{
    for (tec_unit_t * unit = head; unit; unit = unit->next)
        if (strcmp(unit->key, key) == 0)
            return unit->val;
    return NULL;
}

/**
 * unit_free() - free every node in a unit list
 * @head: head of the unit list to free, may be NULL
 *
 * Frees each node's key and value strings along with the node itself.
 */
void unit_free(tec_unit_t *head)
{
    tec_unit_t *curr, *next;

    for (curr = head; curr; curr = next) {
        next = curr->next;
        free(curr->key);
        free(curr->val);
        free(curr);
    }
}
