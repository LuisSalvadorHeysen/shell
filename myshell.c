#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 512
#define MAX_ARGS 100

typedef struct {
    char *cmd;
    char *out;
    int advanced;
} cmd_data;

void print(char *msg) {
    write(STDOUT_FILENO, msg, strlen(msg)); 
}

void error() {
    char error_message[30] = "An error has occurred\n";
    print(error_message);
}

int is_empty(char *s) {
    while (*s)
    {
        if (*s != ' ' && *s != '\t')
        {
            return 0;
        }
        ++s;
    }
    return 1;
}

char *read_and_validate_input(FILE *input, int batch_mode) {
    char *s = malloc(MAX_INPUT_SIZE + 2);

    if (batch_mode) {
        int sz = 0;
        int capacity = MAX_INPUT_SIZE + 2;
        char *full_line = malloc(capacity);
        int c;

        while ((c = fgetc(input)) != EOF && c != '\n') {
            if (sz + 1 >= capacity) {
                capacity *= 2;
                char *new_buf = realloc(full_line, capacity);
                if (!new_buf) {
                    free(full_line);
                    free(s);
                    error();
                    return NULL;
                }
                full_line = new_buf;
            }
            full_line[sz++] = c;
        }

        if (c == '\n') {
            full_line[sz++] = c;
        }
        full_line[sz] = '\0';

        char *tmp = strdup(full_line);
        if (tmp[strlen(tmp) - 1] == '\n') {
            tmp[strlen(tmp) - 1] = '\0';
        }

        if (!is_empty(tmp)) {
            print(full_line);
        }
        free(tmp);

        if (sz > MAX_INPUT_SIZE + 1) {
            error();
            free(full_line);
            free(s);
            return NULL;
        }

        strcpy(s, full_line);
        free(full_line);
    }
    else {
        char *res = fgets(s, MAX_INPUT_SIZE + 2, input);
        if (!res) {
            free(s);
            return NULL;
        }
    }

    int len = strlen(s);
    if (len == 0 || (len == 1 && s[0] == '\n') || is_empty(s)) {
        free(s);
        return NULL;
    }

    if (s[len - 1] == '\n') {
        s[--len] = '\0';
    }

    return s;
}

char **get_tokens(char *s) {
    char **tokens = malloc(MAX_ARGS * sizeof(char *));
    if (!tokens) {
        error();
        return NULL;
    }

    int len = 0;
    char *curr_token = strtok(s, " \t");
    while (curr_token != NULL) {
        if (len + 1 == MAX_ARGS) {
            error();
            return NULL;
        }
        tokens[len++] = curr_token;
        curr_token = strtok(NULL, " \t");
    }
    tokens[len] = NULL;
    return tokens;
}

int is_exit(char *s, int *skip) {
    while (*s == ' ' || *s == '\t') {
        ++s;
    }

    if (strncmp(s, "exit", 4) != 0) {
        return 0;
    }

    s += 4;

    if (*s == '\0' || is_empty(s)) {
        return 1;
    }

    if (*s != ' ' && *s != '\t') {
        return 0;
    }

    else {
        *skip = 1;
        return 1;
    }
}

int is_cd(char *s, char **dir_path, int *skip)
{
    while (*s == ' ' || *s == '\t') {
        ++s;
    }

    if (strncmp(s, "cd", 2) != 0) {
        return 0;
    }

    s += 2;
    if (*s == '\0' || is_empty(s)) {
        *dir_path = NULL;
        return 1;
    }

    if (*s != ' ' && *s != '\t') {
        return 0;
    }

    while (*s == ' ' || *s == '\t') {
        ++s;
    }

    char *tmp = malloc(strlen(s) * sizeof(char));
    strcpy(tmp, s);
    *dir_path = strtok(tmp, " \t");

    int idx = strlen(s) - 1;
    while (s[idx] == ' ' || s[idx] == '\t') {
        --idx;
    }

    ++idx;
    char *tmp1 = malloc((idx + 1) * sizeof(char));
    memcpy(tmp1, s, idx);
    tmp1[idx] = '\0';

    if (strcmp(*dir_path, tmp1) != 0) {
        *skip = 1;
    }

    return 1;
}

void run_cd(char *dir_path) {
    if (dir_path == NULL) {
        dir_path = getenv("HOME");
    }

    if (chdir(dir_path) != 0) {
        error();
    }
}

int is_pwd(char *s, int *skip) {
    while (*s == ' ' || *s == '\t') {
        ++s;
    }

    if (strncmp(s, "pwd", 3) != 0) {
        return 0;
    }

    s += 3;

    if (*s == '\0' || is_empty(s)) {
        return 1;
    }

    if (*s != ' ' && *s != '\t') {
        return 0;
    }

    else {
        *skip = 1;
        return 1;
    }
}

void run_pwd() {
    char *pwd = getcwd(NULL, 0);
    if (pwd != NULL) {
        print(pwd);
        print("\n");
        free(pwd);
    }

    else {
        error();
    }
}

void run_one_command(char *s) {
    char *dir_path = NULL;
    int skip = 0;

    if (is_exit(s, &skip)) {
        if (skip) {
            error();
            return;
        }

        exit(0);
    }
    else if (is_cd(s, &dir_path, &skip)) {
        if (skip) {
            error();
        }

        else {
            run_cd(dir_path);
        }
    }

    else if (is_pwd(s, &skip)) {
        if (skip) {
            error();
        }

        else {
            run_pwd();
        }
    }

    else {
        char **tokens = get_tokens(s);
        if (tokens == NULL) {
            error();
            return;
        }

        int pid = fork();
        if (pid < 0) {
            error();
            free(tokens);
            return;
        }

        if (pid == 0) {
            execvp(tokens[0], tokens);
            free(tokens);
            error();
            _exit(1);
        }

        else {
            int status;
            waitpid(pid, &status, 0);
            free(tokens);
        }
    }
}

cmd_data *pack_command_data(char *s) {
    cmd_data *curr_data = malloc(sizeof(*curr_data));
    curr_data->cmd = NULL;
    curr_data->out = NULL;
    curr_data->advanced = 0;

    char *operand = strstr(s, ">");
    if (operand == NULL) {
        curr_data->cmd = strdup(s);
        return curr_data;
    }

    if (*(operand + 1) == '+') {
        curr_data->advanced = 1;
    }

    *operand = '\0';
    curr_data->cmd = strdup(s);
    *operand = '>';

    operand += 1 + curr_data->advanced;
    while (*operand == ' ' || *operand == '\t') {
        ++operand;
    }

    if (*operand == '\0') {
        free(curr_data->cmd);
        free(curr_data);
        return NULL;
    }

    char *r = operand;
    while (*r != ' ' && *r != '\t' && *r != '\0') {
        ++r;
    }

    char *tmp = r;
    while (*tmp == ' ' || *tmp == '\t') {
        ++tmp;
    }

    if (*tmp != '\0') {
        free(curr_data->cmd);
        free(curr_data);
        return NULL;
    }

    char savechar = *r;
    *r = '\0';
    curr_data->out = strdup(operand);
    *r = savechar;

    return curr_data;
}

int is_custom(char *s, int *skip) {
    char **tmp = malloc(sizeof(char **));
    if (is_cd(s, tmp, skip))
        return 1;
    if (is_pwd(s, skip))
        return 1;
    if (is_exit(s, skip))
        return 1;
    free(tmp);
    return 0;
}

void run_with_redirect(char *s) {
    cmd_data *pack = pack_command_data(s);
    if (!pack) {
        error();
        return;
    }

    if (!pack->out) {
        run_one_command(pack->cmd);
        free(pack->cmd);
        free(pack);
        return;
    }

    int skip = 0;
    if (is_custom(pack->cmd, &skip)) {
        error();
        free(pack->cmd);
        free(pack->out);
        free(pack);
        return;
    }

    if (skip) {
        error();
        free(pack->cmd);
        free(pack->out);
        free(pack);
        return;
    }

    if (!pack->advanced && access(pack->out, F_OK) == 0) {
        error();
        free(pack->cmd);
        free(pack->out);
        free(pack);
        return;
    }

    char tmp_file[25];
    int tmp_fd = -1;

    char *old_content = NULL;
    int old_size = 0;
    if (pack->advanced && access(pack->out, F_OK) == 0) {
        int fd = open(pack->out, O_RDONLY);
        if (fd == -1) {
            error();
            free(pack->cmd);
            free(pack->out);
            free(pack);
            return;
        }

        old_size = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        old_content = malloc(old_size);
        if (read(fd, old_content, old_size) != old_size) {
            error();
            free(old_content);
            close(fd);
            free(pack->cmd);
            free(pack->out);
            free(pack);
            return;
        }
        close(fd);

        strcpy(tmp_file, "/tmp/myshell-XXXXXX");
        tmp_fd = mkstemp(tmp_file);
        if (tmp_fd == -1) {
            error();
            free(old_content);
            free(pack->cmd);
            free(pack->out);
            free(pack);
            return;
        }
    }

    int pid = fork();
    if (pid == 0) {
        int fd;
        if (tmp_fd != -1) {
            fd = tmp_fd;
        }

        else {
            fd = open(pack->out, O_WRONLY | O_CREAT | O_TRUNC, 0664);
            if (fd < 0) {
                error();
                _exit(1);
            }
        }

        if (dup2(fd, STDOUT_FILENO) == -1) {
            error();
            close(fd);
            _exit(1);
        }
        close(fd);

        char **tokens = get_tokens(pack->cmd);
        execvp(tokens[0], tokens);

        error();
        _exit(1);
    }

    else {
        waitpid(pid, NULL, 0);

        if (tmp_fd != -1) {
            int write_fd = open(pack->out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (write_fd == -1) {
                error();
                free(old_content);
                close(tmp_fd);
                unlink(tmp_file);
                free(pack->cmd);
                free(pack->out);
                free(pack);
                return;
            }

            char buf[4096];
            int bytes_read;

            lseek(tmp_fd, 0, SEEK_SET);
            while ((bytes_read = read(tmp_fd, buf, sizeof(buf))) > 0) {
                write(write_fd, buf, bytes_read);
            }

            write(write_fd, old_content, old_size);

            close(write_fd);
            close(tmp_fd);
            unlink(tmp_file);
            free(old_content);
        }
    }

    free(pack->cmd);
    free(pack->out);
    free(pack);
}

void run_multiple(char *s) {
    char *right = s;

    while (1) {
        while (*right != '\0' && *right != ';') {
            ++right;
        }

        int len = right - s;
        char *curr = malloc(len + 1);
        memcpy(curr, s, len);
        curr[len] = '\0';

        if (!is_empty(curr)) {
            run_with_redirect(curr);
        }

        if (*right == '\0') {
            break;
        }

        ++right;
        s = right;
    }
}

void run_shell(FILE *input, int batch_mode) {
    while (1) {
        if (!batch_mode) {
            print("myshell> ");
        }

        char *cmd_buff = read_and_validate_input(input, batch_mode);
        if (!cmd_buff) {
            if (feof(input)) {
                break;
            }
            continue;
        }

        run_multiple(cmd_buff);
        free(cmd_buff);
    }
}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        error();
        exit(1);
    }

    if (argc == 1) {
        run_shell(stdin, 0);
    }

    else {
        FILE *batch_file = fopen(argv[1], "r");
        if (!batch_file) {
            error();
            exit(1);
        }

        run_shell(batch_file, 1);
        fclose(batch_file);
    }

    return 0;
}
