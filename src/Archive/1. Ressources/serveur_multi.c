/* serveur_multi.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#define PORT 5001

int sockfd;

void closeSocket(int s) {
    (void)s;
    close(sockfd);
    exit(0);
}

int parse_csv(char *line, char **fields, int max) {
    int count = 0, i = 0, start = 0;

    while (line[i] && count < max) {
        if (line[i] == ';' || line[i] == '\0') {
            fields[count++] = (i - start > 0) ? strndup(line + start, i - start) : strdup("");
            start = i + 1;
        }
        if (line[i] == '\0') break;
        i++;
    }
    return count;
}

void traiter_client(int sock) {
    char classe[50], filename[256], line[512], buf[256];
    FILE *file;
    int count = 0;

    read(sock, classe, sizeof(classe)-1);
    classe[strcspn(classe, "\n")] = 0;

    snprintf(filename, sizeof(filename), "%s.csv", classe);
    file = fopen(filename, "r");

    if (!file) {
        write(sock, "ERREUR: Classe introuvable\n", 28);
        return;
    }

    fgets(line, sizeof(line), file); // skip header

    FILE *tmp = fopen(filename, "r");
    fgets(line, sizeof(line), tmp);
    while (fgets(line, sizeof(line), tmp)) count++;
    fclose(tmp);

    snprintf(buf, sizeof(buf), "%d\n", count);
    write(sock, buf, strlen(buf));

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0;
        char *fields[10];
        int n = parse_csv(line, fields, 10);

        if (n >= 7) {
            snprintf(buf, sizeof(buf), "%s %s\n", fields[6], fields[4]);
            write(sock, buf, strlen(buf));
            for (int i = 0; i < n; i++) free(fields[i]);
        }
    }
    fclose(file);
}

int main() {
    struct sockaddr_in addr;
    signal(SIGINT, closeSocket);
    signal(SIGCHLD, SIG_IGN);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    listen(sockfd, 5);

    printf("Serveur sur port %d\n", PORT);

    while (1) {
        int client = accept(sockfd, NULL, NULL);

        if (fork() == 0) {
            close(sockfd);
            traiter_client(client);
            close(client);
            exit(0);
        }
        close(client);
    }
    return 0;
}
