// $Id: util.cpp,v 1.3 2012-04-16 12:11:55-07 - - $

/*
 B Stewart Bracken
 bbracken@ucsc.edu
 CMPS109 Spring 12
 */

#include <cstdlib>
#include <unistd.h>

using namespace std;

#include "util.h"

yshell_exn::yshell_exn (const string &what): runtime_error (what) {
}

int exit_status::status = EXIT_SUCCESS;
static string execname_string;

void exit_status::set (int new_status) {
   status = new_status;
}

int exit_status::get() {
   return status;
}

void setexecname (const string &name) {
   execname_string =  name.substr (name.rfind ('/') + 1);
   TRACE ('u', execname_string);
}

string &execname() {
   TRACE ('u', execname_string);
   return execname_string;
}

bool want_echo() {
   const int CIN_FD = 0;
   const int COUT_FD = 1;
   bool cin_isatty = isatty (CIN_FD);
   bool cout_isatty = isatty (COUT_FD);
   TRACE ('u', "cin_isatty = " << cin_isatty
          << ", cout_isatty = " << cout_isatty);
   return ! cin_isatty || ! cout_isatty;
}


wordvec split (const string &line, const string &delimiters) {
   wordvec words;
   size_t end = 0;

   // Loop over the string, splitting out words, and for each word
   // thus found, append it to the output wordvec.
   for (;;) {
      size_t start = line.find_first_not_of (delimiters, end);
      if (start == string::npos) break;
      end = line.find_first_of (delimiters, start);
      words.push_back (line.substr (start, end - start));
   }
   TRACE ('u', words);
   return words;
}

ostream &complain() {
   exit_status::set (EXIT_FAILURE);
   cerr << execname() << ": ";
   return cerr;
}

template <typename item_t>
ostream &operator<< (ostream &out, const vector<item_t> &vec) {
   typename vector<item_t>::const_iterator itor = vec.begin();
   typename vector<item_t>::const_iterator end = vec.end();

   // If the vector is empty, do nothing.
   if (itor != end) {
      // Print out the first element without a space.
      out << *itor++;
      // Print out the rest of the elements each preceded by a space.
      while (itor != end) out << " " << *itor++;
   }
   return out;
}

