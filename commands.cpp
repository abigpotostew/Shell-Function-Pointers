// $Id: commands.cpp,v 1.120 2012-04-16 12:19:07-07 - - $

/*
 B Stewart Bracken
 bbracken@ucsc.edu
 CMPS109 Spring 12
 */

#include "commands.h"
#include "trace.h"
#include <string.h>

commands::commands(): map (commandmap()) {
   map["cat"    ] = fn_cat    ;
   map["cd"     ] = fn_cd     ;
   map["echo"   ] = fn_echo   ;
   map["exit"   ] = fn_exit   ;
   map["ls"     ] = fn_ls     ;
   map["lsr"    ] = fn_lsr    ;
   map["make"   ] = fn_make   ;
   map["mkdir"  ] = fn_mkdir  ;
   map["prompt" ] = fn_prompt ;
   map["pwd"    ] = fn_pwd    ;
   map["rm"     ] = fn_rm     ;
   map["rmr"    ] = fn_rmr    ;
}

function commands::operator[] (const string& cmd) {
   return map[cmd];
}

//Private inner functions:
inode* navigate_path(inode_state& state, const wordvec &words,
                     int path_index, int limitBy);
void lsr_recursive(inode* current_inode, string path);
void rmr_recursive(inode* current_inode);


//Command public functions:m
void fn_cat (inode_state &state, const wordvec &words){
   TRACE ('c', state);
   TRACE ('c', words);
   
   if ( words.size() == 1 ){
      cout << ": No such file or directory" << endl;
   }else{
      wordvec_itor itor = words.begin() + 1;
      const wordvec_itor end = words.end();
      for (int i = 1; itor != end; ++itor, ++i){
         inode* target = navigate_path(state, words, i, 0);
         if ( target != NULL && !target->is_dir() ){
            cout << target->readfile() << endl;
            //cout <<  << endl;
         } else{
            cout << "cat: " << words[i] << 
            ": No such file or directory" << endl;
         }
      }
   }
}

void fn_cd (inode_state &state, const wordvec &words){
   TRACE ('c', state);
   TRACE ('c', words);
   string error;
   if ( words.size() == 1 ){
      state.set_cwd_to_root();
   }
   else if ( words.size() == 2){
      inode* target = navigate_path(state, words, 1, 0);
      if ( target == NULL ) {
         error += "cd: " + words[1] + 
            ": No such file or directory";
         TRACE ('c', "Invalid directory path");
      }else{
         state.set_cwd(target);
         TRACE ('c', "Setting cwd to path");
      }
   }else {
      error += "cd:";
      wordvec_itor itor = words.begin() + 1;
      while ( itor != words.end() ) error += " " + *itor++;
      error += ": No such file or directory";
   }
   if ( error.size() > 0)
      throw yshell_exn(error);
}

void fn_echo (inode_state &state, const wordvec &words){
   TRACE ('c', state);
   TRACE ('c', words);
   
   string output("");
   if ( words.size() > 1 ){
      wordvec_itor itor = words.begin() + 1;
      const wordvec_itor end = words.end();
      for (; itor != end; ++itor) {
         output += *itor;
         output += " ";
      }
      output.erase(output.end()-1);
   }
   cout << output << endl;
}

void fn_exit (inode_state &state, const wordvec &words){
   TRACE ('c', state);
   TRACE ('c', words);
   int code;
   if ( words.size() > 1 ){
      code = atoi(words[1].c_str());
      exit_status::set(code);
   }
   throw ysh_exit_exn();
}

void fn_ls (inode_state &state, const wordvec &words){
   TRACE ('c', state);
   TRACE ('c', words);
   
   if ( words.size() == 1 ){
      //list this directory
      inode* cwd = state.getcwd();
      cout << cwd->name() << ":\n" << cwd << "  " << endl;
   }
   else{
      inode* target;
      wordvec_itor itor = words.begin();
      ++itor;
      wordvec_itor end = words.end();
      for (int i = 1; itor != end; ++itor, ++i){
         target = navigate_path(state, words, i, 0);
         if ( target == NULL ) 
            cerr << "ls: Invalid file or directory: " 
               << words[i] << endl;
         else
            cout << words[i] << ":" << "\n" << target << "  " << endl;
      }
   }
}

void fn_lsr (inode_state &state, const wordvec &words){
   TRACE ('c', state);
   TRACE ('c', words);
   string error("");
   
   inode* head;
   if ( words.size() == 1 ){
      head = state.getcwd();
      string n;
      lsr_recursive(head, n);
   }else {
      wordvec_itor itor = words.begin();
      ++itor; //don't use first slot with 'lsr'
      wordvec_itor end = words.end();
      for (int i = 1; itor!=end; ++itor, ++i){
         head = navigate_path(state, words, i, 0);
         if ( head != NULL ){
            string n;
            lsr_recursive(head, n);
         }else {
            cout << "lsr: No such file or directory: "
               << words[i] << endl;
         }
      }
   }
}

void fn_make (inode_state &state, const wordvec &words){
   TRACE ('c', state);
   TRACE ('c', words);
   string error("");
   if ( words.size() == 1 )
      error += "make: Please specify a filename";
   else {
      inode* target_parent = navigate_path(state, words, 1, 1);
      if ( target_parent != NULL ){
         bool newfile = false;
         TRACE ('c', "Attempting to add a new file: " << words
                << "\nWith target_parent:" << target_parent);
         wordvec path = split(words[1], "/" );
         string dirname(path[path.size()-1]);
         inode* targetfile = target_parent->get_child_dir(dirname);
         //Create a new file if it doesn't exist
         if ( targetfile == NULL ){
            targetfile = new inode(FILE_INODE);
            newfile = true;
            TRACE ('c', "Created a new file in: " << path);
         }
         if ( !targetfile->is_dir() ){
            targetfile->writefile(words);
            if ( newfile == true ){
               string name(words[1]);
               target_parent->add_file(dirname, targetfile);
               TRACE ('c', "Adding new file to directory...");
            }
         }else
            error += "make: Directory already exists with name: " + 
               words[1];
      }else {
         error += "make: Invalid file or directory";
      }
   }
   if ( error.size() > 0 )
      throw yshell_exn(error);
}

void fn_mkdir (inode_state &state, const wordvec &words){
   TRACE ('c', state);
   TRACE ('c', words);
   string error("");
   if ( words.size() == 1 || words.size() > 2 ){
      error += "mkdir: Invalid directory name";
      //cout <<  << endl;
   }
   else if ( words.size() == 2){
      inode* target = navigate_path(state, words, 1, 1);
      if ( target != NULL ){
         //check if the directory already exists
         wordvec path = split(words[1],"/");
         if ( target->get_child_dir(path[path.size()-1]) != NULL )
            error += "mkdir: Directory already exists";
         else {
            //make a new directory here
            inode* newDir = new inode(DIR_INODE);
            target->add_dirent(path[path.size()-1], newDir);
         }
      }else {
         error += "mkdir: Invalid path";
      }
   }
   if ( error.size() > 0 )
      throw yshell_exn(error);
}

void fn_prompt (inode_state &state, const wordvec &words){
   TRACE ('c', state);
   TRACE ('c', words);
   
   //grabs all words in the wordvect and consolodates into a single
   //space separated string.
   string newprompt;
   if ( words.size() > 1 ){
      wordvec_itor itor = words.begin() + 1;
      const wordvec_itor end = words.end();
      for (; itor != end; ++itor) {
         newprompt += *itor;
         newprompt += " ";
      }
      newprompt.erase(newprompt.end()-1);
   }
   state.setprompt(newprompt);
}

void fn_pwd (inode_state &state, const wordvec &words){
   TRACE ('c', state);
   TRACE ('c', words);
   string path("");
   path += state.getcwd()->name();
   inode* parent = state.getcwd()->get_parent();
   while ( parent->get_inode_nr() !=
           state.getroot()->get_inode_nr() ){
      path = parent->name() + "/" + path;
      parent = parent->get_parent();
   }
   if ( path.compare("/") != 0)
      path = "/" + path;
   cout << path << endl; 
}

void fn_rm (inode_state &state, const wordvec &words){
   TRACE ('c', state);
   TRACE ('c', words);
   
   string error("");
   if ( words.size() == 1 || words.size() > 2 ){
      error += "rm: Invalid file or directory";
   }
   else if ( words.size() == 2){
      inode* target = navigate_path(state, words, 1, 0);
      if ( target != NULL ){
         if ( ( target->is_dir() && target->size() <= 2 ) || 
             !target->is_dir() ){
            inode* target_parent = navigate_path(state, words, 1, 1);
            wordvec path = split(words[1],"/");
            if ( target_parent->delete_child( path[path.size()-1] )
                == false ){
               TRACE ('c', "Cannot delete file for unknown reason" );
               error += "rm: Cannot delete '.' or '..'";
            }else 
               TRACE ('c', "Successfully deleted child from parents" );
         }else {
            error += "rm: Cannot delete a non-empty directory";
         }
      }else {
         error += "rm: Invalid file or directory";
      }
   }
   if ( error.size() > 0 )
      throw yshell_exn(error);
}

void fn_rmr (inode_state &state, const wordvec &words){
   TRACE ('c', state);
   TRACE ('c', words);
   
   string error("");
   if ( words.size() == 1 || words.size() > 2){
      error += "rmr: Invalid file or directory";
   }
   else if ( words.size() == 2){
      inode* target_head = navigate_path(state, words, 1, 0);
      inode* parent = navigate_path(state, words, 1, 1);
      TRACE('c', "Recursivly deleting subdirectories starting with: "
         << target_head);
      if ( target_head->is_dir() ){
         rmr_recursive(target_head);
      }
      if ( parent->get_inode_nr() != target_head->get_inode_nr() )
         parent->delete_child(target_head->name());
   }
   if ( error.size() > 0 )
      throw yshell_exn(error);
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

inode* navigate_path(inode_state& state, const wordvec &words,
                     int path_index, int limitBy){
   bool start_at_root = false;
   wordvec path = split(words[path_index],"/");
   if (words[path_index].at(0)=='/')
      start_at_root = true;
   TRACE ('c', "navigate_path(): Navigating path: " << path );
   
   inode* head;
   if ( start_at_root == true ){
      head = state.getroot();
   } else
      head = state.getcwd();
   
   int size = path.size()-limitBy;
   for ( int i = 0; i < size; ++i ){
      if ( head->is_dir() ){
         TRACE ('c', "navigate_path(): head is a directory" << path );
         head = head->get_child_dir(path[i]);
      } else{
         head = NULL;
         TRACE ('c', "navigate_path(): Failed, attempted to navigate \
                  path to a file: "
                  << path );
      }
      if ( head == NULL )
         return NULL;
   }
   return head;
}

//pre-condition: current_inode is a directory
void lsr_recursive(inode* current_inode, string path){
   const string name = current_inode->name();
   path += current_inode->name();
   cout << path
   << ":\n" << current_inode << endl;
   directory* directory = current_inode->get_dirents();
   directory::const_iterator itor = directory->begin();
   //Don't even consider the . and .. directories.
   //Don't even think about it!
   ++itor; ++itor;
   directory::const_iterator end = directory->end();
   if ( path.compare("/") != 0 )
      path += "/";
   for (; itor != end; ++itor) {
      if ( itor->second->is_dir() ){
         lsr_recursive(itor->second, path);
      }
   }
}

//Pre: current_inode is a directory inode
void rmr_recursive(inode* current_inode){
   directory* directory = current_inode->get_dirents();
   directory::iterator itor = directory->begin();
   ++itor; ++itor;
   directory::const_iterator end = directory->end();
   for (; itor != end;) {
      if ( itor->second->is_dir() )
         rmr_recursive(itor->second);
      directory->erase(itor++);
   }
   
}

