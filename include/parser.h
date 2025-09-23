#ifndef PARSER_H
#define PARSER_H

typedef struct command {
    char **argv;               // NULL-terminated argv
    char *infile;              // "< file"
    char *outfile;             // "> / >> file"
    int   append;              // 1 if >>
    int   background;          // last pipeline runs in background
    struct command *next_pipe; // next in pipeline
} command_t;

command_t *parse_line(const char *line);
void       free_command(command_t *cmd);

#endif
