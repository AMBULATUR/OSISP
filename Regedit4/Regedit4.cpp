// Regedit4.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
//
//4. Изучение реестра
//
//Разработать программу
//- создание и открытия ключа
//- чтение флагов составляющих маску ключа
//- закрытия ключа, сохранение изменений
//- добавление данных к ключу
//- поиск ключа(выборка)
//
//дополнительно:
//отслеживание изменение реестра

#include <iostream>
#include <windows.h>
#include <sddl.h>
#include <string>
#include <string.h>



const std::string hKeys[]{
	"HKEY_CLASSES_ROOT",
	"HKEY_CURRENT_CONFIG",
	"HKEY_CURRENT_USER",
	"HKEY_LOCAL_MACHINE",
	"HKEY_USERS" };
HKEY createKey(HKEY hKey, LPCWSTR subKey);
HKEY openKey(HKEY key, LPCWSTR subKey);
PWSTR readAce(HKEY hKey);
LSTATUS addDataToKey(HKEY hKey, LPCWSTR valueName, BYTE* data);
void searchKey(HKEY hKeyParent, LPCWSTR subKey);

int main()
{
	for (auto key : hKeys)
	{
		std::cout << key << std::endl;
	}
	HKEY key = createKey(HKEY_CURRENT_USER, L"subKey");
	if (key)
		std::cout << "Created" << std::endl;
	auto status = RegCloseKey(key);
	if (status == ERROR_SUCCESS)
		std::cout << "Closed" << std::endl;
	HKEY hKey = openKey(HKEY_CURRENT_USER, L"subKey");
	if (hKey)
		std::cout << "Open" << std::endl;
	LSTATUS status2 = addDataToKey(hKey, L"VALUE1", (BYTE*)254);
	if (status2 == ERROR_SUCCESS)
		std::cout << "Data added" << std::endl;
	LSTATUS status3 = RegCloseKey(hKey);
	if (status3 == ERROR_SUCCESS)
		std::cout << "Closed" << std::endl;
	hKey = openKey(HKEY_CURRENT_USER, L"subKey");
	if (hKey)
		std::cout << "Open" << std::endl;
	std::wcout << readAce(hKey) << std::endl; //WIDE wcout,vuzov microsoft, TODO
	searchKey(HKEY_CURRENT_USER, L"System");

}

/// <summary>
/// Создание ключа
/// </summary>
/// <param name="hKey">Ключ</param>
/// <param name="subKey">Подраздел</param>
/// <returns>Created key</returns>
HKEY createKey(HKEY hKey, LPCWSTR subKey)
{
	DWORD dwDisposition;
	HKEY  hKeyResult;
	RegCreateKeyEx(hKey, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyResult, &dwDisposition);
	return hKeyResult;
}

/// <summary>
/// Открытие ключа
/// </summary>
/// <param name="key">Созданный ключ</param>
/// <param name="subKey">Подраздел</param>
/// <returns>Адрес переменной, в которую возвращается дескриптор открытого ключа.</returns>
HKEY openKey(HKEY key, LPCWSTR subKey)
{
	HKEY phKey;
	// Open the key
	RegOpenKeyEx(
		key,
		subKey,
		0,
		KEY_WRITE,
		&phKey
	);
	return phKey;
}
/// <summary>
/// Чтение ACE
/// </summary>
/// <param name="hKeyParent"></param>
/// <param name="subKey"></param>
/// <returns></returns>
LSTATUS addDataToKey(HKEY hKey, LPCWSTR valueName, BYTE* data)
{
	return RegSetValueEx(
		hKey,
		valueName,
		0,
		REG_DWORD,
		(BYTE*)&data,
		sizeof(data));
}

PWSTR readAce(HKEY hKey)
{
	DWORD size = 1024;
	PSECURITY_DESCRIPTOR psd = LocalAlloc(LMEM_FIXED, size);
	PWSTR SecuretyDescriptor;
	if (RegGetKeySecurity(hKey, DACL_SECURITY_INFORMATION, psd, &size) == ERROR_SUCCESS)
	{
		ConvertSecurityDescriptorToStringSecurityDescriptor(psd, SDDL_REVISION_1, DACL_SECURITY_INFORMATION, &SecuretyDescriptor, NULL);
		return SecuretyDescriptor;
	}
}




void searchKey(HKEY hKeyParent, LPCWSTR subKey)
{
	TCHAR name[1024];
	WCHAR className;
	DWORD dwSize = 1024;
	long result;
	result = RegOpenKeyEx(HKEY_CURRENT_USER, subKey, 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE, &hKeyParent);

	if (result != ERROR_SUCCESS)
	{
		auto b = 0;
	}
	else
	{
		DWORD index = 0;
		result = ERROR_SUCCESS;
		while (result == RegEnumKeyEx(hKeyParent, index, name, &dwSize,NULL, NULL, NULL, NULL)) {
			dwSize = 1024; 
			++index;
			std::wcout << index + " " << name <<  std::endl;

		}
	}
}

