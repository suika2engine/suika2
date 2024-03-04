/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

/*
 * A simple HTTP server specialized for Suika2.
 */

#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <io.h>
#include <winsock2.h>

static char req_root[1024];
static char req_js[1024];
static char req_wasm[1024];
static char req_data[1024];

static int is_index_sent;
static int is_js_sent;
static int is_wasm_sent;
static int is_data_sent;

int main(int argc, char *argv[])
{
	printf("LightHttpd/Suika2\n\n");

	/* Generate a random directory number to avoid browser cache mechanism. */
	srand((unsigned)time(NULL));
	int random_dir_number = rand();
	snprintf(req_root, sizeof(req_root), "GET /%d/ HTTP/1.1", random_dir_number);
	snprintf(req_js, sizeof(req_root), "GET /%d/index.js HTTP/1.1", random_dir_number);
	snprintf(req_wasm, sizeof(req_root), "GET /%d/index.wasm HTTP/1.1", random_dir_number);
	snprintf(req_data, sizeof(req_root), "GET /%d/data01.arc HTTP/1.1", random_dir_number);

	/* Initialize the WinSock2 DLL. */
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 0), &wsa) == SOCKET_ERROR) {
		printf("Error: WSAStartup\n");
		return 1;
	}

	/* Create a socket to listen. */
	int listen_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == -1) {
		printf("Error: Cannot create a socket.\n");
		return 1;
	}

	/* Bind the address "127.0.0.1" to the socket. */
	struct sockaddr_in name;
	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons(8080);
	name.sin_addr.s_addr = INADDR_ANY;
	if (bind(listen_sock, (const struct sockaddr *)&name, sizeof(name)) != 0) {
		printf("Error: Cannot bind.\n");
		return 1;
	}

	/* Listen. (In other words, wait for a connection.) */
	if (listen(listen_sock, 1) != 0) {
		printf("Error: Cannot listen.\n");
		return 1;
	}

	/* Open a Web browser. */
	wchar_t szURL[1024];
	swprintf(szURL, sizeof(szURL), L"http://localhost:8080/%d/", random_dir_number);
	ShellExecuteW(NULL, L"open", szURL, NULL, NULL, SW_SHOWNORMAL);

	/* Run HTTP server loop. */
	while (1) {
		/* Exit if all the files are transmitted. */
		if (is_index_sent && is_js_sent && is_wasm_sent && is_data_sent)
			break;

		/* Get a socket for a connection. */
		struct sockaddr_in from;
		int len = sizeof(from);
		int accept_sock = accept(listen_sock, (struct sockaddr *)&from, &len);
		if (accept_sock == -1) {
			printf("Error: Cannot accept.\n");
			return 1;
		}
		printf("Accepted.\n");

		/* Process lines in the request header. */
		char *fname = NULL;
		while (1) {
			char recv_buf[4096];
			int recv_len = 0;
			while (1) {
				if (recv(accept_sock, &recv_buf[recv_len], 1, 0) == -1) {
					printf("Finish.\n");
					return 0;
				}
				if (recv_buf[recv_len] == '\r')
					continue;
				if (recv_buf[recv_len++] == '\n') {
					recv_buf[recv_len] = '\0';
					break;
				}
			}
			printf("Line: %s", recv_buf);

			if (strncmp(recv_buf, req_root, strlen(req_root)) == 0) {
				fname = "index.html";
			} else if (strncmp(recv_buf, req_js, strlen(req_js)) == 0) {
				fname = "index.js";
			} else if (strncmp(recv_buf, req_wasm, strlen(req_root)) == 0) {
				fname = "index.wasm";
			} else if (strncmp(recv_buf, req_data, strlen(req_data)) == 0) {
				fname = "data01.arc";
			} else if (strcmp(recv_buf, "\n") == 0 || strcmp(recv_buf, "\r\n") == 0) {
				break;
			}
		}
		if (fname == NULL) {
			char send_buf[4096];
			printf("404 Not Found\n");
			snprintf(send_buf, sizeof(send_buf), "HTTP/1.1 404 Not Found\nContent-Size: 0\nConnection: Close\n\n");
			send(accept_sock, send_buf, strlen(send_buf), 0);
			close(accept_sock);
			continue;
		}
		printf("File: %s\n", fname);

		/* Open a file, get the size, and get the content. */
		FILE *fp = fopen(fname, "rb");
		if (fp == NULL) {
			printf("Error: Cannot open file %s\n", fname);
			return 1;
		}
		fseek(fp, 0, SEEK_END);
		size_t fsize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char *fdata = malloc(fsize);
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

		/* Send a response header.. */
		printf("Sending response header...\n");
		char send_buf[4096];
		if (strcmp(fname, "index.html") == 0) {
			snprintf(send_buf, sizeof(send_buf), "HTTP/1.1 200 OK\nContent-Type: text/html\nCache-Control: no-cache\nContent-Length: %u\n\n", fsize);
			is_index_sent = 1;
		} else if (strcmp(fname, "index.js") == 0) {
			snprintf(send_buf, sizeof(send_buf), "HTTP/1.1 200 OK\nContent-Type: text/javascript\nCache-Control: no-cache\nContent-Length: %u\n\n", fsize);
			is_js_sent = 1;
		} else if (strcmp(fname, "index.wasm") == 0) {
			snprintf(send_buf, sizeof(send_buf), "HTTP/1.1 200 OK\nContent-Type: application/wasm\nCache-Control: no-cache\nContent-Length: %u\n\n", fsize);
			is_wasm_sent = 1;
		} else if (strcmp(fname, "data01.arc") == 0) {
			snprintf(send_buf, sizeof(send_buf), "HTTP/1.1 200 OK\nContent-Type: application/octet-stream\nCache-Control: no-cache\nContent-Length: %u\n\n", fsize);
			is_data_sent = 1;
		}
		send(accept_sock, send_buf, strlen(send_buf), 0);

		/* Send a response body. */
		printf("Sending response body...\n");
		send(accept_sock, fdata, fsize, 0);
		free(fdata);

		/* Send "Connection: Close". */
		snprintf(send_buf, sizeof(send_buf), "Connection: Close\n", fsize);
		send(accept_sock, send_buf, strlen(send_buf), 0);

		close(accept_sock);
		printf("File %s Ok.\n", fname);
	}

	printf("Server finished.\n");
	return 0;
}
