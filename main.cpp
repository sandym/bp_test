#include <random>
#include <iostream>
#include <thread>
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/process.hpp>

namespace bp = boost::process;

int nbOfNormalExitAnnounced = 0;
int nbOfCrashAnnounced = 0;
int nbOfErrorExitAnnounced = 0;

int nbOfNormalExitReceived = 0;
int nbOfCrashReceived = 0;
int nbOfErrorExitReceived = 0;

std::vector<std::string> split_lines(const std::string &s)
{
	std::vector<std::string> result;
	std::string::size_type a = 0;
	for (std::string::size_type i = 0; i < s.length(); ++i)
	{
		if (s[i] == '\n' or s[i] == '\r' )
		{
			if ((i - a) > 0)
				result.emplace_back(s.substr(a, i - a));
			a = i + 1;
		}
	}
	if ((s.length() - a) > 0)
		result.emplace_back(s.substr(a, s.length() - a));
	return result;
}

struct Child
{
	Child(const std::string &exe, int i_token, boost::asio::io_service &ios) :
		token(i_token),
		buf(32),
		ap(ios),
		child_process(
			exe, "--child",
			std::to_string(token),
			bp::std_out > ap,
			bp::on_exit = [this](int exit, const std::error_code& ec)
			{
				this->on_exit(exit, ec);
			},
			ios)
	{
		startRead();
	}

	void on_exit(int exit, const std::error_code& ec)
	{
		// split lines
		auto lines = split_lines(result);
		if (lines.empty())
		{
			std::cout << "child " << token << " did not produce any output" << std::endl;
			return;
		}
		
		if (lines.back() == "exit")
			++nbOfNormalExitAnnounced;
		if (lines.back() == "error")
			++nbOfErrorExitAnnounced;
		if (lines.back() == "crash")
			++nbOfCrashAnnounced;

		// last line should match exit mode
		
		// how do I detect the crash here !? signal and exit code get mixed up
		
		switch (exit)
		{
		case 255:
			++nbOfErrorExitReceived;
			if (lines.back() != "error")
				std::cout << "child " << token << " unexpected exit code" << std::endl;
			break;
		case 0:
			++nbOfNormalExitReceived;
			if (lines.back() != "exit")
				std::cout << "child " << token << " unexpected exit code" << std::endl;
			break;
		default:
			std::cout << "child " << token << " unexpected exit code : " << exit << std::endl;
			break;
		}
		lines.pop_back();

		// all lines should match token
		if (lines.empty())
		{
			std::cout << "child " << token << " no output" << std::endl;
		}
		for (auto &it : lines)
		{
			if (it != std::to_string(token))
			{
				std::cout << "child " << token << " unexpected outout" << std::endl;
				break;
			}
		}

	}
	void startRead()
	{
		boost::asio::async_read(
		    ap,
		    boost::asio::buffer( buf ),
		    [this]( const boost::system::error_code &ec, std::size_t s ) {
			    this->read( ec, s );
		    } );
	}

	void read( const boost::system::error_code &ec, std::size_t s )
	{
		result.append(buf.begin(), buf.begin() + s);
		if ( s > 0 )
			startRead();
	}

	int token;
	std::vector<char> buf;
	std::string result;
	bp::async_pipe ap;
	bp::child child_process;
};

void output_sleep( const std::string &s )
{
	std::cout << s << std::endl;
	std::this_thread::sleep_for( std::chrono::milliseconds(100) );
}

int main( int argc, char **argv )
{
	std::random_device rand;

	if ( argc > 2 and strcmp( argv[1], "--child" ) == 0 )
	{
		// a child process
		std::this_thread::sleep_for( std::chrono::seconds(1) );

		// 1- output the token for a while
		const char *token = argv[2];
		int nb = 3 + (rand()%10);
		for ( int i = 0; i < nb; ++i )
			output_sleep( token );

		// 2- should we crash, error or just exit?
		// switch ( rand()%3 ) crash detection does not work ?
		switch ( rand()%2 )
		{
			case 0:
				output_sleep( "error" );
				return -1;
				break;
			case 1:
				output_sleep("exit");
				break;
			default:
				output_sleep("crash");
				abort();
				break;
		}
		return 0;
	}
	
	// parent process

	int nbOfChildren = 1;
	if ( argc > 1 )
		nbOfChildren = std::max( atoi( argv[1] ), 1 );

	std::cout << "nb of children to start: " << nbOfChildren << std::endl;

	boost::asio::io_context ios;

	std::vector<std::unique_ptr<Child>> children;
	for ( int i = 0; i < nbOfChildren; ++i )
	{
		//std::cout << "staring child with token: " << i << std::endl;
		children.emplace_back( new Child( argv[0], i, ios ) );
	}

	ios.run();

	for ( auto &it : children )
		it->child_process.wait();

	std::cout << "nbOfNormalExitAnnounced = " << nbOfNormalExitAnnounced << std::endl;
	std::cout << "nbOfNormalExitReceived = " << nbOfNormalExitReceived << std::endl;

	std::cout << "nbOfErrorExitAnnounced = " << nbOfErrorExitAnnounced << std::endl;
	std::cout << "nbOfErrorExitReceived = " << nbOfErrorExitReceived << std::endl;

	std::cout << "nbOfCrashAnnounced = " << nbOfCrashAnnounced << std::endl;
	std::cout << "nbOfCrashReceived = " << nbOfCrashReceived << std::endl;

	std::cout << "done" << std::endl;
	
	assert( nbOfNormalExitAnnounced == nbOfNormalExitReceived );
	assert( nbOfErrorExitAnnounced == nbOfErrorExitReceived );
	assert( nbOfCrashAnnounced == nbOfCrashReceived );
}
