#include "statement.h"

statement* new_statement()
{
    statement* state = malloc(sizeof(statement));

    if (state == NULL) {
        printf("ERROR: failed to allocate a new command.\n");
        return NULL;
    }

    state->label=NULL;
    state->name = NULL;
    state->params = NULL;
    state->params_count = 0;
    state->line = 0;

    return state;
}

bool add_param(statement* state, char *str)
{
    void *new = realloc(state->params, (state->params_count + 1) * sizeof(string));
    if (new == NULL)
    {
        printf("Error while allocating memory!\n");
        return false;
    }
    state->params = new;

    new = malloc((strlen(str) + 1) * sizeof(char));
    if (new == NULL)
    {
        printf("Error while allocating memory!\n");
        return false;
    }

    strcpy(new, str); /* copy text */
    state->params[state->params_count] = new; /* add as string param */

    state->params_count++;
    return true;
}


void print_statement(statement* state)
{
    int i;

    if (state == NULL) return;

    if (state->name != NULL)
        printf("%s ", state->name);

    for (i = 0; i < state->params_count - 1; i++)
        printf("%s, ", state->params[i]);

    if (i < state->params_count) /* print the last param */
        printf("%s", state->params[i]);

    printf("\n");
}

void free_statement(statement* state)
{
    int i;

    if (state == NULL) return;

    if (state->label != NULL)
        free(state->label);

    if (state->name != NULL)
        free(state->name);

    /* each param string */
    for (i = 0; i < state->params_count; i++)
        free(state->params[i]);

    /* the params array itself */
    if (state->params != NULL)
        free(state->params);

    /* the command itself */
    free(state);
}
