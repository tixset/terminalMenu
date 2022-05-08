#pragma once
#include <Arduino.h>

struct tfuncParams {
	int clientIndex;
	int menuIndex;
	String param;
} typeTFuncParams;

extern "C" {
	typedef void (*func)(tfuncParams Params);
}

struct tmenuLines {
	String name;
	func Func;
	int sub = -1;
} typeTMenuLines;

class terminalMenu { 
	public:
		void init(tmenuLines* mLines, int mSize);
		void init(tmenuLines* mLines, int mSize, bool helpSubsEn);
		void helpAttach(func Func);
		void errAttach(func Func);

		int add(String name);
		int add(String name, func Func);
		int add(int sub, String name);
		int add(int sub, String name, func Func);
 
		void ed(int index, String name);
		void ed(int index, int sub);
		void ed(int index, func Func);
		void del(int index);
	
		int goMenu(String line, int clientIndex);
	
		String getHelpLine(int index);
		String getHelpLine(int index, int sub);
	
		int MenuCount;
	private:
		tmenuLines* _mLines;
		int _mSize;
		bool _helpSubsEn = true;
		func _helpFunc;
		func _errFunc;
};

String getValue(String data, char separator, int index) {
	int found = 0;
	int strIndex[] = { 0, -1 };
	int maxIndex = data.length() - 1;
	for (int i = 0; i <= maxIndex && found <= index; i++) {
		if (data.charAt(i) == separator || i == maxIndex) {
			found++;
			strIndex[0] = strIndex[1] + 1;
			strIndex[1] = (i == maxIndex) ? i + 1 : i;
		}
	}
	return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void terminalMenu::init(tmenuLines* mLines, int mSize){
	init(mLines, mSize, true);
}

void terminalMenu::init(tmenuLines* mLines, int mSize, bool helpSubsEn){
	_mLines = mLines;
	_mSize = mSize;
	_helpSubsEn = helpSubsEn;
}

void terminalMenu::helpAttach(func Func){
	_helpFunc = *Func;
}

void terminalMenu::errAttach(func Func){
	_errFunc = *Func;
}

String terminalMenu::getHelpLine(int index){
	return getHelpLine(index, -1);
}

String terminalMenu::getHelpLine(int index, int sub){
	int x = 0;
	String res = "";
	for (int i=0; i <= MenuCount - 1; i++){
		if ((_mLines[i].sub == sub) && (_mLines[i].name != "")) {
			if (index == x) {
				res = _mLines[i].name;
				if (_helpSubsEn) {
					String subs = "";
					int y = 0;
					for (int j=0; j <= MenuCount - 1; j++){
						if (_mLines[j].sub == i) {
							if (y == 0) {
								subs = subs + _mLines[j].name;
							} else {
								subs = subs + "|" + _mLines[j].name;
							}
							y++;
						}
					}
					if (subs != "") {
						res = res + " [" + subs + "]";
					}
				}
				return res;
			}
			x++;
		}
	}
	return "";
}

int terminalMenu::add(String name){
	return add(name, _helpFunc);
}

int terminalMenu::add(String name, func Func){
	return add(-1, name, Func);
}

int terminalMenu::add(int sub, String name){
	return add(sub, name, _helpFunc);
}

int terminalMenu::add(int sub, String name, func Func){
	if (MenuCount + 1 > _mSize) return -1;
	_mLines[MenuCount].name = name;
	_mLines[MenuCount].Func = *Func;
	_mLines[MenuCount].sub = sub;
	MenuCount++;
	return MenuCount - 1;
}

void terminalMenu::ed(int index, String name){
	_mLines[index].name = name;
}

void terminalMenu::ed(int index, int sub){
	_mLines[index].sub = sub;
}

void terminalMenu::ed(int index, func Func){
	_mLines[index].Func = Func;
}

void terminalMenu::ed(int index, String name, int sub){
	_mLines[index].name = name;
	_mLines[index].sub = sub;
}

void terminalMenu::del(int index){
	_mLines[index].name = "";
}

int terminalMenu::goMenu(String line, int clientIndex){
	int x = 0;
	int res = -1;
	int id = -1;
	String param = "";
	while (1) {
		String word = getValue(line, ' ', x);
		if (word == "") {
			res = id;
			break;
		}
		bool found = false;
		for (int i=0; i <= MenuCount - 1; i++){
			int y = 0;
			while (1) {
				String name = getValue(_mLines[i].name, '|', y);
				if (name == "") break;
				if ((name == word) &&
				  (_mLines[i].sub == id)) {
					id = i;
					found = true;
					break;
				}
				y++;
			}
			if (found) {
				break;
			}
		}
		if (not found) {
			param = word;
		}
		x++;
	}
	if (res > -1) {
		tfuncParams Params;
		Params.clientIndex = clientIndex;
		Params.menuIndex = id;
		Params.param = param;
		if((param == "") || (_mLines[res].Func != _helpFunc)){
			if (_mLines[res].Func) {
				(*_mLines[res].Func)(Params);
			}
		} else {
			if (_errFunc) {
				(*_errFunc)(Params);
			}
		}
	}
	return res;
}
