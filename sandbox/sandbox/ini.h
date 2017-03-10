#ifndef __INI_H__
#define __INI_H__

#include <string>
#include "windows.h"

namespace es {

	// use with unicode
	class ini {
		typedef std::string str;
		typedef std::wstring wstr;
	public:
		ini() {}
		ini(str path) {
			wstr tmp(path.begin(), path.end());
			adr = tmp;
		}
		virtual ~ini() {}
	public:
		str reads(str Group, str Key) {
			WCHAR tmp[80];
			wstr g(Group.begin(), Group.end());
			wstr k(Key.begin(), Key.end());
			GetPrivateProfileString(g.c_str(), k.c_str(), TEXT(""), tmp, 80, adr.c_str());
			wstr tmp2 = tmp;
			str tmp3(tmp2.begin(), tmp2.end());
			return tmp3;
		}
		wstr reads(wstr Group, wstr Key) {
			WCHAR tmp[80];
			GetPrivateProfileString(Group.c_str(), Key.c_str(), TEXT(""), tmp, 80, adr.c_str());
			wstr tmp2 = tmp;
			return tmp2;
		}
		int readi(str Group, str Key) {
			wstr g(Group.begin(), Group.end());
			wstr k(Key.begin(), Key.end());
			return GetPrivateProfileInt(g.c_str(), k.c_str(), 0, adr.c_str());
		}
		void writes(str Group, str Key, str Item) {
			wstr g(Group.begin(), Group.end());
			wstr k(Key.begin(), Key.end());
			wstr i(Item.begin(), Item.end());
			WritePrivateProfileString(g.c_str(), k.c_str(), i.c_str(), adr.c_str());
		}
		void writei(str Group, str Key, int Item) {
			wstr g(Group.begin(), Group.end());
			wstr k(Key.begin(), Key.end());
			wstr i = std::to_wstring((long double)Item);
			WritePrivateProfileString(g.c_str(), k.c_str(), i.c_str(), adr.c_str());
		}
		void writei(str Group, str Key, float Item) {
			wstr g(Group.begin(), Group.end());
			wstr k(Key.begin(), Key.end());
			wstr i = std::to_wstring((long double)Item);
			WritePrivateProfileString(g.c_str(), k.c_str(), i.c_str(), adr.c_str());
		}
	public:
		static int readi(str File, str Group, str Key) {
			wstr g(Group.begin(), Group.end());
			wstr k(Key.begin(), Key.end());
			wstr f(File.begin(), File.end());
			return GetPrivateProfileInt(g.c_str(), k.c_str(), 0, f.c_str());
		}
	private:
		wstr adr;
	};

}

#endif __INI_H__#pragma once
