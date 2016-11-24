#include <string>
#include <vector>
#include <initializer_list>


class Logger{
public:
	Logger(std::initializer_list<std::string> origin):
		origin_ {origin}
	{}
	template <typename T>
    friend Logger& operator <<(Logger& log, const T& value);
private:
	std::vector<std::string> origin_;
	std::string originSeparator_ = "/";
	std::string lineBeggining = "";
	std::string originMessageSeparator_ = "\t";
};

template<class T>
Logger& operator<< (Logger& log, const T& message)
{
	std::string logLine = log.lineBeggining;
	for(auto& el : log.origin_){
		logLine += el + log.originSeparator_;
	}
	logLine += log.originMessageSeparator_;
	logLine += message;
	fprintf(stderr, "%s\n", logLine.c_str());
 	return log;
}