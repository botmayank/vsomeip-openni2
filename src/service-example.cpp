#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <thread>
#include <vsomeip/vsomeip.hpp>
#include "grabber.hpp"

#define SAMPLE_SERVICE_ID 0x1234
#define SAMPLE_INSTANCE_ID 0x5678
#define SAMPLE_METHOD_ID 0x0421

#define SAMPLE_EVENTGROUP_ID 0x0666
#define SAMPLE_EVENT_ID 0x0667

// #define PUB_SUB
#define VSOMEIP_ENABLE_SIGNAL_HANDLING

std::shared_ptr <vsomeip::application> app;
Grabber grabber;

void on_message(const std::shared_ptr<vsomeip::message> &_request){

    std::shared_ptr <vsomeip::payload> its_payload = _request->get_payload();
    vsomeip::length_t l = its_payload->get_length();

    //Get payload
    std::stringstream ss;
    for(vsomeip::length_t i=0; i<l; i++) {
        ss << std::setw(2) << std::setfill('0') << std::hex
           << (int)*(its_payload->get_data()+i) << " ";
    }

    std::cout << "SERVICE: Received message with Client/Session ["
            << std::setw(4) << std::setfill('0') << std::hex << _request->get_client() << "/"
            << std::setw(4) << std::setfill('0') << std::hex << _request->get_session() << "] "
            << ss.str() << std::endl; 
	    // << "requesting Gain" << std::endl;

    //Create response
    std::shared_ptr <vsomeip::message> its_response = vsomeip::runtime::get()->create_response(_request);
    its_payload = vsomeip::runtime::get()->create_payload();
    // std::vector <vsomeip::byte_t> its_payload_data;

    std::cout << "Creating response" << std::endl;
    std::ifstream file("cat.jpg", std::ios::binary);

    // read the data:
    std::vector<vsomeip::byte_t> its_payload_data((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
    std::cout<< "Sending file contents" << std::endl;

    // for(int i=13; i>=0; i--){
    //     its_payload_data.push_back(i % 256);
    // }

    // Fetch Camera gain and send back
    // int gain = grabber.getCameraGain();
    // its_payload_data.push_back(gain);

    
    its_payload->set_data(its_payload_data);
    its_response->set_payload(its_payload);
    app->send(its_response, true);
}

void notify_event(){
    vsomeip::byte_t its_data[] = {0x10};
    std::shared_ptr <vsomeip::payload> payload = vsomeip::runtime::get()->create_payload();
    
    while(true){
        its_data[0]++;
        payload->set_data(its_data, sizeof(its_data));
        std::cout<<"Sending data!"<< std::endl;
        app->notify(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID, SAMPLE_EVENT_ID, payload);
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000)); 
    }
    
} 
int main(){
    std::cout << "Starting Service example" << std::endl;

    app = vsomeip::runtime::get()->create_application("World");
    app->init();
    app->register_message_handler(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID, SAMPLE_METHOD_ID, on_message);
    app->offer_service(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID);
       
   /* grabber.InitOpenNI();    
    grabber.InitDevice();
    grabber.InitDepthStream();
    grabber.InitColorStream();
    std::cout<< "Initialized Camera Streams!" << std::endl;
    */

#ifdef PUB_SUB
    std::set<vsomeip::eventgroup_t> its_groups;
    its_groups.insert(SAMPLE_EVENTGROUP_ID);
    app->offer_event(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID, SAMPLE_EVENT_ID, its_groups, true);
    std::cout<< "Offered event, set to notify with payload" << std::endl; 
   
    std::thread notifier(notify_event);
#endif
    app->start();
    
}
