#pragma once
#include "stdafx.h"
typedef struct returnchar{
	char* name;
	bool flag;
};

class Sort
{
public:
	static bool flagE0;
	static bool flagE1;
	static bool flagPSUP;
	static bool flagPSDN;
	static bool flagPA1;
	static bool flagPAend;
	static bool flagPA2;
	static char* OneKeyMap[89];
	static char* E0name[16];
	static unsigned char E0value[16];

	static struct returnchar find(unsigned char num){
		struct returnchar result;

		if(!Sort::flagE0 && !Sort::flagE1){
		if(num == 224){
			Sort::flagE0=true;
			result.name="wait";
			return result;
		}else if(num == 225){
			Sort::flagE1=true;
			result.name="wait";
			return result;
		}
		
		if(num<128){
			if(num>0 && num<89){
				result.name=OneKeyMap[num];
				result.flag=true;
				return result;
			}else{
				result.name="error";
				return result;
			}
		}else{
			num = num-128;
			if(num>0 && num<89){
				result.name=OneKeyMap[num];
				result.flag=false;
				return result;
			}else{
				result.name="error";
				return result;
			}
		}

		}else if(Sort::flagE0 && !Sort::flagE1){
			/*E0,47 E0,97 "HOME",	
			{"PRNT SCRN","E0,2A,E0,37","E0,B7,E0,AA"},	*/
			Sort::flagE0=false;
			
			if(Sort::flagPSUP){
				Sort::flagPSUP=false;
				if(num == 0x37){
					result.name="keyboard.prtsc_T";
					result.flag=true;
					return result;
				}
				else{
					result.name="error";
					return result;
				}
					
			}
			if(Sort::flagPSDN){
				Sort::flagPSDN=false;
				if(num == 0xAA){
					result.name="keyboard.prtsc_T";
					result.flag=false;
					return result;
				}
				else{
					result.name="error";
					return result;
				}
					
			}
			
			
			if(num == 0x47){
				result.name="keyboard.home_T";
				result.flag=true;
				return result;
			}
			if(num == 0xC7){
				result.name="keyboard.home_T";
				result.flag=false;
				return result;
			}
			if(num == 0x2A){
				Sort::flagPSUP=true;
				result.name="wait";
				return result;
			}
			if(num == 0xB7){
				Sort::flagPSDN=true;
				result.name="wait";
				return result;
			}
			if(num>128){
				result.flag=false;
				num-=128;
			}else{
				result.flag=true;
				
			}
			for(int i=0;i<16;i++){
				if(E0value[i] == num){
					result.name=E0name[i];
					return result;
				}
			}
			result.name="error";
			return result;
			
		}else if(!Sort::flagE0 && Sort::flagE1){
			/*
			{"PAUSE","E1,1D,45",""},	
			{"PAUSE","E1,9D,C5",""  逻辑有问题}	
			*/
			if(num == 0x1D){
				Sort::flagPA1=true;
				result.name="wait";
				return result;
			}
			if(num == 0x9D && Sort::flagPAend){
				Sort::flagPAend=false;
				Sort::flagPA2=true;
				result.name="wait";
				return result;
			}
			
			Sort::flagE1=false;
			if(Sort::flagPA1){
				Sort::flagPA1 =false;
				if(num == 0x45){
					//result.name="keyboard.pause_T";
					//result.flag=true;
					//return result;
					Sort::flagPAend =true;
					result.name="wait";
					return result;
					
				}else {
					result.name="error";
					return result;
				}
					
			}
			if(Sort::flagPA2){
				Sort::flagPA2 =false;
				if(num == 0xC5){
					result.name="keyboard.pause_T";
					result.flag=true;
					return result;
				}
				else{
					result.name="error";
					return result;
				}
			}
			result.name="error";
			return result;
			
		}else{
			Sort::flagE1=false;
			Sort::flagE0=false;
			result.name="error";
			return result;
		}
		result.name="error";
		return result;	
	}
};
