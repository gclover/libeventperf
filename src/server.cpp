
extern "C" {

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

}

#include <iostream>
#include <Poco/SharedPtr.h>

using Poco::SharedPtr;


namespace server {

class Server 
{
public:
	Server(struct event_base* eventbase, int port)
	: eventbase_(eventbase), port_(port) 
	{}

	~Server() 
	{
		evconnlistener_free(listener_);
	}

	bool start() 
	{
		struct sockaddr_in sin = this->create_addr(port_);
		listener_ = evconnlistener_new_bind(eventbase_, &Server::onListener, (void *)eventbase_,
	    						LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
	    						(struct sockaddr*)&sin, sizeof(sin));
		if (!listener_) 
		{
			std::cerr << "Could not create a listener!" << std::endl;
			return false;
		}

		return true;
	}

	bool stop()
	{
	}

private:

	static void onListener(struct evconnlistener *listener, evutil_socket_t fd, 
				struct sockaddr *sa, int socklen, void *user_data)
	{
		struct event_base *base = (struct event_base*)user_data;
		struct bufferevent *bev;

		const char MESSAGE[] = "Hello, World!\n";
		bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
		if (!bev) 
		{
			std::cerr << "Error constructing bufferevent!" << std::endl;
			event_base_loopbreak(base);
			return;
		}
		bufferevent_setcb(bev, NULL, &Server::onWrite, &Server::onEvent, NULL);
		bufferevent_enable(bev, EV_WRITE);
		bufferevent_disable(bev, EV_READ);

		std::cout << "write message" << std::endl;
		bufferevent_write(bev, MESSAGE, strlen(MESSAGE));
	}

	static void onWrite(struct bufferevent *bev, void *user_data)
	{
		struct evbuffer *output = bufferevent_get_output(bev);
		if (evbuffer_get_length(output) == 0) 
		{
			std::cerr << "flushed answer" << std::endl;
			bufferevent_free(bev);
		}
	}

	static void onEvent(struct bufferevent *bev, short events, void *user_data)
	{
		if (events & BEV_EVENT_EOF) 
			std::cout << "Connection closed." << std::endl;
		else if (events & BEV_EVENT_ERROR) 
			std::cout << "Got an error on the connection" << std::endl;
		/* None of the other events can happen here, since we haven't enabled
		 * timeouts */
		bufferevent_free(bev);
	}

	struct sockaddr_in create_addr(int port)
	{
		struct sockaddr_in sin;
		memset(&sin, 0, sizeof(sin));
        	sin.sin_family = AF_INET;
        	sin.sin_port = htons(port_);
		return sin;
	}

	struct evconnlistener *listener_;

	struct event_base *eventbase_ ;
	int port_;
};

class Application
{
public:
	Application()  {}
	~Application() {}

	int run() 
	{	
		int port = 9995;

		struct event_base* eventbase =  event_base_new();
		if (!eventbase)
		{
			std::cerr << "Could not initialize libevent!" << std::endl;
			return 1;
		}

		Server server(eventbase, port);

		struct event* event = evsignal_new(eventbase, SIGINT, &Application::onSignal, (void *)eventbase);
		if (!event || event_add(event, NULL) < 0) {
			std::cerr << "Could not create/add a signal event!" << std::endl;
			return 1;
		}
	
		server.start();

		event_base_dispatch(eventbase);

		server.stop();
		event_free(event);
		event_base_free(eventbase);
		return 0;
	}

private:

	static void onSignal(evutil_socket_t sig, short events, void *user_data)
	{
		struct event_base *base = (struct event_base*)user_data;
		struct timeval delay = { 1, 0 };
		std::cout << "Caught on interrupt signal; exiting cleanly in two seconds" << std::endl;
		event_base_loopexit(base, &delay);
	}

};

}

int main(int argc, char* argv[])
{
	using namespace server;
	Application app;
	return app.run();	
}

