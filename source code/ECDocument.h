#ifndef ECDOCUMENT_H
#define ECDOCUMENT_H

#include "ECTextViewImp.h"
#include "ECObserver.h"
#include "ECKeywatcher.h"

#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>

class ECKeywatcher;

class ECDocument
{
public:
    ECDocument(char * fname){
        //load file into the document
        this->fname = fname;
        std::string line;
        std::ifstream file;
        //standard file I/O 
        file.open(fname, std::ios::in);
        if (file.is_open()){
            //insert a blank character to create a new row
            doc.push_back("");
            getline(file, line);
            doc[0] = line;
            while(getline(file, line)){
                doc.push_back(line);
            }
        }
        else {
            //start the blank document if no file contents
            doc.push_back("");
        }
        file.close();
    }
    ~ECDocument(){
        //handles saving to file when deconstructing
        std::ofstream file;
        file.open(fname, std::ios::out);
        file.clear();
        //clear the file to keep things neat
        //then load the rows in 1 by 1
        if (file.is_open()){
            for (int i = 0; i < doc.size(); i++){
                file<<doc[i]<<"\n";
            }
        }
        file.close();
    }
    void AddLine(int x, int y){
        //check if a new row is needed; add if so
        if(y >= doc.size()){
            doc.push_back("");
            return;
        }
        //check if at the end of a row; simply add new blank row if so
        if(x >= doc[y].size()){
            doc.insert(doc.begin()+y+1, "");
            return;
        }
        //otherwise we split the row
        else{
            std::string curline = doc[y];
            std::string nextline = curline.substr(x);
            std::string prevline = curline.substr(0,x);
            doc[y]=prevline;
            doc.insert(doc.begin()+y+1, nextline);
            return;
        }
    }
    void AddText(int letter, int x, int y){
        //insert text, very simple
        char let = letter;
        doc[y].insert(doc[y].begin()+x, let);
            return;
    }
    int DelText(int x, int y){
        //delete text at position
        //also very simple
        int let = doc[y][x-1];
        doc[y].erase(doc[y].begin()+x-1);
        return let;
    }
    void DelRow(int y){
        //returns the x position to move the cursor to
        //moves any text from the current row to the row above
        doc[y-1] += doc[y];
        doc.erase(doc.begin()+y);
    }
    //these are used to allow the keyhandler to check up on features of the document
    //these mostly work to help make sure the cursor stays in legal positions
    //and for page implementation
    int GetLineLen(int y){
        return doc[y].size();
    }
    int GetNumLines(){
        return doc.size();
    }
    std::string GetDocLine(int y){
        return doc[y];
    }
    int size(){
        return doc.size();
    }
    //helper for finding in the find/replace system
    int NumMatchesInRow(int row, std::string word){
        int sz = word.size();
        int i = 0;
        int n = 0;
        while (i+sz <= doc[row].size()){
            if (doc[row].substr(i, sz) == word){
                n+=1;
                i+=sz;
            }
            else{
                i++;
            }
        }
        return n;
    }
    //finds the position of the nth match to the word
    //returns -1 if none found
    int FindNthMatch(int row, int n, std::string word){
        int sz = word.size();
        int occs = 0;
        int i = 0;
        while (i+sz <= doc[row].size()){
            if (doc[row].substr(i, sz) == word){
                occs+=1;
                if (occs == n){
                    return i;
                }
                i+=sz;
            }
            else{
                i++;
            }
        }
        return -1;
    }
    //replaces all occurences of a word in a row
    //returns the row as it originally was for use in undo
    //this seems more efficient and less error prone than preserving several x/y positions
    std::string ReplaceWordInRow(int row, std::string oldword, std::string newword){
        int sz = oldword.size();
        int i = 0;
        std::string oldrow = doc[row];
        while (i+sz <= doc[row].size()){
            if(doc[row].substr(i, sz) == oldword){
                //remove old word
                doc[row].erase(doc[row].begin()+i, doc[row].begin()+i+sz);
                //insert new word
                doc[row].insert(i, newword);
            }
            else{
                i++;
            }
        }
        return oldrow;
    }
    //used for undoing the replacement; simply puts the old row back
    void SwapRow(int row, std::string newrow){
        doc[row] = newrow;
    }
private:
    std::vector <std::string> doc;
    char * fname;
};

#endif