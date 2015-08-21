
#include <v8.h>
#include <node.h>
#include <msclr\marshal.h>
#include <vector>
#include <time.h>
#include "Common.h"

using namespace System;
using namespace System::Globalization;
using namespace System::Reflection;
using namespace System::Collections;
using namespace System::Collections::Generic;

using namespace v8;
using namespace Node2ClrCommon;

static void _logToConsole(const int level, const RunningInfo* info);
static uint32_t _getObjectMemberUInt32Value(Isolate* isolate, v8::Handle<v8::Object> v8Value, const char* key);
static v8::Handle<v8::String> _genKeySame(v8::Isolate* isolate, System::String^ pName);
static v8::Handle<v8::String> _genKeyCamel(v8::Isolate* isolate, System::String^ pName);
static v8::Handle<v8::String> _genKeyLower(v8::Isolate* isolate, System::String^ pName);

static void _throw(v8::Isolate* isolate, const char* function, const char* message);
static bool _isNullable(System::Type^ t);
static v8::Handle<v8::Value> _convertNetObject(v8::Isolate* isolate, System::Object^ obj, System::Type^ type,GenV8ObjectKey genKey);

const char* TIMESAPN_Days = "days";
const char* TIMESAPN_Hours = "hours";
const char* TIMESAPN_Minutes = "minutes";
const char* TIMESAPN_Seconds = "seconds";
const char* TIMESAPN_Milliseconds = "milliseconds";


bool Node2ClrCommon::Common::debug = false;
Node2ClrCommon::Logger Common::logger = NULL;
Node2ClrCommon::Logger Common::CONSOLELOGGER = _logToConsole;
GenV8ObjectKey Node2ClrCommon::GenV8ObjectKey_CAMEL = _genKeyCamel;
GenV8ObjectKey Node2ClrCommon::GenV8ObjectKey_SAME = _genKeySame;
GenV8ObjectKey Node2ClrCommon::GenV8ObjectKey_LOWER = _genKeyLower;

v8::Handle<v8::String> ConvertCommon::ToV8String(v8::Isolate* isolate, System::String^ value)
{
	msclr::interop::marshal_context context;

	const uint16_t* unicode = (const uint16_t*)context.marshal_as<const wchar_t*>(value);

	return v8::String::NewFromTwoByte(isolate, unicode);
}

inline v8::Handle<v8::String> ConvertCommon::ToV8String(v8::Isolate* isolate, const char* value)
{
	return v8::String::NewFromUtf8(isolate, value);
}

inline v8::Handle<v8::Integer> ConvertCommon::ToV8Number(v8::Isolate* isolate, int8_t value)
{
	return v8::Int32::New(isolate, (int32_t)value);
}

inline v8::Handle<v8::Integer> ConvertCommon::ToV8Number(v8::Isolate* isolate, uint8_t value)
{
	return v8::Int32::NewFromUnsigned(isolate, (uint32_t)value);
}

inline v8::Handle<v8::Integer> ConvertCommon::ToV8Number(v8::Isolate* isolate, int16_t value)
{
	return v8::Int32::New(isolate, (int32_t)value);
}

inline v8::Handle<v8::Integer> ConvertCommon::ToV8Number(v8::Isolate* isolate, uint16_t value)
{
	return v8::Int32::NewFromUnsigned(isolate, (uint32_t)value);
}

inline v8::Handle<v8::Integer> ConvertCommon::ToV8Number(v8::Isolate* isolate, int32_t value)
{
	return v8::Int32::New(isolate, value);
}

inline v8::Handle<v8::Integer> ConvertCommon::ToV8Number(v8::Isolate* isolate, uint32_t value)
{
	return v8::Integer::NewFromUnsigned(isolate, value);
}

inline v8::Handle<v8::Number> ConvertCommon::ToV8Number(v8::Isolate* isolate, int64_t value)
{
	if (value >= INT32_MIN && value <= INT32_MAX)
	{
		return v8::Int32::New(isolate, (int32_t)value);
	}

	return v8::Number::New(isolate, (double)value);
}

inline v8::Handle<v8::Number> ConvertCommon::ToV8Number(v8::Isolate* isolate, uint64_t value)
{
	if (value <= UINT32_MAX)
	{
		return v8::Uint32::NewFromUnsigned(isolate, (uint32_t)value);
	}

	return v8::Number::New(isolate, (double)value);
}

inline v8::Handle<v8::Number> ConvertCommon::ToV8Number(v8::Isolate* isolate, float value)
{
	return v8::Number::New(isolate, (double)value);
}

inline v8::Handle<v8::Number> ConvertCommon::ToV8Number(v8::Isolate* isolate, double value)
{
	return v8::Number::New(isolate, value);
}

inline v8::Handle<v8::Boolean> ConvertCommon::ToV8Boolean(v8::Isolate* isolate, bool value)
{
	return v8::Boolean::New(isolate, value);
}

inline v8::Handle<v8::Object> ConvertCommon::ToV8Object(v8::Isolate* isolate, std::map<const char*, v8::Handle<v8::Value>> value)
{
	v8::Handle<v8::Object> obj = v8::Object::New(isolate);
	
	HandleScope scope(isolate);

	std::map<const char*, v8::Handle<v8::Value>>::iterator it = value.begin();

	for (; it != value.end(); ++it)
	{
		obj->Set(v8::String::NewFromUtf8(isolate, it->first), it->second);
	}

	return obj;
}

inline v8::Handle<v8::Object> ConvertCommon::ToV8Object(v8::Isolate* isolate, System::TimeSpan^ value)
{
	v8::Handle<v8::Object> obj = v8::Object::New(isolate);

	HandleScope scope(isolate);

	obj->Set(ConvertCommon::ToV8String(isolate, TIMESAPN_Days), ConvertCommon::ToV8Number(isolate, value->Days));
	obj->Set(ConvertCommon::ToV8String(isolate, TIMESAPN_Hours), ConvertCommon::ToV8Number(isolate, value->Hours));
	obj->Set(ConvertCommon::ToV8String(isolate, TIMESAPN_Minutes), ConvertCommon::ToV8Number(isolate, value->Minutes));
	obj->Set(ConvertCommon::ToV8String(isolate, TIMESAPN_Seconds), ConvertCommon::ToV8Number(isolate, value->Seconds));
	obj->Set(ConvertCommon::ToV8String(isolate, TIMESAPN_Milliseconds), ConvertCommon::ToV8Number(isolate, value->Milliseconds));

	return obj;
}


v8::Handle<v8::Object> ConvertCommon::ToV8Object(v8::Isolate* isolate, System::Object^ value, GenV8ObjectKey genKey)
{
	GenV8ObjectKey keyFunc = genKey != NULL ? genKey : GenV8ObjectKey_CAMEL;

	if (genKey == NULL && Common::logger != NULL)
	{
		LOG(LogLevel::Warnning, "Gen Key function is null, use .Net property name as Javascript object key name")
	}

	v8::Handle<v8::Object> obj = v8::Object::New(isolate);

	if (value != nullptr)
	{
		HandleScope scope(isolate);

		List<MemberInfo^>^ members = gcnew List<MemberInfo^>();
		members->AddRange(value->GetType()->GetProperties(BindingFlags::Public | BindingFlags::Instance));
		members->AddRange(value->GetType()->GetFields(BindingFlags::Public | BindingFlags::Instance));

		for (int i = 0; i < members->Count; ++i)
		{
			MemberInfo^ cur = members[i];
			v8::Handle<v8::String> k = genKey(isolate, cur->Name);
			
			System::Type^ vType;
			System::Object^ v;

			if (cur->MemberType == MemberTypes::Property)
			{
				v = ((PropertyInfo^)cur)->GetValue(value);
				vType = ((PropertyInfo^)cur)->PropertyType;
			}
			else
			{
				v = ((FieldInfo^)cur)->GetValue(value);
				vType = ((FieldInfo^)cur)->FieldType;
			}

			bool isNullable = _isNullable(vType);
			vType = isNullable ? System::Nullable::GetUnderlyingType(vType) : vType;

			if (isNullable && v == nullptr)
			{
				obj->Set(k, v8::Handle<v8::Value>());
			}
			else
			{
				v8::Handle<v8::Value> v8Value = _convertNetObject(isolate, v, vType, genKey);
				obj->Set(k, v8Value);
			}
		}
	}

	return obj;
}

System::String^ ConvertCommon::ToString(v8::Handle<v8::Value> v8Value)
{
	V8TypeInfo info;

	if (Common::debug)
	{
		TypeCommon::GetV8TypeInfo(v8Value, &info);
	}

	v8::String::Utf8Value utf8(v8Value);

	System::String^ vcUrlString = utf8.length() > 0 ? (gcnew System::String((char*)*utf8)) : System::String::Empty;

	return vcUrlString;
}

inline bool ConvertCommon::ToBoolean(v8::Handle<v8::Value> v8Value)
{
	return v8Value->BooleanValue();
}

inline uint8_t ConvertCommon::ToByte(v8::Handle<v8::Value> v8Value)
{
	uint32_t v = v8Value->Uint32Value();
	return (uint8_t)v;
}

inline int8_t ConvertCommon::ToSByte(v8::Handle<v8::Value> v8Value)
{
	int32_t v = v8Value->Int32Value();
	return (int8_t)v;
}

inline int16_t ConvertCommon::ToInt16(v8::Handle<v8::Value> v8Value)
{
	int32_t v = v8Value->Int32Value();
	return (int16_t)v;
}

inline uint16_t ConvertCommon::ToUInt16(v8::Handle<v8::Value> v8Value)
{
	uint32_t v = v8Value->Uint32Value();
	return (uint16_t)v;
}

inline int32_t ConvertCommon::ToInt32(v8::Handle<v8::Value> v8Value)
{
	return v8Value->Int32Value();
}

inline uint32_t ConvertCommon::ToUInt32(v8::Handle<v8::Value> v8Value)
{
	return v8Value->Uint32Value();
}

inline int64_t ConvertCommon::ToInt64(v8::Handle<v8::Value> v8Value)
{
	return v8Value->IntegerValue();
}

inline uint64_t ConvertCommon::ToUInt64(v8::Handle<v8::Value> v8Value)
{
	if (v8Value->IsUint32())
	{
		return v8Value->Uint32Value();
	}

	return (uint64_t)v8Value->NumberValue();
}

inline float ConvertCommon::ToSingle(v8::Handle<v8::Value> v8Value)
{
	double d = v8Value->NumberValue();
	return (float)d;
}

inline double ConvertCommon::ToDouble(v8::Handle<v8::Value> v8Value)
{
	return v8Value->NumberValue();
}

System::TimeSpan^ ConvertCommon::ToTimeSpan(v8::Handle<v8::Object> v8Value)
{
	Isolate* isolate = Isolate::GetCurrent();

	v8::Handle<v8::Array> keys = v8Value->GetPropertyNames();

	int32_t days = 0;
	int32_t hours = 0;
	int32_t minutes = 0;
	int32_t seconds = 0;
	int32_t milliseconds = 0;

	const int BUFFERSIZE = 20;
	char buffer[BUFFERSIZE];
	for (uint32_t i = 0; i < keys->Length(); ++i)
	{
		v8::Handle<v8::Value> key = keys->Get(i);
		v8::Handle<v8::Value> temp = v8Value->Get(key);

		memset(buffer, 0, BUFFERSIZE);

		if (ConvertCommon::TryGetString(key, buffer, BUFFERSIZE - 1))
		{
			int32_t val = (temp->IsNull() || temp->IsUndefined()) ? 0 : temp->Int32Value();

			if (_strnicmp(TIMESAPN_Days, buffer, strlen(TIMESAPN_Days) == 0))
			{
				days = val;
			}
			else if (_strnicmp(TIMESAPN_Hours, buffer, strlen(TIMESAPN_Hours) == 0))
			{
				hours = val;
			}
			else if (_strnicmp(TIMESAPN_Minutes, buffer, strlen(TIMESAPN_Minutes) == 0))
			{
				minutes = val;
			}
			else if (_strnicmp(TIMESAPN_Seconds, buffer, strlen(TIMESAPN_Seconds) == 0))
			{
				seconds = val;
			}
			else if (_strnicmp(TIMESAPN_Milliseconds, buffer, strlen(TIMESAPN_Milliseconds) == 0))
			{
				milliseconds = val;
			}
		}
	}

	System::TimeSpan^ span = gcnew System::TimeSpan(days, hours, minutes, seconds, milliseconds);

	return span;
}

inline bool ConvertCommon::TryGetString(const v8::Handle<v8::Value> v8Value, char* buffer, size_t length)
{
	v8::Handle<v8::String> str = v8Value->ToString();

	int wCount = str->WriteUtf8(buffer, (int)length);

	return wCount >= 0;
}

void TypeCommon::GetV8TypeInfo(const v8::Handle<v8::Value> v8Value, V8TypeInfo* info)
{
	info->isUndefined = v8Value->IsUndefined();
	info->isNull = v8Value->IsNull();
	info->isTrue = v8Value->IsTrue();
	info->isFalse = v8Value->IsFalse();

	info->isString = v8Value->IsString();
	info->isSymbol = v8Value->IsSymbol();
	info->isFunction = v8Value->IsFunction();
	info->isArray = v8Value->IsArray();
	info->isObject = v8Value->IsObject();
	info->isBoolean = v8Value->IsBoolean();
	info->isNumber = v8Value->IsNumber();
	info->isExternal = v8Value->IsExternal();
	info->isInt32 = v8Value->IsInt32();
	info->isUint32 = v8Value->IsUint32();
	info->isDate = v8Value->IsDate();
	info->isBooleanObject = v8Value->IsBooleanObject();
	info->isNumberObject = v8Value->IsNumberObject();
	info->isStringObject = v8Value->IsStringObject();
	info->isSymbolObject = v8Value->IsSymbolObject();
	info->isNativeError = v8Value->IsNativeError();
	info->isRegExp = v8Value->IsRegExp();
	info->isPromise = v8Value->IsPromise();
	info->isArrayBuffer = v8Value->IsArrayBuffer();
	info->isTypedArray = v8Value->IsTypedArray();
	info->isUint8Array = v8Value->IsUint8Array();
	info->isUint8ClampedArray = v8Value->IsUint8ClampedArray();
	info->isInt8Array = v8Value->IsInt8Array();
	info->isUint16Array = v8Value->IsUint16Array();
	info->isInt16Array = v8Value->IsInt16Array();
	info->isUint32Array = v8Value->IsUint32Array();
	info->isInt32Array = v8Value->IsInt32Array();
	info->isFloat32Array = v8Value->IsFloat32Array();
	info->isFloat64Array = v8Value->IsFloat64Array();
	info->isDataView = v8Value->IsDataView();
}



static void _logToConsole(const int level, const RunningInfo* info)
{
	time_t nowTicks;
	time(&nowTicks);

	struct tm now;
	gmtime_s(&now, &nowTicks);

	printf("[%4d-%02d-%02d %02d:%02d:%02d]:", now.tm_year, now.tm_mon, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);
	printf("[%s]-[%s]:[%s]:[%s]:[%s]", level, info->region, info->file, info->function, info->message);
}

static uint32_t _getObjectMemberUInt32Value(Isolate* isolate, v8::Handle<v8::Object> v8Value, const char* key)
{
	HandleScope scope(isolate);

	v8::Local<v8::Value> k = v8::String::NewFromUtf8(isolate, key);
	v8::Local<v8::Value> value = v8Value->Get(k);

	V8TypeInfo info;
	if (Common::debug)
	{
		TypeCommon::GetV8TypeInfo(value, &info);
	}

	int32_t days = (value->IsNull() || value->IsUndefined()) ? 0 : value->Int32Value();
	return days;
}

v8::Handle<v8::String> _genKeySame(v8::Isolate* isolate,  System::String^ pName)
{
	return ConvertCommon::ToV8String(isolate, pName);
}

v8::Handle<v8::String> _genKeyCamel(v8::Isolate* isolate, System::String^ pName)
{
	msclr::interop::marshal_context context;
	char* utf8 = (char*)context.marshal_as<const char*>(pName);

	if (strlen(utf8) > 0)
	{
		utf8[0] = (char)tolower(utf8[0]);
	}

	return v8::String::NewFromUtf8(isolate, utf8);
}

v8::Handle<v8::String> _genKeyLower(v8::Isolate* isolate, System::String^ pName)
{
	return ConvertCommon::ToV8String(isolate, pName->ToLowerInvariant());
}

static bool _isNullable(System::Type^ t)
{
	return t->FullName->StartsWith("System.Nullable`1[[", StringComparison::Ordinal);
}


void _throw(v8::Isolate* isolate, const char* function, const char* message)
{
	HandleScope scope(isolate);

	isolate->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(isolate, message)));
	return;
}

v8::Handle<v8::Value> _convertNetObject(v8::Isolate* isolate, System::Object^ v, System::Type^ vType, GenV8ObjectKey genKey)
{
	TypeCode code = System::Type::GetTypeCode(vType);

	v8::Handle<v8::Value> v8Value = v8::Handle<v8::Value>();
	System::Type^ temp = nullptr;
	switch (code)
	{

	case System::TypeCode::DBNull:
	case System::TypeCode::Empty:
		_throw(isolate, __FUNCTION__, "Property Type for is empty/dbnull");
		break;
	case System::TypeCode::Object:

		if ((temp = vType->GetInterface("IEnumerable`1", false)) != nullptr)
		{
			System::Type^ innerType = vType->GetGenericArguments()[0];
			if (innerType == System::Object::typeid)
			{
				msclr::interop::marshal_context context;
				const char* typeName = (const char*)context.marshal_as<const char*>(System::String::Format("Can't convert type [{0}] to v8 array!", innerType->FullName));
				LOG(LogLevel::Fatal, typeName);
			}
			else
			{
				if (v == nullptr)
				{
					v8Value = v8::Array::New(isolate, 0);
				}
				else
				{
					std::vector<v8::Handle<v8::Value>> items;
					System::Collections::IEnumerator^ it = ((System::Collections::IEnumerable^)v)->GetEnumerator();

					while (it->MoveNext())
					{
						v8::Handle<v8::Value> itemValue = _convertNetObject(isolate, it->Current, innerType, genKey);
						items.push_back(itemValue);
					}

					v8::Handle<v8::Array> arr = v8::Array::New(isolate, (int)items.size());

					int j = 0;
					std::vector<v8::Handle<v8::Value>>::iterator _it = items.begin();
					for (; _it != items.end(); ++_it)
					{
						v8::Handle<v8::Value> cur = *_it;

						arr->Set(j++, cur);
					}

					v8Value = arr;
				}
			}
		}
		else
		{
			v8Value = ConvertCommon::ToV8Object(isolate, v, genKey);
		}
		break;

	case System::TypeCode::Boolean:
		v8Value = ConvertCommon::ToV8Boolean(isolate, (bool)(v));
		break;

	case System::TypeCode::Char:
		v8Value = ConvertCommon::ToV8Number(isolate, (char)v);
		break;

	case System::TypeCode::SByte:
		v8Value = ConvertCommon::ToV8Number(isolate, (int8_t)v);
		break;

	case System::TypeCode::Byte:
		v8Value = ConvertCommon::ToV8Number(isolate, (uint8_t)v);
		break;

	case System::TypeCode::Int16:
		v8Value = ConvertCommon::ToV8Number(isolate, (int16_t)v);
		break;

	case System::TypeCode::UInt16:
		v8Value = ConvertCommon::ToV8Number(isolate, (uint16_t)v);
		break;

	case System::TypeCode::Int32:
		v8Value = ConvertCommon::ToV8Number(isolate, (int32_t)v);
		break;

	case System::TypeCode::UInt32:
		v8Value = ConvertCommon::ToV8Number(isolate, (uint32_t)v);
		break;

	case System::TypeCode::Int64:
		v8Value = ConvertCommon::ToV8Number(isolate, (int64_t)v);
		break;

	case System::TypeCode::UInt64:
		v8Value = ConvertCommon::ToV8Number(isolate, (uint64_t)v);
		break;

	case System::TypeCode::Single:
		v8Value = ConvertCommon::ToV8Number(isolate, (float)v);
		break;

	case System::TypeCode::Double:
		v8Value = ConvertCommon::ToV8Number(isolate, (double)v);
		break;

	case System::TypeCode::Decimal:
		v8Value = ConvertCommon::ToV8Number(isolate, (double)v);
		break;

	case System::TypeCode::DateTime:
		v8Value = ConvertCommon::ToV8String(isolate, ((System::DateTime^)v)->ToString("O", CultureInfo::InvariantCulture));
		break;

	case System::TypeCode::String:
		v8Value = ConvertCommon::ToV8String(isolate, ((System::String^)v));

		break;
	default:
		msclr::interop::marshal_context context;
		const char* msg = context.marshal_as<const char*>(System::String::Format("Unknown TypeCode = {0}",code.ToString()));
		LOG(LogLevel::Warnning, msg);
		break;
	}

	return v8Value;
}