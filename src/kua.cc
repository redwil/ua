/*
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * The Original Code was developed for an EU.EDGE internal project and
 * is made available according to the terms of this license.
 * 
 * The Initial Developer of the Original Code is Istvan T. Hernadvolgyi,
 * EU.EDGE LLC.
 *
 * Portions created by EU.EDGE LLC are Copyright (C) EU.EDGE LLC.
 * All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License (the "GPL"), in which case the
 * provisions of GPL are applicable instead of those above.  If you wish
 * to allow use of your version of this file only under the terms of the
 * GPL and not to allow others to use your version of this file under the
 * License, indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by the GPL.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under either the License or the GPL.
 */

// Look for a file which is the "same" as this
//
// THE NAME kua COMES FROM Keresd UgyanAz (Hungarian for Find the Same)
//
// BUILD:
//
// g++ -O3 -o kua filei.cc kua.cc -I . 
// 
// $ kua -vh
//
// will provide help on using the program

#if !defined(__KUA_VERSION)
#define __KUA_VERSION "1.1.1"
#endif

#include <filei.h>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iomanip>

extern "C" {
#include <stdio.h>
#include <getopt.h>
}

static char __help[] = 
"kua [OPTION]... [FILE]...\n\n"
"where OPTION is\n" 
"  -f <file>:  file to compare to\n"
"  -i:         ignore case\n"
"  -w:         ignore white space\n"
"  -n:         do not ask the FS for file size\n"
"  -v:         verbose output (prints stuff to stderr), verbose help\n" 
"  -m <max>:   consider only the first <max> bytes\n"
"  -s <sep>:   separator (default SPACE)\n"
"  -p:         also print the hash value\n"
"  -b <bsize>: set internal buffer size (default 1024)\n"
"  -a <alg>:   hash algorithm: md5, sha1, sha256, b3, xxh64\n"
"  -q:         quote file names with single quotes\n"
"  -V:         print version and exit\n"
"  -h:         this help (-vh more verbose help)\n"
"  --long:     long options mirror short ones (e.g., --file, --help,\n"
"              --version, --ignore-case, --ignore-white, --no-size,\n"
"              --verbose, --max, --separator, --print-hash,\n"
"              --buffer, --algorithm, --quote)\n"
"  -           read file names from stdin\n";

static char __vhelp[] =
"kua looks for files which are identical to the one given as the argument "
"of -f. For example, \n\n"
"  $ kua -f f.txt `ls`\n\n"
"looks for files identical to f.txt in the current directory, while\n\n"
"  $ find ~ -type f | kua -f f.txt -\n\n"
"will compare f.txt to each file under home.\n"
"Blame\n\n"
"  istvan.hernadvolgyi@gmail.com\n\n";

static void __pversion() {
   std::cout << "kua version: " << __KUA_VERSION 
#if defined(__UA_USEHASH)
             << "_hash"
#else
             << "_tree"
#endif
             << std::endl;
}

static void __phelp(bool v) {
   __pversion();
   std::cout << std::endl;
   if (v) {
      std::cout << "Find files identical to the given one." 
                << std::endl << std::endl
                << __help << std::endl << __vhelp 
                << std::endl << std::endl;
   } else {
      std::cout << __help << std::endl
                << "Type kua -vh for more help. If in doubt,"
                << std::endl << std::endl
                << "$ find ... | kua -f <file> -" << std::endl;
   }
   std::cout.flush();
}

int main(int argc, char* const * argv) {

   
   std::string cfile;

   bool ic = false; // ignore case
   bool iw = false; // ignore white space
   bool v = false; // verbose
   bool ph = false; // print hash
   int BN = 1024; // buffer size
   bool count = true; // take size into account
   bool quote = false; // quote file names with single quotes
   int max = 0; // max bytes to consider
   std::string sep(" "); // separator

   bool comm = true; // from command line

   filei_hash_alg alg = filei_hash_alg::MD5;
   bool version_only = false; // print version and exit

   if (argc <= 1) {
      __phelp(false);
      return 1;
   }

   static struct option long_opts[] = {
      {"file",        required_argument, 0, 'f'},
      {"help",        no_argument,       0, 'h'},
      {"version",     no_argument,       0, 'V'},
      {"ignore-case", no_argument,       0, 'i'},
      {"ignore-white",no_argument,       0, 'w'},
      {"no-size",     no_argument,       0, 'n'},
      {"verbose",     no_argument,       0, 'v'},
      {"buffer",      required_argument, 0, 'b'},
      {"max",         required_argument, 0, 'm'},
      {"separator",   required_argument, 0, 's'},
      {"print-hash",  no_argument,       0, 'p'},
      {"algorithm",   required_argument, 0, 'a'},
      {"quote",       no_argument,       0, 'q'},
      {0,0,0,0}
   };

   int opt;
   while((opt = ::getopt_long(argc,argv,"f:hb:viwna:qVm:s:p", long_opts, nullptr)) != -1) {
      switch(opt) {
         case 'f':
            cfile = std::string(::optarg);
            break;
         case 'b':
            BN = ::atoi(::optarg);
            if (!BN) {
               std::cerr << "Invalid buffer size " << ::optarg << std::endl;
               return 1;
            }
            break;
         case 'i':
            ic = true;
            break;
         case 'v':
            v = true;
            break;
         case 'w':
            iw = true;
            break;
         case 'n':
            count = false;
            break;
         case 'm':
            max = ::atoi(::optarg);
            break;
         case 's':
            sep = std::string(::optarg);
            break;
         case 'q':
            quote = true;
            break;
         case 'p':
            ph = true;
            break;
         case 'V':
            version_only = true;
            break;
         case 'h':
            __phelp(v);
            return 0;
         case 'a':
            if (strcmp(::optarg, "md5") == 0) alg = filei_hash_alg::MD5;
            else if (strcmp(::optarg, "sha1") == 0) alg = filei_hash_alg::SHA1;
            else if (strcmp(::optarg, "sha256") == 0) alg = filei_hash_alg::SHA256;
            else if (strcmp(::optarg, "b3") == 0) alg = filei_hash_alg::BLAKE3;
            else if (strcmp(::optarg, "xxh64") == 0) alg = filei_hash_alg::XXHASH64;
            else {
               std::cerr << "Unknown algorithm: " << ::optarg << std::endl;
               return 1;
            }
            break;
         case '?':
            std::cerr << "Type " << argv[0] << " -h for options." << std::endl;
            return 1;
      }
   }

   if (version_only) {
      __pversion();
      return 0;
   }

   if (!cfile.size()) {
      std::cerr << "File param missing. See kua -vh" << std::endl;
      return 1;
   }

   if (count && iw) count = false;

   if (argc > ::optind) { 
      if (argc >= ::optind +1 && *argv[::optind] == '-') {
         if (argc > ::optind + 1) {
            std::cerr << "Spurious arguments!" << std::endl;
            return 1;
         }
         ++optind;
         comm = false; // read files names from stdin
      }
   }

   char fileb[FILENAME_MAX];

   off_t n = 0;

   if (count) {
      try {
         n = filei::fsize(cfile);
      } catch(const char *e) {
         std::cerr << e << std::endl;
      }
   }

   std::string hash_hex;
   if (ph) {
      try {
         filei ref(cfile, ic, iw, max, BN, alg);
         std::ostringstream oss;
         oss << std::hex;
         for(int i=0; i<ref.hash_len(); ++i) {
            int hi = ref[i] >> 4 & 0x0f;
            int lo = ref[i] & 0x0f;
            oss << hi << lo;
         }
         hash_hex = oss.str();
      } catch(const char* e) {
         std::cerr << "Could not hash " << cfile << ": " << e << std::endl;
         return 1;
      }
   }

   for(int i = ::optind;;) {
      char* file;
      if (comm) {
         if (i == argc) break;
         file = argv[i++];
      } else {
         std::cin.getline(fileb,1024);
         if (std::cin.eof()) break;
         file = fileb;
      }


      if (v) std::cerr << "Considering " << file << std::endl;
      try {
         if (count) {
            const off_t sz = filei::fsize(file);
            if (n != sz) continue;
         }
         if (filei::eq(cfile,file,ic,iw,max,BN,alg)) {
            if (ph) {
               std::cout << hash_hex << sep;
            }
            if (quote) std::cout << "'";
            std::cout << file;
            if (quote) std::cout << "'";
            std::cout << std::endl;
         }
      } catch(const char* e) {
         if (v) std::cerr << "Skipping " << file << ", " << e << std::endl;
         continue;
      }
   }


   return 0;

}
