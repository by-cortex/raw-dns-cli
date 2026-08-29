#include "args.h"
#include "table_print.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

int check_args(int argc, char *argv[], struct options *out_options) {
  static struct option long_options[] = {
      {"hex", no_argument, 0, 'x'},
      {"help", no_argument, 0, 'h'},
      {"timeout", required_argument, 0, 't'},
      {"port", required_argument, 0, 'p'},
      {0, 0, 0, 0},
  };

  int opt;
  while ((opt = getopt_long(argc, argv, "xhtp:", long_options, NULL)) != -1) {
    switch (opt) {
    case 'x':
      out_options->show_hex = 1;
      break;
    case 'h':
      print_help(argv[0]);
      return 1;
    case 't':
      out_options->timeout_sec = atoi(optarg);
      break;
    case 'p':
      out_options->port = atoi(optarg);
      break;
    default:
      print_help(argv[0]);
      return 1;
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "Error: Domain name is required\n");
    print_help(argv[0]);
    return 1;
  }
  if (out_options->timeout_sec <= 0) {
    fprintf(stderr, "Error: -t argument must be greater than 0\n");
    return 1;
  }
  out_options->domain = argv[optind];

  if (optind + 1 < argc) {
    out_options->dns_ip = argv[optind + 1];
  }

  return 0;
}
