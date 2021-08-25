#ifndef ECCOMMAND_H
#define ECCOMMAND_H

#include <vector>
#include <iostream>
#include "ECDocument.h"
class ECDocument;
//keywatcher passes document pointer to commands for whatever operations are needed
//commands then operate on document when Executed from CommandHistory
//Unexecute methods are used to allow undo() operations
class Command {
public:
    virtual void Execute() = 0;
    virtual void Unexecute() = 0;
};

class CmdEnter : public Command {
public:
    CmdEnter(ECDocument * docIn, int x, int y) : docIn(docIn), y(y), x(x) {}
    //used to add a line
    virtual void Execute(){
        docIn->AddLine(x, y);
    }
    virtual void Unexecute(){
        //y is offset bc the row is techincally added at y+1
        docIn->DelRow(y+1);
    }
private:
    ECDocument * docIn;
    int x;
    int y;
};

class CmdBackspaceText : public Command {
public:
    CmdBackspaceText(ECDocument * docIn, int x, int y) : docIn(docIn), y(y), x(x) {}
    //called when backspace targets text
    virtual void Execute(){
        textDeleted = docIn->DelText(x,y);
    }
    virtual void Unexecute(){
        //similar to CMDENTER, the text removed is technically removed from x-1
        docIn->AddText(textDeleted, x-1, y);
    }
private:
    ECDocument * docIn;
    int x;
    int y;
    int textDeleted;
};

class CmdBackspaceRow : public Command {
public:
    CmdBackspaceRow(ECDocument * docIn, int x, int y) : docIn(docIn), x(x), y(y){}
    //called when backspacing at the start of a row
    //calls to perform row merging
    virtual void Execute(){
        docIn->DelRow(y);
    }
    virtual void Unexecute(){
        //pretty much is the same as pressing enter in the middle of a row
        docIn->AddLine(x,y-1);
    }
private:
    ECDocument * docIn;
    int x;
    int y;
};

class CmdAddText : public Command {
public:
    CmdAddText(ECDocument * docIn, int key, int x, int y) : docIn(docIn), key(key), x(x), y(y) {}
    //used for text insertion
    //this could not be more simple
    virtual void Execute(){
        docIn->AddText(key, x, y);
    }
    virtual void Unexecute(){
        docIn->DelText(x+1, y);
    }
private:
    ECDocument * docIn;
    int key;
    int x;
    int y;
};

class CmdReplace : public Command {
public:
    CmdReplace(ECDocument * docIn, std::string oldtext, std::string newtext, int start_y, int pgsz) 
    : docIn(docIn), oldtext(oldtext), newtext(newtext), start_y(start_y), pgsz(pgsz) {}
    //the initialization list looks very ugly but this actually fairly simple
    //performs the replace operation by using the replacewordinrow method
    //then saves the old rows
    virtual void Execute(){
        for(int i = 0; i+start_y < docIn->size() && i < pgsz; i++){
			//replace all occurences of oldtext with newtext
            oldrows.push_back(docIn->ReplaceWordInRow(i+start_y, oldtext, newtext));
		}
    }
    //uses the old rows to undo the replacement
    //if we were to use replacewordinrow, errors would occur
    //specifically that if the replacement word had already been present
    //it gets screwy
    virtual void Unexecute(){
        for(int i = 0; i+start_y < docIn->size() && i < pgsz; i++){
			//replace all occurences of newtext with oldtext
            docIn->SwapRow(i+start_y, oldrows[i]);
		}
    }
private:
    ECDocument * docIn;
    std::string newtext;
    std::string oldtext;
    std::vector<std::string> oldrows;
    int start_y;
    int pgsz;
};
//tracks the commands and executes them as desired
//this code is borrowed almost exactly from 6.1
//the only change is the two bug fixes that were discussed on piazza
class CommandHistory {
public:
    CommandHistory() : posCurCmd(-1) {};
    void Undo(){
        //checks if undo is valid
        if (posCurCmd < 0){
            return;
        }
        //uses unexecute
        listCommands[posCurCmd]->Unexecute();
        --posCurCmd;
        return;
    }
    void Redo(){
        //makes sure redo is valid
        if (posCurCmd >= (int)listCommands.size()-1){
            return;
        }
        //simply performs the command again
        ++posCurCmd;
        listCommands[posCurCmd]->Execute();
        return;
    }
    void ExecuteCmd(Command *cmd){
        //calls the command
        cmd->Execute();
        //clears any commands ahead to keep redo from becoming weird
        if(posCurCmd >= -1){
            int lstsz = listCommands.size();
            for(unsigned int i = posCurCmd+1; i<lstsz; ++i){
                delete listCommands.back();
                listCommands.pop_back();
            }
        }
        //stores the command
        listCommands.push_back(cmd);
        ++posCurCmd;
    }
    void Clear(){
        //used to 'reset' the history if need be
        listCommands.clear();
        posCurCmd = -1;
    }
private:
    std::vector<Command *> listCommands;
    int posCurCmd;
};

#endif