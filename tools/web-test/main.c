#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <io.h>
#include <winsock2.h>

#define REQ_ROOT	"GET / HTTP/1.1"
#define REQ_JS		"GET /index.js HTTP/1.1"
#define REQ_WASM	"GET /index.wasm HTTP/1.1"
#define REQ_DATA	"GET /data01.arc HTTP/1.1"

int main(int argc, char *argv[])
{
	char recv_buf[4096], send_buf[4096], *fname;
	WSADATA wsa;
	struct sockaddr_in name, from;
	FILE *fp;
	size_t fsize;
	char *fdata;
	int listen_sock, accept_sock, len, line_len;
	int is_index_sent, is_js_sent, is_wasm_sent, is_data_sent;

	if (WSAStartup(MAKEWORD(2, 0), &wsa) == SOCKET_ERROR) {
		printf("Error: WSAStartup\n");
		return 1;
	}

	listen_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == -1) {
		printf("Error: Cannot create a socket.\n");
		return 1;
	}

	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons(8080);
	name.sin_addr.s_addr = INADDR_ANY;

	if (bind(listen_sock, (const struct sockaddr *)&name, sizeof(name)) != 0) {
		printf("Error: Cannot bind.\n");
		return 1;
	}
	if (listen(listen_sock, 1) != 0) {
		printf("Error: Cannot listen.\n");
		return 1;
	}

	ShellExecute(NULL, "open", "http://localhost:8080/", NULL, NULL, SW_SHOWNORMAL);

	is_index_sent = 0;
	is_js_sent = 0;
	is_wasm_sent = 0;
	is_data_sent = 0;
	while (1) {
		if (is_index_sent && is_js_sent && is_wasm_sent && is_data_sent)
			break;

		len = sizeof(from);
		accept_sock = accept(listen_sock, (struct sockaddr *)&from, &len);
		if (accept_sock == -1) {
			printf("Error: Cannot accept.\n");
			return 1;
		}
		printf("Accepted.\n");

		fname = NULL;
		while (1) {
			line_len = 0;
			while (1) {
				if (recv(accept_sock, &recv_buf[line_len], 1, 0) == -1) {
					printf("Finish.\n");
					return 0;
				}
				if (recv_buf[line_len] == '\r')
					continue;
				if (recv_buf[line_len++] == '\n') {
					recv_buf[line_len] = '\0';
					break;
				}
			}
			printf("Line: %s", recv_buf);

			if (strncmp(recv_buf, REQ_ROOT, strlen(REQ_ROOT)) == 0) {
				fname = "index.html";
				snprintf(send_buf, sizeof(send_buf), "HTTP/1.1 200 OK\nContent-Type: text/html\n");
				send(accept_sock, send_buf, strlen(send_buf), 0);
				is_index_sent = 1;
			} else if (strncmp(recv_buf, REQ_JS, strlen(REQ_ROOT)) == 0) {
				fname = "index.js";
				snprintf(send_buf, sizeof(send_buf), "HTTP/1.1 200 OK\nContent-Type: text/javascript\n");
				send(accept_sock, send_buf, strlen(send_buf), 0);
				is_js_sent = 1;
			} else if (strncmp(recv_buf, REQ_WASM, strlen(REQ_ROOT)) == 0) {
				fname = "index.wasm";
				snprintf(send_buf, sizeof(send_buf), "HTTP/1.1 200 OK\nContent-Type: application/wasm\n");
				send(accept_sock, send_buf, strlen(send_buf), 0);
				is_wasm_sent = 1;
			} else if (strncmp(recv_buf, REQ_DATA, strlen(REQ_ROOT)) == 0) {
				fname = "data01.arc";
				snprintf(send_buf, sizeof(send_buf), "HTTP/1.1 200 OK\nContent-Type: application/octet-stream\n");
				send(accept_sock, send_buf, strlen(send_buf), 0);
				is_data_sent = 1;
			} else if (strcmp(recv_buf, "\n") == 0 || strcmp(recv_buf, "\r\n") == 0) {
				break;
			}
		}
		if (fname == NULL){
			printf("404 Not Found\n");
			snprintf(send_buf, sizeof(send_buf), "HTTP/1.1 404 Not Found\nContent-Size: 0\nConnection: Close\n\n");
			send(accept_sock, send_buf, strlen(send_buf), 0);
			close(accept_sock);
			continue;
		}
		printf("File: %s\n", fname);

		fp = fopen(fname, "rb");
		if (fp == NULL) {
			printf("Error: Cannot open file %s\n", fname);
			return 1;
		}
		fseek(fp, 0, SEEK_END);
		fsize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		fdata = malloc(fsize);
		if (fdata == NULL) {
			printf("Error: Cannot allocate memory.\n");
			return 1;
		}
		printf("File-size: %u\n", fsize);
		if (fread(fdata, fsize, 1, fp) != 1) {
			printf("Error: Cannot read file %s\n", fname);
			return 1;
		}
		fclose(fp);

		printf("Sending response...\n");
		snprintf(send_buf, sizeof(send_buf), "Cache-Control: no-cache\nContent-Length: %u\n\n", fsize);
		send(accept_sock, send_buf, strlen(send_buf), 0);
		send(accept_sock, fdata, fsize, 0);
		snprintf(send_buf, sizeof(send_buf), "Connection: Close\n", fsize);
		send(accept_sock, send_buf, strlen(send_buf), 0);
		close(accept_sock);

		free(fdata);
		printf("Done.\n");
	}

	printf("Exit.\n");
	return 0;
}
