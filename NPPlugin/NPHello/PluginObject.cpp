#include "PluginObject.h"

#include <string>
#include <sstream>
#include <vector>
#include <Windows.h>
#include "plugin.h"
#include "comdef.h"
#include "json/json.h"

// define function name called by JS
const char* kPluginFunction = "function";
// define property name set by JS
const char* kPluginCallback = "callback";

extern NPNetscapeFuncs NPNFuncs;

namespace {
// create a threed
DWORD WINAPI FunctionThread(LPVOID lparam){ 
	PluginObject* pluginObj = (PluginObject*)lparam;
  if (pluginObj==NULL)
    return false;

  pluginObj->PluginFunction();
  return true;
}

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr){
  // when got a empty wstring, vs2010 will break on an asserting: string 
  // substring out of range
  if (wstr.size()==0) return "";
  int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
  std::string strTo( size_needed, 0 );
  WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
  return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str){
  if (str.size()==0) return L"";
  int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
  std::wstring wstrTo( size_needed, 0 );
  MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
  return wstrTo;
}
}


PluginObject::PluginObject(NPP npp):
    npp(npp),
    hThread(0),
    mJSFunction(NULL) {
}

PluginObject::~PluginObject(void)
{

}

void PluginObject::deallocate(){
  if(mJSFunction != NULL){
    NPN_ReleaseObject(mJSFunction);  
	}
}

void PluginObject::invalidate()
{
}

bool PluginObject::hasMethod(NPIdentifier methodName)
{
	bool bRev = false;
	NPUTF8 *pName = NPNFuncs.utf8fromidentifier(methodName);

  if (strcmp(pName, kPluginFunction) ==0){
    return true;
  } else {
	  return false;
  }
}

bool PluginObject::invokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	return true;
}

bool PluginObject::invoke(NPIdentifier methodName,
    const NPVariant* args, uint32_t argCount, NPVariant* result) { 

  char* name = NPNFuncs.utf8fromidentifier(methodName);
  bool ret_val = false;
  std::string outString;

  if (!name) {
    return ret_val;
  }
  if (strcmp(name, kPluginFunction)==0){
    ret_val = true;
    hThread = CreateThread(NULL, 0, FunctionThread, this, 0, NULL);
    outString = "Called plugin method from external.";
  } else {
    // Exception handling. 
    outString = "Called an invalid method.";
  }
  char* npOutString = (char *)NPNFuncs.memalloc(outString.length() + 1);
  if (!npOutString)
    return false;
  strcpy_s(npOutString, outString.length()+1, outString.c_str());
  STRINGZ_TO_NPVARIANT(npOutString, *result);

  NPNFuncs.memfree(name);
  return ret_val;
}

bool PluginObject::hasProperty(NPIdentifier propertyName)
{
	bool bRev = false;
	NPUTF8 *pName = NPNFuncs.utf8fromidentifier(propertyName);

	if (pName!=NULL){
    if (strcmp(pName, kPluginCallback) == 0){
			return true;
    }
	}
	return bRev;
}

bool PluginObject::getProperty(NPIdentifier propertyName, NPVariant *result)
{
	return false;
}

bool PluginObject::setProperty(NPIdentifier name,const NPVariant *value){
	bool bRev = false;
  if (name == NPN_GetStringIdentifier(kPluginCallback) ){
    mJSFunction = NPN_RetainObject(NPVARIANT_TO_OBJECT(*value));  
		bRev = true;
  }
	return bRev;
}

bool PluginObject::removeProperty(NPIdentifier name)
{
	return true;
}

bool PluginObject::enumerate(NPIdentifier **identifier,uint32_t *count)
{
  return false;
}

bool PluginObject::construct(const NPVariant *args,uint32_t argCount, NPVariant *result)
{
	return true;
}

NPObject *PluginObject::_allocate(NPP npp,NPClass *aClass)
{
	return new PluginObject(npp);
}

void PluginObject::_deallocate(NPObject *npobj)
{
	((PluginObject*)npobj)->deallocate();

	if(npobj)
	{
		delete npobj;
	}
}

void PluginObject::_invalidate(NPObject *npobj)
{
	((PluginObject*)npobj)->invalidate();
}

bool PluginObject::_hasMethod(NPObject* obj, NPIdentifier methodName)
{
	return ((PluginObject*)obj)->hasMethod(methodName);
}

bool PluginObject::_invokeDefault(NPObject *obj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	return ((PluginObject*)obj)->invokeDefault(args,argCount,result);
}

bool PluginObject::_invoke(NPObject* obj, NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	return ((PluginObject*)obj)->invoke(methodName,args,argCount,result);
}

bool PluginObject::_hasProperty(NPObject *obj, NPIdentifier propertyName)
{
	return ((PluginObject*)obj)->hasProperty(propertyName);
}

bool PluginObject::_getProperty(NPObject *obj, NPIdentifier propertyName, NPVariant *result)
{
	return ((PluginObject*)obj)->getProperty(propertyName,result);
}

bool PluginObject::_setProperty(NPObject *npobj, NPIdentifier name,const NPVariant *value)
{
	return ((PluginObject*)npobj)->setProperty(name,value);
}

bool PluginObject::_removeProperty(NPObject *npobj, NPIdentifier name)
{
	return ((PluginObject*)npobj)->removeProperty(name);
}

bool PluginObject::_enumerate(NPObject *npobj, NPIdentifier **identifier,uint32_t *count)
{
	return ((PluginObject*)npobj)->enumerate(identifier,count);
}

bool PluginObject::_construct(NPObject *npobj, const NPVariant *args,uint32_t argCount, NPVariant *result)
{
	return ((PluginObject*)npobj)->construct(args,argCount,result);
}

void PluginObject::CallJSFunction(std::string result){
	int iRev = 0;

  if (mJSFunction != NULL){
		// 转换参数列表
		NPVariant relements[1];
    STRINGZ_TO_NPVARIANT(result.c_str(), relements[0]);

		// 调用JS函数
    NPVariant jsResult; 
		NPN_InvokeDefault(npp, mJSFunction, relements, 1, &jsResult);

		if (NPVARIANT_IS_STRING(jsResult)){
			NPString rString = NPVARIANT_TO_STRING(jsResult);

			char revBuf[255] = {0};
			memcpy(revBuf, rString.UTF8Characters, rString.UTF8Length);
		}

		// 释放从浏览器那获取的结果
		NPN_ReleaseVariantValue(&jsResult);
	}
  return;
}

void PluginObject::PluginFunction(){
  try{
    Sleep(3000);
    Json::Value root;
    root["msg"] = "Plugin process complete with three minutes!";
    CallJSFunction(root.toStyledString());
  } catch(HRESULT hr){
    _com_error error(hr);
    std::wstring msg(error.ErrorMessage());
    Json::Value root;
    root["msg"] = utf8_encode(msg);
    CallJSFunction(root.toStyledString());
  }
}