#pragma once

#include <common/exception/exceptions.h>
#include <common/utility/string.h>

#include <string>

#pragma warning(push, 1)

extern "C" 
{
#include <libavutil/error.h>
}

#pragma warning(pop)

namespace caspar { namespace ffmpeg {

struct ffmpeg_error : virtual caspar_exception{};
struct averror_bsf_not_found : virtual ffmpeg_error{};
struct averror_decoder_not_found : virtual ffmpeg_error{};
struct averror_demuxer_not_found : virtual ffmpeg_error{};
struct averror_encoder_not_found : virtual ffmpeg_error{};
struct averror_eof : virtual ffmpeg_error{};
struct averror_exit : virtual ffmpeg_error{};
struct averror_filter_not_found : virtual ffmpeg_error{};
struct averror_muxer_not_found : virtual ffmpeg_error{};
struct averror_option_not_found : virtual ffmpeg_error{};
struct averror_patchwelcome : virtual ffmpeg_error{};
struct averror_protocol_not_found : virtual ffmpeg_error{};
struct averror_stream_not_found : virtual ffmpeg_error{};

static std::string av_error_str(int errn)
{
	char buf[256];
	memset(buf, 0, 256);
	if(av_strerror(errn, buf, 256) < 0)
		return "";
	return std::string(buf);
}

static void throw_on_ffmpeg_error(int ret, const char* source, const char* func, const char* local_func, const char* file, int line)
{
	if(ret >= 0)
		return;

	switch(ret)
	{
	case AVERROR_BSF_NOT_FOUND:
		::boost::exception_detail::throw_exception_(averror_bsf_not_found()<<										
			msg_info(av_error_str(ret)) <<							
			source_info(narrow(source)) << 						
			boost::errinfo_api_function(func) <<					
			boost::errinfo_errno(AVUNERROR(ret)), local_func, file, line);	
	case AVERROR_DECODER_NOT_FOUND:
		::boost::exception_detail::throw_exception_(averror_decoder_not_found()<<										
			msg_info(av_error_str(ret)) <<							
			source_info(narrow(source)) << 						
			boost::errinfo_api_function(func) <<					
			boost::errinfo_errno(AVUNERROR(ret)), local_func, file, line);
	case AVERROR_DEMUXER_NOT_FOUND:
		::boost::exception_detail::throw_exception_(averror_demuxer_not_found()<<										
			msg_info(av_error_str(ret)) <<							
			source_info(narrow(source)) << 						
			boost::errinfo_api_function(func) <<					
			boost::errinfo_errno(AVUNERROR(ret)), local_func, file, line);
	case AVERROR_ENCODER_NOT_FOUND:
		::boost::exception_detail::throw_exception_(averror_encoder_not_found()<<										
			msg_info(av_error_str(ret)) <<							
			source_info(narrow(source)) << 						
			boost::errinfo_api_function(func) <<					
			boost::errinfo_errno(AVUNERROR(ret)), local_func, file, line);	
	case AVERROR_EOF:	
		::boost::exception_detail::throw_exception_(averror_eof()<<										
			msg_info(av_error_str(ret)) <<							
			source_info(narrow(source)) << 						
			boost::errinfo_api_function(func) <<					
			boost::errinfo_errno(AVUNERROR(ret)), local_func, file, line);
	case AVERROR_EXIT:				
		::boost::exception_detail::throw_exception_(averror_exit()<<										
			msg_info(av_error_str(ret)) <<							
			source_info(narrow(source)) << 						
			boost::errinfo_api_function(func) <<					
			boost::errinfo_errno(AVUNERROR(ret)), local_func, file, line);
	case AVERROR_FILTER_NOT_FOUND:				
		::boost::exception_detail::throw_exception_(averror_filter_not_found()<<										
			msg_info(av_error_str(ret)) <<							
			source_info(narrow(source)) << 						
			boost::errinfo_api_function(func) <<					
			boost::errinfo_errno(AVUNERROR(ret)), local_func, file, line);
	case AVERROR_MUXER_NOT_FOUND:	
		::boost::exception_detail::throw_exception_(averror_muxer_not_found()<<										
			msg_info(av_error_str(ret)) <<							
			source_info(narrow(source)) << 						
			boost::errinfo_api_function(func) <<					
			boost::errinfo_errno(AVUNERROR(ret)), local_func, file, line);
	case AVERROR_OPTION_NOT_FOUND:	
		::boost::exception_detail::throw_exception_(averror_option_not_found()<<										
			msg_info(av_error_str(ret)) <<							
			source_info(narrow(source)) << 						
			boost::errinfo_api_function(func) <<					
			boost::errinfo_errno(AVUNERROR(ret)), local_func, file, line);
	case AVERROR_PATCHWELCOME:	
		::boost::exception_detail::throw_exception_(averror_patchwelcome()<<										
			msg_info(av_error_str(ret)) <<							
			source_info(narrow(source)) << 						
			boost::errinfo_api_function(func) <<					
			boost::errinfo_errno(AVUNERROR(ret)), local_func, file, line);
	case AVERROR_PROTOCOL_NOT_FOUND:	
		::boost::exception_detail::throw_exception_(averror_protocol_not_found()<<										
			msg_info(av_error_str(ret)) <<							
			source_info(narrow(source)) << 						
			boost::errinfo_api_function(func) <<					
			boost::errinfo_errno(AVUNERROR(ret)), local_func, file, line);
	case AVERROR_STREAM_NOT_FOUND:
		::boost::exception_detail::throw_exception_(averror_stream_not_found()<<										
			msg_info(av_error_str(ret)) <<							
			source_info(narrow(source)) << 						
			boost::errinfo_api_function(func) <<					
			boost::errinfo_errno(AVUNERROR(ret)), local_func, file, line);
	default:
		::boost::exception_detail::throw_exception_(ffmpeg_error()<<										
			msg_info(av_error_str(ret)) <<							
			source_info(narrow(source)) << 						
			boost::errinfo_api_function(func) <<					
			boost::errinfo_errno(AVUNERROR(ret)), local_func, file, line);
	}
}

static void throw_on_ffmpeg_error(int ret, const std::wstring& source, const char* func, const char* local_func, const char* file, int line)
{
	throw_on_ffmpeg_error(ret, narrow(source).c_str(), func, local_func, file, line);
}


//#define THROW_ON_ERROR(ret, source, func) throw_on_ffmpeg_error(ret, source, __FUNC__, __FILE__, __LINE__)

#define THROW_ON_ERROR_STR_(call) #call
#define THROW_ON_ERROR_STR(call) THROW_ON_ERROR_STR_(call)

#define THROW_ON_ERROR(ret, func, source) \
		throw_on_ffmpeg_error(ret, source, func, __FUNCTION__, __FILE__, __LINE__);		

#define THROW_ON_ERROR2(call, source)										\
	[&]() -> int															\
	{																		\
		int ret = call;														\
		throw_on_ffmpeg_error(ret, source, THROW_ON_ERROR_STR(call), __FUNCTION__, __FILE__, __LINE__);	\
		return ret;															\
	}()

}}