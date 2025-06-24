/*
 * @File	  : http_util.hpp
 * @Coding	  : utf-8
 * @Author    : Bing
 * @Time      : 2023/03/19 14:02
 * @Introduce : HTTP工具相关
*/


#pragma once

#include <memory>
#include <nlohmann/json.hpp>

#ifndef HTTP_UTIL
#define HTTP_UTIL  

using json = nlohmann::json;

class HTTPUtil final{
public:
    class StatusCodeHandle final{
    public:
        enum status {
            NotExistThisStatusCode = 0,
            Continue = 100,
            SwitchingProtocol = 101,
            Processing = 102,
            EarlyHints = 103,
            OK = 200,
            Created = 201,
            Accepted = 202,
            Non_AuthoritativeInformation = 203,
            NoContent = 204,
            ResetContent = 205,
            PartialContent = 206,
            Multi_Status = 207,
            AlreadyReported = 208,
            IMUsed = 226,
            MultipleChoices = 300,
            MovedPermanently = 301,
            Found = 302,
            SeeOther = 303,
            NotModified = 304,
            TemporaryRedirect = 307,
            PermanentRedirect = 308,
            BadRequest = 400,
            Unauthorized = 401,
            PaymentRequired = 402,
            Forbidden = 403,
            NotFound = 404,
            MethodNotAllowed = 405,
            NotAcceptable = 406,
            ProxyAuthenticationRequired = 407,
            RequestTimeout = 408,
            Conflict = 409,
            Gone = 410,
            LengthRequired = 411,
            PreconditionFailed = 412,
            ContentTooLarge = 413,
            URITooLong = 414,
            UnsupportedMediaType = 415,
            RangeNotSatisfiable = 416,
            ExpectationFailed = 417,
            IAmATeapot = 418,
            MisdirectedRequest = 421,
            UnprocessableContent = 422,
            Locked = 423,
            FailedDependency = 424,
            TooEarly = 425,
            UpgradeRequired = 426,
            PreconditionRequired = 428,
            TooManyRequests = 429,
            RequestHeaderFieldsTooLarge = 431,
            UnavailableForLegalReasons = 451,
            InternalServerError = 500,
            NotImplemented = 501,
            BadGateway = 502,
            ServiceUnavailable = 503,
            GatewayTimeout = 504,
            HTTPVersionNotSupported = 505,
            VariantAlsoNegotiates = 506,
            InsufficientStorage = 507,
            LoopDetected = 508,
            NotExtended = 510,
            NetworkAuthenticationRequired = 511
        };

        const inline static json getJsonResult(status state, std::string_view msg = "", uint16_t status_code = 0) {
            auto now{ std::chrono::system_clock::now() };
            std::time_t t = std::chrono::system_clock::to_time_t(std::move(now));
            std::tm local_tm = *std::localtime(&t);
            std::ostringstream oss;
            oss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S");

            json result;
            result["date"] = oss.str();
            result["code"] = state;

            if (status_code) {
                result["status"] = status_code;
            }

            if (!msg.empty()) {
                result["detail"] = msg;
            }

            switch (state)
            {
            case BadRequest:
                result["message"] = "Bad Request";
                break;
            case Unauthorized:
                result["message"] = "Unauthorized";
                break;
            case PaymentRequired:
                result["message"] = "Payment Required";
                break;
            case Forbidden:
                result["message"] = "Forbidden";
                break;
            case NotFound:
                result["message"] = "Not Found";
                break;
            case MethodNotAllowed:
                result["message"] = "Method Not Allowed";
                break;
            case NotAcceptable:
                result["message"] = "Not Acceptable";
                break;
            case ProxyAuthenticationRequired:
                result["message"] = "Proxy Authentication Required";
                break;
            case RequestTimeout:
                result["message"] = "Request Timeout";
                break;
            case Conflict:
                result["message"] = "Conflict";
                break;
            case Gone:
                result["message"] = "Gone";
                break;
            case LengthRequired:
                result["message"] = "Length Required";
                break;
            case PreconditionFailed:
                result["message"] = "Precondition Failed";
                break;
            case ContentTooLarge:
                result["message"] = "Content Too Large";
                break;
            case URITooLong:
                result["message"] = "URI Too Long";
                break;
            case UnsupportedMediaType:
                result["message"] = "Unsupported Media Type";
                break;
            case RangeNotSatisfiable:
                result["message"] = "Range Not Satisfiable";
                break;
            case ExpectationFailed:
                result["message"] = "Expectation Failed";
                break;
            case IAmATeapot:
                result["message"] = "I'm a teapot";
                break;
            case MisdirectedRequest:
                result["message"] = "Misdirected Request";
                break;
            case UnprocessableContent:
                result["message"] = "Unprocessable Content";
                break;
            case Locked:
                result["message"] = "Locked";
                break;
            case FailedDependency:
                result["message"] = "Failed Dependency";
                break;
            case TooEarly:
                result["message"] = "Too Early";
                break;
            case UpgradeRequired:
                result["message"] = "Upgrade Required";
                break;
            case PreconditionRequired:
                result["message"] = "Precondition Required";
                break;
            case TooManyRequests:
                result["message"] = "Too Many Requests";
                break;
            case RequestHeaderFieldsTooLarge:
                result["message"] = "Request Header Fields Too Large";
                break;
            case UnavailableForLegalReasons:
                result["message"] = "Unavailable For Legal Reasons";
                break;
            case InternalServerError:
                result["message"] = "Internal Server Error";
                break;
            case NotImplemented:
                result["message"] = "Not Implemented";
                break;
            case BadGateway:
                result["message"] = "Bad Gateway";
                break;
            case ServiceUnavailable:
                result["message"] = "Service Unavailable";
                break;
            case GatewayTimeout:
                result["message"] = "Gateway Timeout";
                break;
            case HTTPVersionNotSupported:
                result["message"] = "HTTP Version Not Supported";
                break;
            case VariantAlsoNegotiates:
                result["message"] = "Variant Also Negotiates";
                break;
            case InsufficientStorage:
                result["message"] = "Insufficient Storage";
                break;
            case LoopDetected:
                result["message"] = "Loop Detected";
                break;
            case NotExtended:
                result["message"] = "Not Extended";
                break;
            case NetworkAuthenticationRequired:
                result["message"] = "Network Authentication Required";
                break;
            default:
                result["message"] = "Not Exist This Status Code";
                break;
            }
            return result;
        };

        constexpr inline static status getStatus(int code) {
            switch (code)
            {
            case Continue:
                return Continue;
            case SwitchingProtocol:
                return SwitchingProtocol;
            case Processing:
                return Processing;
            case EarlyHints:
                return EarlyHints;
            case OK:
                return OK;
            case Created:
                return Created;
            case Accepted:
                return Accepted;
            case Non_AuthoritativeInformation:
                return Non_AuthoritativeInformation;
            case NoContent:
                return NoContent;
            case ResetContent:
                return ResetContent;
            case PartialContent:
                return PartialContent;
            case Multi_Status:
                return Multi_Status;
            case AlreadyReported:
                return AlreadyReported;
            case IMUsed:
                return IMUsed;
            case MultipleChoices:
                return MultipleChoices;
            case MovedPermanently:
                return MovedPermanently;
            case Found:
                return Found;
            case SeeOther:
                return SeeOther;
            case NotModified:
                return NotModified;
            case TemporaryRedirect:
                return TemporaryRedirect;
            case PermanentRedirect:
                return PermanentRedirect;
            case BadRequest:
                return BadRequest;
            case Unauthorized:
                return Unauthorized;
            case PaymentRequired:
                return PaymentRequired;
            case Forbidden:
                return Forbidden;
            case NotFound:
                return NotFound;
            case MethodNotAllowed:
                return MethodNotAllowed;
            case NotAcceptable:
                return NotAcceptable;
            case ProxyAuthenticationRequired:
                return ProxyAuthenticationRequired;
            case RequestTimeout:
                return RequestTimeout;
            case Conflict:
                return Conflict;
            case Gone:
                return Gone;
            case LengthRequired:
                return LengthRequired;
            case PreconditionFailed:
                return PreconditionFailed;
            case ContentTooLarge:
                return ContentTooLarge;
            case URITooLong:
                return URITooLong;
            case UnsupportedMediaType:
                return UnsupportedMediaType;
            case RangeNotSatisfiable:
                return RangeNotSatisfiable;
            case ExpectationFailed:
                return ExpectationFailed;
            case IAmATeapot:
                return IAmATeapot;
            case MisdirectedRequest:
                return MisdirectedRequest;
            case UnprocessableContent:
                return UnprocessableContent;
            case Locked:
                return Locked;
            case FailedDependency:
                return FailedDependency;
            case TooEarly:
                return TooEarly;
            case UpgradeRequired:
                return UpgradeRequired;
            case PreconditionRequired:
                return PreconditionRequired;
            case TooManyRequests:
                return TooManyRequests;
            case RequestHeaderFieldsTooLarge:
                return RequestHeaderFieldsTooLarge;
            case UnavailableForLegalReasons:
                return UnavailableForLegalReasons;
            case InternalServerError:
                return InternalServerError;
            case NotImplemented:
                return NotImplemented;
            case BadGateway:
                return BadGateway;
            case ServiceUnavailable:
                return ServiceUnavailable;
            case GatewayTimeout:
                return GatewayTimeout;
            case HTTPVersionNotSupported:
                return HTTPVersionNotSupported;
            case VariantAlsoNegotiates:
                return VariantAlsoNegotiates;
            case InsufficientStorage:
                return InsufficientStorage;
            case LoopDetected:
                return LoopDetected;
            case NotExtended:
                return NotExtended;
            case NetworkAuthenticationRequired:
                return NetworkAuthenticationRequired;
            default:
                return NotExistThisStatusCode;
            }
        };

        const inline static json getSimpleJsonResult(int code, std::string_view msg = "", uint16_t status_code = 0) {
            return getJsonResult(getStatus(code), msg, status_code);
        };
    private:
        StatusCodeHandle(void) = delete;
        ~StatusCodeHandle(void) = delete;
        StatusCodeHandle(const StatusCodeHandle&) = delete;
        StatusCodeHandle(StatusCodeHandle&&) = delete;
        StatusCodeHandle& operator=(const StatusCodeHandle&) = delete;
        StatusCodeHandle& operator=(StatusCodeHandle&&) = delete;
    };
private:

};

#endif