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
#include <string.h>

template<int DIGITS,int FLOOR_VALUE=0>
  class atozCounter
  {
  public:
  atozCounter()
  {
    init();
  }
  
  atozCounter(char *initStr)
  {
    init(initStr);
  }
  
  virtual void init()
  {
    for(int i=0;i<DIGITS;i++)
      buff[i] = 0;
  }
  
  virtual void init(char *initStr)
  {
    if(strlen(initStr)==DIGITS)
      {
	for(int i=0;i<DIGITS;i++)
	  buff[DIGITS-i-1] = initStr[i]-'a';
      }
  }
  
  virtual void addOne()
  {
    int carryOccured = 1;
    int index = 0;
    
    while(carryOccured)
      {
	if(buff[index]=='z'-'a')
	  {
	    buff[index] = 0;
	    index++;
	    if(index>=DIGITS)
	      carryOccured = 0;
	  }
	else
	  {
	    buff[index]++;
	    carryOccured = 0;
	  }
      }
  }
  
  virtual int isLessThan(char *anotherStr)
  {
    int retVal = 1;
    atozCounter<DIGITS> other;
    other.init(anotherStr);
    
    for(int i=0;i<DIGITS;i++)
      {
	int index = DIGITS-i-1;
	if(buff[index]>other.buff[index])
	  {
	    retVal = 0;
	    break;
	  }
      }
    
    return retVal;
  }
  
  virtual int equalTo(char *anotherStr)
  {
    int retVal = 1;
    atozCounter<DIGITS> other;
    other.init(anotherStr);
    
    for(int i=0;i<DIGITS;i++)
      {
	int index = DIGITS-i-1;
	if(buff[index]!=other.buff[index])
	  {
	    retVal = 0;
	    break;
	  }
      }
    
    return retVal;
  }
  
  virtual void getStr(char *b)
  {
    for(int i=0;i<DIGITS;i++)
      {
	b[i] = buff[DIGITS-i-1]+'a';
      }
    
  }
  
  virtual int getVal()
  {
    int retVal = 0;
    for(int i=0;i<DIGITS;i++)
      {
	retVal *= 'z'-'a'+1;
	retVal += buff[DIGITS-i-1];
      }

    return retVal+FLOOR_VALUE;
  }
  
  char buff[DIGITS];  
  };


/*
class tester
{
 public:
  static void findGreatest(char *sl[])
  {
    int index = 1;
    atozCounter<4> currentMax;
    currentMax.init(sl[0]);

    while(*sl[index]!='*')
      {
	char *str = sl[index];

	if(str[0]!='*')
	  {
	    if(currentMax.isLessThan(str))
	       currentMax.init(str);
	  }
	else
	  break;

	index++;
      }

    char buff[5];
    buff[4] = 0;
    currentMax.getStr(buff);
    printf("max = %s\n",buff);
  }

  static void test()
  {
    atozCounter<4> c1;
    c1.init("aaaa");
    atozCounter<4> c2;
    char buff[5];

    buff[4] = 0;
    int num;
    char *str1;
    char *str2;
    
    c1.getStr(buff);
    num = c1.getVal();
    printf("%s %d\n",buff,num);
    
    c1.addOne();
    c1.getStr(buff);
    num = c1.getVal();
    printf("%s %d\n",buff,num);

    c1.getStr(buff);
    num = c1.getVal();
    printf("%s %d\n",buff,num);

    str1 = "aaac";
    str2 = "aaaa";
    c1.init(str1);
    printf("%s is less than %s = %d\n",str1,str2,c1.isLessThan(str2));

    str1 = "aaac";
    str2 = "aaad";
    c1.init(str1);
    printf("%s is less than %s = %d\n",str1,str2,c1.isLessThan(str2));

    str1 = "aaac";
    str2 = "aabc";
    c1.init(str1);
    printf("%s is less than %s = %d\n",str1,str2,c1.isLessThan(str2));

    str1 = "aaaa";
    str2 = "aaaa";
    c1.init(str1);
    printf("%s is equal to %s = %d\n",str1,str2,c1.equalTo(str2));

    str1 = "aaaa";
    str2 = "aaac";
    c1.init(str1);
    printf("%s is equal to %s = %d\n",str1,str2,c1.equalTo(str2));

    str1 = "aaaa";
    str2 = "aaca";
    c1.init(str1);
    printf("%s is equal to %s = %d\n",str1,str2,c1.equalTo(str2));

    char *sl[] = {"aaaa","aaaf","aaae","*"};
    findGreatest(sl);
  }
  
};
*/
