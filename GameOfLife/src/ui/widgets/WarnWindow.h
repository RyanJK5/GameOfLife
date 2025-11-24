#ifndef __WarnWindow_h__
#define __WarnWindow_h__

#include <string>
#include <string_view>

namespace gol
{
enum class WarnWindowState
{
	None,
	Success,
	Failure
};

class WarnWindow
{
public:
	WarnWindow(std::string_view title, std::string_view message = "")
		: m_Title(title)
		, Message(message)
	{ }

	WarnWindowState Update();

	bool Active = false;
	std::string Message;
private:
	std::string m_Title;
};
}

#endif