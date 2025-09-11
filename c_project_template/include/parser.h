#ifndef PARSER_H
#define PARSER_H
typedef struct command {
  char **argv; char *infile; char *outfile; int append; int background;
  struct command *next_pipe;
} command_t;
command_t *parse_line(const char *line);
void free_command(command_t *cmd);
#endif
