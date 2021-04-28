#ifndef DEBUGGER_HPP
# define DEBUGGER_HPP

# include <iostream>
# include <string>
# include <ctime>
# include <sys/time.h>

# define DEBUG_START(ACTIVATE) Debugger::start(ACTIVATE)
# define DEBUG_COUT(MESSAGE) if (Debugger::_webserv_debugger.get_is_active() == true) std::cout << "[" << Debugger::get_date() << "] " << MESSAGE << std::endl

class Debugger
{

	public:

		Debugger();
		~Debugger();

		static void 		start(bool activate);
		bool				get_is_active();
		static std::string	get_date();
		static Debugger 	_webserv_debugger;

	private:
		Debugger( Debugger const & src );
		Debugger &		operator=( Debugger const & rhs );
		bool _is_active;
};

#endif /* ******************************************************** DEBUGGER_H */