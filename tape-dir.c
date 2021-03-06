/* Copyright (C) 2018 Lars Brinkhoff <lars@nocrew.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <stdio.h>
#include <string.h>

#include "dis.h"

#define MFD_OCT    0551646164416LL /* M.F.D. */
#define FILE1_OCT  0104651544511LL /* (FILE) */
#define FILE2_OCT  0164651544516LL /* .FILE. */
#define DIR_OCT    0104451621100LL /* (DIR) */

static char *tape_type (int n)
{
  if (n == 0)
    return "Random";
  else if ((n & 0700000) == 0700000)
    return "Incremental";
  else
    return "Full";
}

static void read_tape_header (FILE *f)
{
  word_t word;
  int count;
  char str[7];

  word = get_word (f);
  count = (word >> 18);
  count ^= 0400000;
  count -= 0400000;
  count = -count;
  if (count != 4)
    printf ("Count: %d\n", count);

  word = get_word (f);
  printf ("Tape: %lld\n", (word >> 18) & 0777777);
  printf ("Reel: %lld\n", word & 0777777);

  word = get_word (f);
  sixbit_to_ascii (word, str);
  printf ("Date: %s\n", str);
  
  word = get_word (f);
  printf ("Type: %s (%llo)\n", tape_type (word), word);

  word = get_word (f);
  printf ("Word: %llo\n", word);
  word = get_word (f);
  printf ("Word: %llo\n", word);
}

static int not_dir_name (word_t fn1, word_t fn2)
{
  if (fn1 == MFD_OCT && fn2 == FILE1_OCT)
    return 0;
  if (fn1 == FILE2_OCT && fn2 == DIR_OCT)
    return 0;
  return 1;
}

static void read_directory (FILE *f)
{
  word_t fn1, fn2, word;
  char ufd[7];
  char str[7];

  word = get_word (f);
  sixbit_to_ascii (word, ufd);

  for (;;)
    {
      fn1 = get_word (f);
      if (fn1 == -1)
        exit (0);
      else if (fn1 == 0)
        return;

      fn2 = get_word (f);
      word = get_word (f);
      if (not_dir_name (fn1, fn2))
        {
          sixbit_to_ascii (fn1, str);
          printf (" %s  %s ", ufd, str);
          sixbit_to_ascii (fn2, str);
          printf (" %s ", str);
          //word &= 0177777777777LL;
          print_datime (stdout, word);
          putchar ('\n');
        }
    }
}

static void read_info (FILE *f)
{
  read_tape_header (f);
  for (;;)
    read_directory (f);
}

static void
usage (const char *x)
{
  fprintf (stderr, "Usage: %s -x|-t <file>\n", x);
  exit (1);
}

int
main (int argc, char **argv)
{
  FILE *f;

  if (argc != 3)
    usage (argv[0]);

  if (argv[1][0] != '-')
    usage (argv[0]);

  switch (argv[1][1])
    {
    case 't':
      break;
    case 'x':
      break;
    default:
      usage (argv[0]);
      break;
    }

  f = fopen (argv[2], "rb");
  if (f == NULL)
    {
      fprintf (stderr, "error\n");
      exit (1);
    }

  read_info (f);

  return 0;
}
