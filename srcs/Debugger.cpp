#include "Debugger.hpp"

Debugger Debugger::_webserv_debugger;

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Debugger::Debugger() : _is_active(false) {}

Debugger::Debugger(Debugger const & src) {
	(void)src;
}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Debugger::~Debugger() {}


/*
** --------------------------------- OPERATORS --------------------------------
*/

Debugger &Debugger::operator=( Debugger const & rhs) {
	(void)rhs;
	return *this;
}


/*
** --------------------------------- METHODS ----------------------------------
*/

bool	Debugger::get_is_active() {
	return _is_active;
}

void	Debugger::start(bool activate) {
	_webserv_debugger._is_active = activate;
}

//void	Debugger::print_std_out(std::string str) {
//	if (Debugger::_webserv_debugger.get_is_active() == true) {
//		std::cout << "DEBUG: " << str << std::endl;
//	}
//}


/* ************************************************************************** */