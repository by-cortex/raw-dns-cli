#ifndef ARGS_H
#define ARGS_H

struct options {
  int show_hex;
  int timeout_sec;
  char *domain;
  char *dns_ip;
  int port;
};

int check_args(int argc, char *argv[], struct options *out_options);

#endif // ARGS_H
