/*
* Copyright (c) 1999-2006,2007, Craig S. Harrison
*
* All rights reserved.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/
#include <stdio.h>
#include <stdlib.h>
#include "cshTypes_Collection.h"
#include "cshTypes_String.h"
#include "fileStream.h"
#include <string.h>
#include "atozCounter.h"

void cshexit(int i)
{
  printf("OOPS!\n");
		exit(i);
}
typedef CSHCollection<CSHString *>::collection stringListType;

class production
{
	public:

		production():numOfGrammarSymbols(0){}
		virtual void setLVal(CSHString &r);
		virtual void addRVal(CSHString &l);
		virtual CSHString &getLVal()
		{
			return lVal;
		}

		virtual CSHString *getRValAt(int i)
		{
			if(i<productionRValList.getNumberOfItems())
				return (productionRValList.getValueAtIndex(i));
			else
				return NULL;
		}

		virtual int findIndexOfTokenInR(CSHString *tok,int from=0)
		{
			int retVal = -1;
			for(int i=from;i<productionRValList.getNumberOfItems();i++)
			{
				if(productionRValList.getValueAtIndex(i)->equal(tok->GetBuffer()))
				{
					retVal = i;
					break;
				}
			}

			return retVal;
		}

		int numOfGrammarSymbols;

	private:
		CSHString lVal;
		stringListType productionRValList;
};

void production::setLVal(CSHString &r)
{
	lVal = r;
}

void production::addRVal(CSHString &l)
{
	productionRValList.add(new CSHString(l));
}

typedef CSHCollection<production *>::collection productionListType;

struct itemSetItem
{
	itemSetItem():prod(NULL),dotPos(-1),processed(0)
	{
	}

	itemSetItem(production *p,int dp):prod(p),dotPos(dp),processed(0)
	{
	}

	virtual int equal(itemSetItem *anotherSetItem)
	{
		int retVal = 0;
		if((prod==anotherSetItem->prod) && (dotPos==anotherSetItem->dotPos))
			retVal = 1;

		return retVal;
	}

	production *prod;
	int dotPos;
	int processed;
};

typedef CSHCollection<itemSetItem *>::collection itemSetListType;

struct transition
{
	transition(CSHString *t,int ns):transitionSymbol(t),newstate(ns){}
	CSHString *transitionSymbol;
	int newstate;
};

typedef CSHCollection<transition *>::collection transitionList;

struct setListAndTransitions
{
	itemSetListType *itemSetList;
	transitionList *transList;
};

typedef setListAndTransitions itemSetListTypeXX;
typedef CSHCollection<itemSetListTypeXX *>::collection itemsListType;

struct followsItem;

typedef CSHCollection<followsItem *>::collection followsItemItemListType;

struct followsItemListItem
{
	followsItemItemListType fi;
	CSHString *nonTerminal;
	int patched;
};
typedef followsItemListItem followsItemListType;


typedef CSHCollection<followsItemListType *>::collection nonTerminalsFollowItemsListType;

class grammar
{
public:
  
  virtual void init(stringListType &languageDescription);
  virtual void patchLabels();
  virtual void generateParseTable();
  virtual void dumpRelabeledGrammarProductions();
  
private:
  virtual void parseTerminal(CSHString &s);
  virtual void parseNonterminal(CSHString &s);
  virtual void parseProduction(CSHString &s);
  virtual void parseLabeledProduction(CSHString &s);
  virtual void parseInitialProduction(CSHString &s);
  virtual productionListType *productionIndexLHSEqual(CSHString *s);
  virtual void generateClosure(itemSetListTypeXX *itemSetList);
  virtual int isInList(CSHString *s,stringListType &l);
  virtual void dumpItemSetList(int currState,itemSetListTypeXX *isl);
  virtual void dumpItemSetTransitionList(int currState,itemSetListTypeXX *isl);
  virtual void dumpGOTOTransitionList(int currState,itemSetListTypeXX *isl);
  virtual void generateAllSetsOfItems();
  virtual int setOfItemsExists(itemSetListTypeXX *soil);
  virtual void generateAllFollows();
  virtual int indexInFollowList(followsItem *fi,followsItemItemListType *currFollowsList);
  virtual int getTotalNumberOfFollowItems(nonTerminalsFollowItemsListType &ntl);
  virtual int numFollowsItems(followsItemItemListType *il);
  virtual void patchTerminals(CSHString *nt,followsItemItemListType *fil,nonTerminalsFollowItemsListType *ntfl);
  virtual void addAFollows(CSHString *tok,followsItemListType *currFollowsList);
  virtual void addFirstsToFollowsList(CSHString *t,followsItemListType *currFollowsList);
    
  static const char *terminalString;
  static const char *nonterminalString;
  static const char *productionString;
  static const char *labeledProductionString;
  static const char *initialProductionString;
  
  stringListType terminalList;
  stringListType nonterminalList;
  productionListType productionList;
  stringListType productionLabelList;
  itemsListType itemsList;
  CSHString initialProductionLHS;
  int productionListLen;
  nonTerminalsFollowItemsListType nonTerminalFollows;
};

const char *grammar::terminalString = "TERMINAL:";
const char *grammar::nonterminalString = "NONTERMINAL:";
const char *grammar::productionString = "PRODUCTION:";
const char *grammar::labeledProductionString = "LABELEDPRODUCTION:";
const char *grammar::initialProductionString = "INITIALPRODUCTION:";

int grammar::isInList(CSHString *s,stringListType &l)
{
	int retVal = 0;
	for(int i=0;i<l.getNumberOfItems();i++)
	{
		if(s->equal(*(l.getValueAtIndex(i))))
		{
			retVal = 1;
			break;
		}
	}

	return retVal;
}

productionListType *grammar::productionIndexLHSEqual(CSHString *s)
{
	//This could be optimised by only determining these sets once...
	productionListType *pl = new productionListType;
	for(int i=0;i<productionListLen;i++)
	{
		production *pinternal = productionList.getValueAtIndex(i);
		if(pinternal->getLVal().equal(*s))
		{
			pl->add(pinternal);
		}
	}

	return pl;
}

void grammar::generateClosure(itemSetListTypeXX *itemSetList)
{
	stringListType tempNonTerminalAddedList;
	for(int index = 0;index<itemSetList->itemSetList->getNumberOfItems();index++)
	{
		itemSetItem *isi = itemSetList->itemSetList->getValueAtIndex(index);
		CSHString *currGS = isi->prod->getRValAt(isi->dotPos);
		if(currGS!=NULL)
			if(isInList(currGS,nonterminalList))
			{
				//This is a non terminal GS so need to
				//add all these productions at dot 0 if we
				//have not already done this.
				if(!isInList(currGS,tempNonTerminalAddedList))
				{
					productionListType * pl = productionIndexLHSEqual(currGS);
					for(int j=0;j<pl->getNumberOfItems();j++)
					{
						itemSetItem *initItem = new itemSetItem(pl->getValueAtIndex(j),0);
						itemSetList->itemSetList->add(initItem);
					}
					tempNonTerminalAddedList.add(new CSHString(*currGS));
				}
			}
	}
}

struct followsItem
{
	int equals(followsItem *fi2)
	{
		int retVal = 0;
		if((end==1)&&(fi2->end==1))
		{
			retVal = 1;
		}
		else
		{
			if(!((end==1)||(fi2->end==1)))
				if(tok->equal(fi2->tok->GetBuffer()))
					retVal = 1;
		}

		return retVal;
	}

	CSHString *tok;
	int follows;
	int end;
};


void grammar::generateParseTable()
{
	//OK first find the start symbol
	productionListType *pl= productionIndexLHSEqual(&initialProductionLHS);

	itemSetListTypeXX *itemSetList = new itemSetListTypeXX;
	itemSetList->transList = new transitionList;
	itemSetList->itemSetList = new itemSetListType;
	int i;
	for(i=0;i<pl->getNumberOfItems();i++)
	{
		itemSetItem *initItem = new itemSetItem(pl->getValueAtIndex(i),0);
		itemSetList->itemSetList->add(initItem);
	}

	generateClosure(itemSetList);

	itemsList.add(itemSetList);

	generateAllSetsOfItems();

	generateAllFollows();

	printf("=================================\n");
	printf("=======Start List Of Items=======\n");
	printf("=================================\n");
	for(i=0;i<itemsList.getNumberOfItems();i++)
	{
		dumpItemSetList(i,itemsList.getValueAtIndex(i));
	}
	printf("=================================\n");
	printf("========End List Of Items========\n");
	printf("=================================\n");

	printf("=================================\n");
	printf("=======Start List Of Trans=======\n");
	printf("=================================\n");
	for(i=0;i<itemsList.getNumberOfItems();i++)
	{
		dumpItemSetTransitionList(i,itemsList.getValueAtIndex(i));
	}
	printf("=================================\n");
	printf("========End List Of Trans========\n");
	printf("=================================\n");

	printf("=================================\n");
	printf("=======Start Parser Entries======\n");
	printf("=================================\n*/\n");

	for(i=0;i<itemsList.getNumberOfItems();i++)
	{
		itemSetListTypeXX *currList = itemsList.getValueAtIndex(i);

		for(int j=0;j<currList->itemSetList->getNumberOfItems();j++)
		{
			itemSetItem *currItem = currList->itemSetList->getValueAtIndex(j);

			//Entry[1]
			CSHString *next = currItem->prod->getRValAt(currItem->dotPos);
			if(next!=NULL)
			{
				if(isInList(next,terminalList))
				{
					//We have a terminal
					//So find this terminal in the trans list
					for(int i2=0;i2<currList->transList->getNumberOfItems();i2++)
					{
						if(currList->transList->getValueAtIndex(i2)->transitionSymbol->equal(next->GetBuffer()))
						{
						  printf("SHIFT_ACTION(%d,\"%s\",%d)\n",i,next->GetBuffer(),currList->transList->getValueAtIndex(i2)->newstate);
						}
					}
				}
			}

			//Entry[2]
			if(!currItem->prod->getLVal().equal(initialProductionLHS.GetBuffer()))
			{
				if(currItem->dotPos==currItem->prod->numOfGrammarSymbols)
				{
					//Now we need to find the LHS follows list

					followsItemListItem *fi = NULL;
					for(int i4=0;i4<nonTerminalFollows.getNumberOfItems();i4++)
					{
						CSHString *nt =nonTerminalFollows.getValueAtIndex(i4)->nonTerminal;
						CSHString c = currItem->prod->getLVal();
						if(nt->equal(c.GetBuffer()))
						{
							fi = nonTerminalFollows.getValueAtIndex(i4);
							break;
						}
					}

					
					for(int i3=0;i3<fi->fi.getNumberOfItems();i3++)
					{
						followsItem *fii = fi->fi.getValueAtIndex(i3);

						int redNum = -9999;
						//We need to find the index of this prod

						for(int i5=0;i5<productionList.getNumberOfItems();i5++)
						{
							if(productionList.getValueAtIndex(i5)==currItem->prod)
							{
								redNum = i5;
								break;
							}
						}

						if(fii->end==1)
						  printf("REDUCE_ACTION_ON_END_OF_TOKENS(%d,\"%s\")\n",i,productionLabelList.getValueAtIndex(redNum)->GetBuffer());
						else
						  printf("REDUCE_ACTION(%d,\"%s\",\"%s\")\n",i,fii->tok->GetBuffer(),productionLabelList.getValueAtIndex(redNum)->GetBuffer());
					}
				}
			}
			else
			{
				//Entry[2]
				//We have the augmented grammar symbol.
				if(currItem->dotPos==currItem->prod->numOfGrammarSymbols)
					printf("END_OF_TOKENS_ACTION_ACCEPT(%d)\n",i);
			}
		}
	}

	printf("//=================================\n");
	printf("//========End Parser Entries=======\n");
	printf("//=================================\n\n");

	printf("//=================================\n");
	printf("//========Start GOTO Entries=======\n");
	printf("//=================================\n");

	for(i=0;i<itemsList.getNumberOfItems();i++)
	{
	  dumpGOTOTransitionList(i,itemsList.getValueAtIndex(i));
	}

	printf("/*\n=================================\n");
	printf("========End GOTO Entries=========\n");
	printf("=================================\n");}

int grammar::indexInFollowList(followsItem *fi,followsItemItemListType *currFollowsList)
{
	//followsItemListType *currFollowsList

	int retVal = -1;
	for(int i=0;i<currFollowsList->getNumberOfItems();i++)
	{
		if(currFollowsList->getValueAtIndex(i)->equals(fi))
		{
			retVal = i;
			break;
		}
	}
	
	return retVal;
}

int grammar::numFollowsItems(followsItemItemListType *il)
{
	int retVal = 0;
	for(int i=0;i<il->getNumberOfItems();i++)
		if(il->getValueAtIndex(i)->end==0)
			if(il->getValueAtIndex(i)->follows==1)
				retVal++;

	return retVal;
}

int grammar::getTotalNumberOfFollowItems(nonTerminalsFollowItemsListType &ntl)
{
	int retVal = 0;

	for(int i=0;i<ntl.getNumberOfItems();i++)
	{
		retVal += numFollowsItems(&(ntl.getValueAtIndex(i)->fi));
		/*
		for(int j=0;j<ntl.getValueAtIndex(i)->fi.getNumberOfItems();j++)
			if(ntl.getValueAtIndex(i)->fi.getValueAtIndex(j)->end==0)
				if(ntl.getValueAtIndex(i)->fi.getValueAtIndex(j)->follows==1)
					retVal++;
		*/
	}
	return retVal;
}

void grammar::patchTerminals(CSHString *nt,followsItemItemListType *fil,nonTerminalsFollowItemsListType *ntfl)
{
	//					if(!isInFollowList(newFI,&(currFollowsList->fi)))
	//						currFollowsList->fi.add(newFI);
	followsItem *lookupFI = new followsItem;
	lookupFI->end = 0;
	lookupFI->follows = 1;
	lookupFI->tok = nt;

	for(int i=0;i<ntfl->getNumberOfItems();i++)
	{
		followsItemListType *curr = ntfl->getValueAtIndex(i);

		int index = indexInFollowList(lookupFI,&(curr->fi));
		if(index != -1)
		{
			curr->fi.removeItemAtIndex(index);

			for(int j=0;j<fil->getNumberOfItems();j++)
			{
				followsItem *copyFI = fil->getValueAtIndex(j);
				
				if(indexInFollowList(copyFI,&(curr->fi))==-1)
					curr->fi.add(copyFI);
			}
		}
	}
}

void grammar::addAFollows(CSHString *tok,followsItemListType *currFollowsList)
{
  followsItem *newFI = new followsItem;
  newFI->tok = tok;
  newFI->follows = 0;
  newFI->end = 0;

  if(indexInFollowList(newFI,&(currFollowsList->fi))==-1)
    currFollowsList->fi.add(newFI);
  else
    delete newFI;
}

void grammar::addFirstsToFollowsList(CSHString *tok,followsItemListType *currFollowsList)
{

  printf("finding FIRST(%s)\n",tok->GetBuffer());
  
  stringListType tempList;

  tempList.add(tok);

  for(int i=0;i<tempList.getNumberOfItems();i++)
    {
      CSHString *t = tempList.getValueAtIndex(i);
      productionListType *prodList = productionIndexLHSEqual(t);

      int prodSize = prodList->getNumberOfItems();
      for(int j=0;j<prodSize;j++)
	{
	  CSHString *currentFirstProd = prodList->getValueAtIndex(j)->getRValAt(0);
	  if(isInList(currentFirstProd,terminalList))
	    {
	      printf("\t%s\n",currentFirstProd->GetBuffer());
	      addAFollows(currentFirstProd,currFollowsList);
	    }
	  else
	    if(!isInList(currentFirstProd,tempList))
	      tempList.add(currentFirstProd);
	}

      delete prodList;
    }
}

void grammar::generateAllFollows()
{
	int numOfNonTerminals = nonterminalList.getNumberOfItems();

	for(int i=0;i<numOfNonTerminals;i++)
	{
		CSHString *currNonTerminal = nonterminalList.getValueAtIndex(i);
		followsItemListType *currFollowsList = new followsItemListType;
		currFollowsList->patched = 0;
		currFollowsList->nonTerminal = currNonTerminal;
		nonTerminalFollows.add(currFollowsList);

		for(int j=0;j<productionList.getNumberOfItems();j++)
		{
			production *currProd = productionList.getValueAtIndex(j);

			int index = currProd->findIndexOfTokenInR(currNonTerminal,0);
			while(index!=-1)
			{
				if(index==(currProd->numOfGrammarSymbols)-1)
				{
				  followsItem *newFI = new followsItem;
				  //Only do this is the lhs if the augmented grammar symbol
				  //Add a condition here
				  newFI->end = 1;
				  if(indexInFollowList(newFI,&(currFollowsList->fi))==-1)
				    currFollowsList->fi.add(newFI);
				}
				else
				{
				  CSHString *t = currProd->getRValAt(index+1);
				  if(isInList(t,terminalList))
				      addAFollows(t,currFollowsList);
				  else
				    {
				      addFirstsToFollowsList(t,currFollowsList);
				    }
				}
				index = currProd->findIndexOfTokenInR(currNonTerminal,index+1);
			}
			
			//This next section is wrong, if we are at the end of the
			//rhs and we have found our current nonTerminal gs, then
			//we add a follows(lhs), this code is wrong as it checks the lhs is
			//the current nt and then adds follows(last rhs gs) so the follows list is
			//completely up the spout!
			/*
			if(currProd->getLVal().equal(currNonTerminal->GetBuffer()))
			{
				CSHString *endTok = currProd->getRValAt(currProd->numOfGrammarSymbols-1);
				if(isInList(endTok,nonterminalList))
				{
					followsItem *newFI = new followsItem;
					newFI->tok = endTok;
					newFI->follows = 1;
					newFI->end = 0;

					if(indexInFollowList(newFI,&(currFollowsList->fi))==-1)
						currFollowsList->fi.add(newFI);
				}
			}
			*/
			if(currProd->getRValAt(currProd->numOfGrammarSymbols-1)->equal(currNonTerminal->GetBuffer()))
			{
				if(!initialProductionLHS.equal(currProd->getLVal().GetBuffer()))
				{
					//We have found another follows
					followsItem *newFI = new followsItem;
					newFI->tok = new CSHString(currProd->getLVal());
					newFI->follows = 1;
					newFI->end = 0;

					if(indexInFollowList(newFI,&(currFollowsList->fi))==-1)
						currFollowsList->fi.add(newFI);
				}
			}
		}
	}

	//Now we need to patch all the other non terminal follows into our lists.

	int patched;
	int i;
	do
	{
		patched = 0;

		for(i=0;i<numOfNonTerminals;i++)
		{
			followsItemListType *l = nonTerminalFollows.getValueAtIndex(i);
			
			if(numFollowsItems(&(l->fi))==0 && (l->patched==0))
			{
				patchTerminals(l->nonTerminal,&(l->fi),&nonTerminalFollows);
				l->patched = 1;
				patched = 1;
			}
		}
	}
	while(patched);
	int nonTerminalFollowsCount = getTotalNumberOfFollowItems(nonTerminalFollows);
	if(nonTerminalFollowsCount>0)
	{
		printf("Error there are follows(X) that cannot be patched.\n");
		cshexit(1);
	}

	/////Now do the output...
	for(i=0;i<nonTerminalFollows.getNumberOfItems();i++)
	{
		followsItemItemListType &l = nonTerminalFollows.getValueAtIndex(i)->fi;

		printf("FOLLOWS(%s)=",nonterminalList.getValueAtIndex(i)->GetBuffer());
		for(int j=0;j<l.getNumberOfItems();j++)
		{
			followsItem *fi = l.getValueAtIndex(j);
			if(fi->end==1)
			{
				printf("$,");
			}
			else
			{
				if(fi->follows==1)
				{
					printf("FOLLOWS(%s),",fi->tok->GetBuffer());
				}
				else
					printf("%s,",fi->tok->GetBuffer());
			}
		}
		printf("\n");
	}
}

void grammar::generateAllSetsOfItems()
{
	for(int i=0;i<itemsList.getNumberOfItems();i++)
	{
		itemSetListTypeXX *itemSetList = itemsList.getValueAtIndex(i);

		CSHString *chosenTransition = NULL;
		int numberProcessed = 0;
		while(numberProcessed<itemSetList->itemSetList->getNumberOfItems())
		{
			itemSetListTypeXX *tempItemSetList;
			for(int j=0;j<itemSetList->itemSetList->getNumberOfItems();j++)
			{
				itemSetItem *item = itemSetList->itemSetList->getValueAtIndex(j);
				if(!item->processed)
				{
					if(chosenTransition==NULL)
					{
						tempItemSetList = new itemSetListTypeXX;
						tempItemSetList->transList = new transitionList;

						tempItemSetList->itemSetList = new itemSetListType;

						chosenTransition = item->prod->getRValAt(item->dotPos);
					}

					if(chosenTransition==NULL)
					{
						item->processed = 1;
						numberProcessed++;
					}
					else
					{
						CSHString *currRVal = item->prod->getRValAt(item->dotPos);
						if(currRVal==NULL)
						{
							item->processed = 1;
							numberProcessed++;
						}
						else if(chosenTransition->equal(*currRVal))
						{
							//We now need to add all this as a
							//new item with the dot move forward by one.
							itemSetItem *newItem = new itemSetItem(item->prod,item->dotPos+1);
							tempItemSetList->itemSetList->add(newItem);
							item->processed = 1;
							numberProcessed++;
						}
					}
				}
			}

			if(chosenTransition!=NULL)
			{
				generateClosure(tempItemSetList);

				//We now have a candidate setOfItems list
				//We only need to add this to our list if
				//it does not aleady exits

				//setOfItemsExists does not return the correct number
				//for I4 --F--> I3 does not correctly identify I3 as the temp setOfItems.
				int indexForSetOfItems = setOfItemsExists(tempItemSetList);
				if(indexForSetOfItems==-1)
					itemsList.add(tempItemSetList);

				if(indexForSetOfItems==-1)
					indexForSetOfItems = itemsList.getNumberOfItems()-1;
				itemSetList->transList->add(new transition(chosenTransition,indexForSetOfItems));
			}

			chosenTransition = NULL;
		}
	}
}

int grammar::setOfItemsExists(itemSetListTypeXX *soil)
{
	int retVal = -1;
	for(int i=0;i<itemsList.getNumberOfItems();i++)
	{
		itemSetListTypeXX *currSOIL = itemsList.getValueAtIndex(i);
		if(currSOIL->itemSetList->getNumberOfItems()==soil->itemSetList->getNumberOfItems())
		{
			int foundSingleMatch = 0;
			int len = currSOIL->itemSetList->getNumberOfItems();
			for(int i1=0;i1<len;i1++)
			{
				foundSingleMatch = 0;
				itemSetItem *soil1 = currSOIL->itemSetList->getValueAtIndex(i1);
				for(int i2=0;i2<len;i2++)
				{
					itemSetItem *soil2 = soil->itemSetList->getValueAtIndex(i2);
					if(soil1->equal(soil2))
					{
						foundSingleMatch = 1;
						break;
					}
				}
				if(!foundSingleMatch)
					break;
			}

			if(foundSingleMatch)
			{
				retVal = i;
				break;
			}
		}
	}
	return retVal;
}

void grammar::dumpItemSetTransitionList(int currState,itemSetListTypeXX *isl)
{
	printf("I%d\n------------\n",currState);
	for(int i=0;i<isl->transList->getNumberOfItems();i++)
	{
		transition *ct = isl->transList->getValueAtIndex(i);
		printf("[%s] %d\n",ct->transitionSymbol->GetBuffer(),ct->newstate);
	}
	printf("------------\n");
}

void grammar::dumpGOTOTransitionList(int currState,itemSetListTypeXX *isl)
{
	for(int i=0;i<isl->transList->getNumberOfItems();i++)
	{
		transition *ct = isl->transList->getValueAtIndex(i);

		if(isInList(ct->transitionSymbol,nonterminalList))
		{
			printf("GOTO_ENTRY(%d,\"%s\",%d)\n",currState,ct->transitionSymbol->GetBuffer(),ct->newstate);
		}
	}
}


void grammar::dumpItemSetList(int currState,itemSetListTypeXX *isl)
{
	printf("I%d\n------------\n",currState);
	for(int i=0;i<isl->itemSetList->getNumberOfItems();i++)
	{
		itemSetItem *item = isl->itemSetList->getValueAtIndex(i);
		printf("%s->",item->prod->getLVal().GetBuffer());
		for(int j=0;j<item->prod->numOfGrammarSymbols;j++)
		{
			if(j==item->dotPos)
				printf(".");
			printf(item->prod->getRValAt(j)->GetBuffer());
		}
		if(item->prod->numOfGrammarSymbols==item->dotPos)
			printf(".");
		printf(" %d\n",item->dotPos);
	}
	printf("------------\n");
}

void grammar::init(stringListType &languageDescription)
{
	int len = languageDescription.getNumberOfItems();

	for(int i=0;i<len;i++)
	{
		CSHString *s = languageDescription.getValueAtIndex(i);
		if(s->find(terminalString)==0)
			parseTerminal(*s);
		else if(s->find(nonterminalString)==0)
			parseNonterminal(*s);
		else if(s->find(productionString)==0)
			parseProduction(*s);
		else if(s->find(labeledProductionString)==0)
			parseLabeledProduction(*s);
		else if(s->find(initialProductionString)==0)
			parseInitialProduction(*s);
	}

	productionListLen = productionList.getNumberOfItems();
}

void grammar::patchLabels()
{
  int firstLabelInit = 0;
  atozCounter<4> currentMax;

  int size = productionLabelList.getNumberOfItems();
  for(int i=0;i<size;i++)
    {
      CSHString *label = productionLabelList.getValueAtIndex(i);
      if(label!=NULL)
	{
	  if(firstLabelInit)
	    {
	      if(currentMax.isLessThan(*label))
		currentMax.init(*label);
	    }
	  else
	    {
	      currentMax.init(*label);
	      firstLabelInit = 1;
	    }
	}
    }

  if(!firstLabelInit)
    currentMax.init("aaaa");

  char b[5];
  b[4] = 0;
  currentMax.getStr(b);  
  printf("currentMax label = %s\n",b);

  for(int i=0;i<size;i++)
    {
      CSHString *label = productionLabelList.getValueAtIndex(i);
      if(label==NULL)
	{
	  if(firstLabelInit)
	    currentMax.addOne();
	  
	  firstLabelInit = 1;
	  currentMax.getStr(b);  
	  productionLabelList.replaceValueAtIndex(i,new CSHString(b));
	}
    }
}

void grammar::dumpRelabeledGrammarProductions()
{
  printf("=================================\n");
  printf("===Start Relabeled Productions===\n");
  printf("=================================\n");
  atozCounter<4> atozLabel;
  char b[5];
  b[4] = 0;

  int size = productionLabelList.getNumberOfItems();
  for(int i=0;i<size;i++)
    {
      CSHString *label = productionLabelList.getValueAtIndex(i);
      atozLabel.init(*label);
      atozLabel.getStr(b);  
      //printf("label = %s\n",b);

      production *p = productionList.getValueAtIndex(i);

      printf("LABELEDPRODUCTION:[] [%s][%s] ",b,p->getLVal().GetBuffer());

      for(int j=0;j<p->numOfGrammarSymbols;j++)
	{
	  printf("[%s]",p->getRValAt(j)->GetBuffer());
	}

      printf("\n");
    }
  printf("=================================\n");
  printf("====End Relabeled Productions====\n");
  printf("=================================\n");
}

void grammar::parseInitialProduction(CSHString &s)
{
	CSHString newS = s.extract(strlen(initialProductionString),s.length());
	//INITIALPRODUCTION:[][E']

	char ldel = newS[0];
	char rdel = newS[1];
	int leftOfItem = newS.findChar(ldel,2);
	int rightOfItem = newS.findChar(rdel,leftOfItem);

	CSHString s2 = newS.extract(leftOfItem+1,rightOfItem);

	initialProductionLHS = s2;
}

void grammar::parseTerminal(CSHString &s)
{
	CSHString newS = s.extract(strlen(terminalString),s.length());
	//TERMINAL:[][+]
	//OK so first 2 chars say what the terminal is delimited by.
	char ldel = newS[0];
	char rdel = newS[1];
	int leftOfItem = newS.findChar(ldel,2);
	int rightOfItem = newS.findChar(rdel,leftOfItem);
	
	CSHString s2 = newS.extract(leftOfItem+1,rightOfItem);

	terminalList.add(new CSHString(s2));
}

void grammar::parseNonterminal(CSHString &s)
{
	CSHString newS = s.extract(strlen(nonterminalString),s.length());
	//NONTERMINAL:[][E]
	//OK so first 2 chars say what the terminal is delimited by.
	char ldel = newS[0];
	char rdel = newS[1];
	int leftOfItem = newS.findChar(ldel,2);
	int rightOfItem = newS.findChar(rdel,leftOfItem);

	CSHString s2 = newS.extract(leftOfItem+1,rightOfItem);

	nonterminalList.add(new CSHString(s2));
}

void grammar::parseProduction(CSHString &s)
{
	CSHString newS = s.extract(strlen(productionString),s.length());
	//PRODUCTION:[] [E'] [E]
	//OK so first 3 chars say what the terminal is
	//delimited by and the Right/Lefthand size separator.
	char ldel = newS[0];
	char rdel = newS[1];
	char rlsep = newS[2];

	int leftOfItem = newS.findChar(ldel,3);
	int rightOfItem = newS.findChar(rdel,leftOfItem);

	productionLabelList.add(NULL);
	CSHString prodLeft = newS.extract(leftOfItem+1,rightOfItem);

	production *newP = new production;
	newP->setLVal(prodLeft);

	if(isInList(&prodLeft,terminalList))
	  {
 	    printf("lhs %s in terminal list\n",prodLeft.GetBuffer());
	    cshexit(7);
	  }
	else if(!isInList(&prodLeft,nonterminalList))
	  {
 	    printf("lhs %s is not in nonterminal list\n",prodLeft.GetBuffer());
	    cshexit(7);
	  }

	char charAfterLeft = newS[rightOfItem+1];
	if(charAfterLeft!=rlsep)
	{
		printf("rlSeparator not found.\n");
		cshexit(6);
	}

	int currPos = rightOfItem+2;
	leftOfItem = newS.findChar(ldel,currPos);
	rightOfItem = newS.findChar(rdel,leftOfItem);
	while(leftOfItem!=-1 && rightOfItem!=-1)
	{
		CSHString s3 = newS.extract(leftOfItem+1,rightOfItem);

		int presenceCount = 0;

		if(isInList(&s3,terminalList))
		  presenceCount++;

		if(isInList(&s3,nonterminalList))
		  presenceCount++;

		if(presenceCount!=1)
		  {
		    printf("presence count for %s is %d, should be either a terminal or a nonterminal.\n",s3.GetBuffer(),presenceCount);
		    cshexit(6);
		  }

		newP->addRVal(s3);
		newP->numOfGrammarSymbols++;

		currPos = rightOfItem + 1;
		leftOfItem = newS.findChar(ldel,currPos);
		rightOfItem = newS.findChar(rdel,leftOfItem);
	}

	productionList.add(newP);
}

void grammar::parseLabeledProduction(CSHString &s)
{
	CSHString newS = s.extract(strlen(labeledProductionString),s.length());
	//PRODUCTION:[] [E'] [E]
	//OK so first 3 chars say what the terminal is
	//delimited by and the Right/Lefthand size separator.
	char ldel = newS[0];
	char rdel = newS[1];
	char rlsep = newS[2];

	int leftOfLabel = newS.findChar(ldel,3);
	int rightOfLabel = newS.findChar(rdel,leftOfLabel);

	int leftOfItem = newS.findChar(ldel,rightOfLabel);
	int rightOfItem = newS.findChar(rdel,leftOfItem);

	CSHString label = newS.extract(leftOfLabel+1,rightOfLabel);
	productionLabelList.add(new CSHString(label));
	CSHString prodLeft = newS.extract(leftOfItem+1,rightOfItem);

	production *newP = new production;
	newP->setLVal(prodLeft);

	if(isInList(&prodLeft,terminalList))
	  {
 	    printf("lhs %s in terminal list\n",prodLeft.GetBuffer());
	    cshexit(7);
	  }
	else if(!isInList(&prodLeft,nonterminalList))
	  {
 	    printf("lhs %s is not in nonterminal list\n",prodLeft.GetBuffer());
	    cshexit(7);
	  }

	char charAfterLeft = newS[rightOfItem+1];
	if(charAfterLeft!=rlsep)
	{
		printf("rlSeparator not found.\n");
		cshexit(6);
	}

	int currPos = rightOfItem+2;
	leftOfItem = newS.findChar(ldel,currPos);
	rightOfItem = newS.findChar(rdel,leftOfItem);
	while(leftOfItem!=-1 && rightOfItem!=-1)
	{
		CSHString s3 = newS.extract(leftOfItem+1,rightOfItem);

		int presenceCount = 0;

		if(isInList(&s3,terminalList))
		  presenceCount++;

		if(isInList(&s3,nonterminalList))
		  presenceCount++;

		if(presenceCount!=1)
		  {
		    printf("presence count for %s is %d, should be either a terminal or a nonterminal.\n",s3.GetBuffer(),presenceCount);
		    cshexit(6);
		  }

		newP->addRVal(s3);
		newP->numOfGrammarSymbols++;

		currPos = rightOfItem + 1;
		leftOfItem = newS.findChar(ldel,currPos);
		rightOfItem = newS.findChar(rdel,leftOfItem);
	}

	productionList.add(newP);
}

//#define CSH_JSON
//#define CSH_SIMPLEEXPR
#define CSH_SIMPLE_FIRST_TEST

#define NEWLANGUAGEDEFINITIONSTRING(X) languageDescription.add(new CSHString(X));
int main(int argc,char *argv[])
{
  if(argc==2)
    {
      stringListType languageDescription;
      try
	{
	  getsFileStream fs(argv[1]);
	  while(1)
	    {
	      CSHString temp(fs.gets());
	      temp.removeWhitespaceFromBothEnds();
	      int truncTo = -1;
	      int posOfCR = temp.find("\r");
	      int posOfLF = temp.find("\n");
	      int length = temp.GetLength();
	      
	      if(posOfCR==-1)
		posOfCR = length;
	      
	      if(posOfLF==-1)
		posOfLF = length;
	      
	      if(posOfLF<posOfCR)
		truncTo = posOfLF;
	      else
		truncTo = posOfCR;
	      
	      CSHString line = temp.extract(0,truncTo);
	      
	      if(line.GetLength()>0)
		{
		  NEWLANGUAGEDEFINITIONSTRING(line);
		}
	    }
	}
      catch(CSHStreamExceptions::couldNotOpenStream &e)
	{
	  printf("ERROR - Could not open file %s\n",e.errorString.GetBuffer());
	}
      catch(getsStream::noMoreStrings &)
	{
	  printf("/*\n");
	  grammar g;
	  g.init(languageDescription);
	  g.patchLabels();
	  g.generateParseTable();
	  g.dumpRelabeledGrammarProductions();
	  printf("*/\n");
	}
    }
  else
    {
      printf("USAGE : %s grammarSpecification.ptg\n",argv[0]);
    }

  return 0;
}
