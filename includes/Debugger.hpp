#ifndef DEBUGGER_HPP
# define DEBUGGER_HPP

# include <iostream>
# include <string>

# define DEBUG_START(ACTIVATE) Debugger::start(ACTIVATE)
# define DEBUG_COUT(MESSAGE) if (Debugger::_webserv_debugger.get_is_active() == true) std::cout << "DEBUG: " << MESSAGE << std::endl

class Debugger
{

	public:

		Debugger();
		~Debugger();

		static void start(bool activate);
//		static void print_std_out(const std::string str);
		bool		get_is_active();

		static Debugger _webserv_debugger;

	private:
		Debugger( Debugger const & src );
		Debugger &		operator=( Debugger const & rhs );
		bool _is_active;
};

#endif /* ******************************************************** DEBUGGER_H */