#include <v8.h>
#include <node.h>
#include <gcroot.h>
#include <map>

using namespace v8;

namespace Node2ClrCommon {

	struct V8TypeInfo;
	struct RunningInfo;
	
	typedef void(*Logger)(const int level, const RunningInfo* info);
	typedef v8::Handle<v8::String>(*GenV8ObjectKey)(v8::Isolate* isolate, System::String^ pName);

	extern GenV8ObjectKey GenV8ObjectKey_LOWER;
	extern GenV8ObjectKey GenV8ObjectKey_CAMEL;
	extern GenV8ObjectKey GenV8ObjectKey_SAME;

#define LOG(level,msg) RunningInfo info; \
	info.file = __FILE__; \
	info.function = __FUNCSIG__; \
	info.message = (msg); \
	Common::logger((level), &info); \

	enum LogLevel{
		Fatal = 0,
		Error = 1,
		Warnning = 2,
		Debug = 3,
		Info = 4
	};

	struct RunningInfo{
		const char* region;
		const char* file;
		const char* function;
		const char* message;
	};

	enum MatchClrSignatureResultCode
	{
		ResultCode_Success = 0,
		ResultCode_StartOutOfRange = 1,
		ResultCode_CountOutOfRange = 2,
		ResultCode_MismatchCount = 3,
		ResultCode_TooManyArguments = 4,

		ResultCode_Mismatch = 128,
		ResultCode_Mismatch1,
		ResultCode_Mismatch2,
		ResultCode_Mismatch3,
		ResultCode_Mismatch4,
		ResultCode_Mismatch5,
		ResultCode_Mismatch6,
		ResultCode_Mismatch7,
		ResultCode_Mismatch8,
		ResultCode_Mismatch9,
		ResultCode_Mismatch10,
		ResultCode_MismatchMax
	};

	class Common{
	public:
		static bool debug;
		static Logger logger;
		static Logger CONSOLELOGGER;
	};

	class TypeCommon{
	private:

	public:
		static void GetV8TypeInfo(const v8::Handle<v8::Value> value, V8TypeInfo* info);

		inline static MatchClrSignatureResultCode GetMismatchParameterCode(int i);
		inline static int GetMismatchParameterIndex(MatchClrSignatureResultCode code);

		static bool IsTimeSpan(v8::Handle<v8::Object> v8Value);
		static bool IsMatch(v8::Handle<v8::Value> v8Value, System::Type^ clrType);
		static MatchClrSignatureResultCode IsMatchClrSignature(Isolate* isolate, const v8::FunctionCallbackInfo<v8::Value>& args, int start, int count, array<System::Type^, 1>^ signature);
	};

	class ConvertCommon{
	public:

		static v8::Handle<v8::String> ToV8String(v8::Isolate* isolate, System::String^ value);
		static v8::Handle<v8::String> ToV8String(v8::Isolate* isolate, const char* value);

		static v8::Handle<v8::Integer> ToV8Number(v8::Isolate* isolate, int8_t value);
		static v8::Handle<v8::Integer> ToV8Number(v8::Isolate* isolate, uint8_t value);
		static v8::Handle<v8::Integer> ToV8Number(v8::Isolate* isolate, int16_t value);
		static v8::Handle<v8::Integer> ToV8Number(v8::Isolate* isolate, uint16_t value);
		static v8::Handle<v8::Integer> ToV8Number(v8::Isolate* isolate, int32_t value);
		static v8::Handle<v8::Integer> ToV8Number(v8::Isolate* isolate, uint32_t value);
		static v8::Handle<v8::Number> ToV8Number(v8::Isolate* isolate, int64_t value);
		static v8::Handle<v8::Number> ToV8Number(v8::Isolate* isolate, uint64_t value);
		static v8::Handle<v8::Number> ToV8Number(v8::Isolate* isolate, float value);
		static v8::Handle<v8::Number> ToV8Number(v8::Isolate* isolate, double value);
		static v8::Handle<v8::Boolean> ToV8Boolean(v8::Isolate* isolate, bool value);

		static v8::Handle<v8::Object> ToV8Object(v8::Isolate* isolate, std::map<const char*, v8::Handle<v8::Value>> value);
		static v8::Handle<v8::Object> ToV8Object(v8::Isolate* isolate, System::TimeSpan^ value);
		static v8::Handle<v8::Object> ToV8Object(v8::Isolate* isolate, System::Object^ value, GenV8ObjectKey genKey);

		static System::String^ ToString(v8::Handle<v8::Value> v8Value);
		static bool ToBoolean(v8::Handle<v8::Value> v8Value);
		static uint8_t ToByte(v8::Handle<v8::Value> v8Value);
		static int8_t ToSByte(v8::Handle<v8::Value> v8Value);
		static int16_t ToInt16(v8::Handle<v8::Value> v8Value);
		static uint16_t ToUInt16(v8::Handle<v8::Value> v8Value);
		static int32_t ToInt32(v8::Handle<v8::Value> v8Value);
		static uint32_t ToUInt32(v8::Handle<v8::Value> v8Value);
		static int64_t ToInt64(v8::Handle<v8::Value> v8Value);
		static uint64_t ToUInt64(v8::Handle<v8::Value> v8Value);
		static float ToSingle(v8::Handle<v8::Value> v8Value);
		static double ToDouble(v8::Handle<v8::Value> v8Value);
		static System::TimeSpan^ ToTimeSpan(v8::Handle<v8::Object> v8Value);

		static bool TryGetString(const v8::Handle<v8::Value> v8Value, char* buffer, size_t length);

	};

	struct V8TypeInfo{
		bool isUndefined;
		bool isNull;
		bool isTrue;
		bool isFalse;
		bool isString;
		bool isSymbol;
		bool isFunction;
		bool isArray;
		bool isObject;
		bool isBoolean;
		bool isNumber;
		bool isExternal;
		bool isInt32;
		bool isUint32;
		bool isDate;
		bool isBooleanObject;
		bool isNumberObject;
		bool isStringObject;
		bool isSymbolObject;
		bool isNativeError;
		bool isRegExp;
		bool isPromise;
		bool isArrayBuffer;
		bool isTypedArray;
		bool isUint8Array;
		bool isUint8ClampedArray;
		bool isInt8Array;
		bool isUint16Array;
		bool isInt16Array;
		bool isUint32Array;
		bool isInt32Array;
		bool isFloat32Array;
		bool isFloat64Array;
		bool isDataView;
	};
}