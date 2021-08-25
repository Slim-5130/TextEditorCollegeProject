#ifndef ECKEYWATCHER_H
#define ECKEYWATCHER_H

#include "ECTextViewImp.h"
#include "ECObserver.h"
#include "ECDocument.h"
#include "ECCommand.h"

#include <vector>
#include <string>
#include <algorithm>

class ECKeywatcher : public ECObserver
{
public:
    ECKeywatcher( ECTextViewImp * pSub, ECDocument * pDoc) : pSub(pSub), pDoc(pDoc){
		this->pSub->Attach(this);
		page = 0;
		mode = 0;
		Update();
		history.Clear();
		//this is necessary bc for some reason the file I/O inserts a weird character
		//this deletes it before the user can touch it
		pDoc->DelText(pSub->GetCursorX(), pSub->GetCursorY());
		pSub->SetCursorX(0);
        UpdateDisplay();
	}
    virtual void Update(){
        int key = pSub->GetPressedKey();
		int offset = page*pSub->GetRowNumInView();
        switch(mode){
            case 0 :{
            //editor mode
                switch(key){
                    case ENTER:{
                    //run the newline command and update the cursor
                        CmdEnter * cmd = new CmdEnter(pDoc, pSub->GetCursorX(), pSub->GetCursorY()+offset);
                        history.ExecuteCmd(cmd);
                        pSub->SetCursorY(pSub->GetCursorY()+1);
                        pSub->SetCursorX(0);
                        break;}
                    case ARROW_RIGHT:{
                    //move either one position to the right or to the next row
                        if (pDoc->GetLineLen(pSub->GetCursorY()+offset) >= pSub->GetCursorX()+1){
					        pSub->SetCursorX(pSub->GetCursorX()+1);
				        }
				        else if (pDoc->size() > pSub->GetCursorY()+offset+1){
					        pSub->SetCursorX(0);
					        pSub->SetCursorY(pSub->GetCursorY()+1);
				        }
                        break;}
                    case ARROW_LEFT:{
                    //move either one position to the left or to the previous row
                        if (0 <= pSub->GetCursorX()-1){
					        pSub->SetCursorX(pSub->GetCursorX()-1);
				        }
				        else if (0<= pSub->GetCursorY()+offset-1){
					        pSub->SetCursorY(pSub->GetCursorY()-1);
					        pSub->SetCursorX(pDoc->GetLineLen(pSub->GetCursorY()+offset)); 
				        }
                        break;}
                    case ARROW_UP:{
                    //move one row up
                        if (0 <= pSub->GetCursorY()-1+offset){
                            pSub->SetCursorY(pSub->GetCursorY()-1);
				        }
                        break;}
                    case ARROW_DOWN:{
                    //move one row down
                        if (pDoc->GetNumLines() > pSub->GetCursorY()+1+offset){
                            pSub->SetCursorY(pSub->GetCursorY()+1);
                        }
                        break;}
                    case BACKSPACE:{
                    //run the command to delete a character or merge lines
                        if (0 <= pSub->GetCursorX()-1){
					        CmdBackspaceText * cmd = new CmdBackspaceText(pDoc, pSub->GetCursorX(), pSub->GetCursorY()+offset);
					        history.ExecuteCmd(cmd);
					        pSub->SetCursorX(pSub->GetCursorX()-1);
				        }
				        else if (0<= pSub->GetCursorY()+offset-1){
					        pSub->SetCursorX(pDoc->GetLineLen(pSub->GetCursorY()+offset-1));
					        CmdBackspaceRow * cmd = new CmdBackspaceRow(pDoc, pSub->GetCursorX(), pSub->GetCursorY()+offset);
					        history.ExecuteCmd(cmd);
			                pSub->SetCursorY(pSub->GetCursorY()-1);
				        }
                        break;}
                    case CTRL_Z:{
                        history.Undo();
                        break;}
                    case CTRL_Y:{
                        history.Redo();
                        break;}
                    case CTRL_F:{
                        mode = 1;
                        break;}
                    default:{
                        CmdAddText * cmd = new CmdAddText(pDoc, key, pSub->GetCursorX(), pSub->GetCursorY()+offset);
				        history.ExecuteCmd(cmd);
				        pSub->SetCursorX(pSub->GetCursorX()+1);
                        break;}
                }
                break;}
            case 1 :{
            //search mode
                switch(key){
                    case ENTER:{
                        HighlightMatches();
                        break;}
                    case '/':{
                        mode = 2;
                        break;}
                    case ESC:{
                        mode = 0;
                        pSub->ClearColor();
                        search.clear();
                        break;}
                    case BACKSPACE:{
                        if (search.size() > 0){
					        search.erase(search.end()-1);
				        }
                        break;}
                    default:{
                        if (key != ARROW_DOWN && key != ARROW_LEFT && key != ARROW_RIGHT && key != ARROW_UP){
				            search.push_back((char) key);
			            }
                        break;}
                }
                break;}
            case 2 :{
            //replace mode
                switch(key){
                    case ENTER:{
                        ReplaceWord();
                        break;}
                    case ESC:{
                        mode = 1;
				        replace.clear();
                        break;}
                    case BACKSPACE:{
                        if (replace.size() > 0){
					        replace.erase(replace.end()-1);
				        }
                        break;}
                    default:{
                        if (key != ARROW_DOWN && key != ARROW_LEFT && key != ARROW_RIGHT && key != ARROW_UP){
				            replace.push_back((char) key);
                        }
                        break;}
                }
                break;}
        }
        //update display
        UpdateDisplay();
    }
    void HighlightMatches(){
        //we need to re-declare the offset
        int offset = page*pSub->GetRowNumInView();
        if (search != ""){
            //highlight all occurences of the word on the current page
            pSub->ClearColor();
            for(int i = 0; i+offset < pDoc->size() && i < pSub->GetRowNumInView(); i++){
                int n = pDoc->NumMatchesInRow(i+offset, search);
                for(int j = 0; j < n; j++){
                    int xs = pDoc->FindNthMatch(i+offset, j+1, search);
                    pSub->SetColor(i, xs, xs+search.size()-1, TEXT_COLOR_RED);
                }
            }
            highlight_search = search;
        }
    }
    void ReplaceWord(){
        int offset = page*pSub->GetRowNumInView();
        mode = 1;
		pSub->ClearColor();
		//do the replacement
		if (highlight_search != "" && replace != ""){
            //use the replace command
			CmdReplace * cmd = new CmdReplace(pDoc, highlight_search, replace, offset, pSub->GetRowNumInView());
			history.ExecuteCmd(cmd);
			//set search to be the new word
			search = replace;
			//fix the highlight
            //this is similar to HighlightMatches
			offset = page*pSub->GetRowNumInView();
			for(int i = 0; i+offset < pDoc->size() && i < pSub->GetRowNumInView(); i++){
			    int n = pDoc->NumMatchesInRow(i+offset, search);
			    for(int j = 0; j < n; j++){
					int xs = pDoc->FindNthMatch(i+offset, j+1, search);
					pSub->SetColor(i, xs, xs+search.size()-1, TEXT_COLOR_RED);
				}
			}
			highlight_search = search;
		    //clear out replace for next time
			replace.clear();
		}
        return;
    }
    void UpdateDisplay(){
        //update the display with new document traits
		//filter by page size to handle pages
		pSub->InitRows();
		//refresh offset
		int offset = page*pSub->GetRowNumInView();
		//fix cursor for page changes from enter/backspace/up/down
		if(pDoc->GetNumLines() <= pSub->GetCursorY()+offset){
            //correct for undo
        	pSub->SetCursorY(pSub->GetCursorY()-1);
        }
		if (pSub->GetCursorY() < 0){
            //page down
			page -= 1;
			pSub->SetCursorY(pSub->GetRowNumInView()-1);
		}
		if (pSub->GetCursorY() >= pSub->GetRowNumInView()){
            //page up
			page += 1;
			pSub->SetCursorY(0);
		}
        offset = page*pSub->GetRowNumInView();
		if(pSub->GetCursorX() > pDoc->GetLineLen(pSub->GetCursorY()+offset)){
            //make sure X is always correct after row is moved
			pSub->SetCursorX(pDoc->GetLineLen(pSub->GetCursorY()+offset)); 
		}
		//update rows on display
		for(int i = 0; i+offset < pDoc->size() && i < pSub->GetRowNumInView(); i++){
			pSub->AddRow(pDoc->GetDocLine(i+offset));
		}
		//update status row
		pSub->ClearStatusRows();
		if (mode == 0){
			pSub->AddStatusRow("text editor", "version 5.0", true);
		}
		if(mode == 1){
			//display text being searched for
			pSub->AddStatusRow("search for text : "+ search, "ENTER to highlight, / to continue", true);
		}
		if (mode == 2){
			//display replacement text
			pSub->AddStatusRow("enter replacement text : "+replace, "ENTER to replace", true);
		}
    }
private:
    ECTextViewImp * pSub;
	ECDocument * pDoc;
	int page;
	//modes : [0->editor, 1->search, 2->replace]
	int mode;
	std::string search;
	std::string highlight_search;
	std::string replace;
	CommandHistory history;
};
#endif