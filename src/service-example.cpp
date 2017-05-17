#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vsomeip/vsomeip.hpp>
#include "grabber.hpp"

#define SAMPLE_SERVICE_ID 0x1234
#define SAMPLE_INSTANCE_ID 0x5678
#define SAMPLE_METHOD_ID 0x0421

#define SAMPLE_EVENTGROUP_ID 0x0666
#define SAMPLE_EVENT_ID 0x0667

// #define PUB_SUB

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
            << ss.str() << "requesting Gain" << std::endl;

    //Create response
    std::shared_ptr <vsomeip::message> its_response = vsomeip::runtime::get()->create_response(_request);
    its_payload = vsomeip::runtime::get()->create_payload();
    std::vector <vsomeip::byte_t> its_payload_data;

    // std::ifstream sample_text("example.txt", std::ios::in | std::ios::binary);
    // std::vector <vsomeip::byte_t> its_payload_data((std::istreambuf_iterator<vsomeip::byte_t>(sample_text)),
            // std::istreambuf_iterator<vsomeip::byte_t>());

    // for(int i=9; i>=0; i--){
    //     its_payload_data.push_back(i % 256);
    // }

    //Fetch Camera gain and send back
    int gain = grabber.getCameraGain();
    its_payload_data.push_back(gain);

    its_payload->set_data(its_payload_data);

    its_response->set_payload(its_payload);
    app->send(its_response, true);
}

int main(){
    std::cout << "Starting Service example" << std::endl;

    app = vsomeip::runtime::get()->create_application("Jhingalala");
    app->init();
    app->register_message_handler(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID, SAMPLE_METHOD_ID, on_message);
    app->offer_service(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID);
       
    grabber.InitOpenNI();    
    grabber.InitDevice();
    grabber.InitDepthStream();
    grabber.InitColorStream();
    std::cout<< "Initialized Camera Streams!" << std::endl;

#ifdef PUB_SUB
    const vsomeip::byte_t its_data[] = {0x10};
    std::shared_ptr <vsomeip::payload> payload = vsomeip::runtime::get()->create_payload();
    payload->set_data(its_data, sizeof(its_data));

    std::set<vsomeip::eventgroup_t> its_groups;
    its_groups.insert(SAMPLE_EVENTGROUP_ID);
    app->offer_event(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID, SAMPLE_EVENT_ID, its_groups, true);
    app->notify(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID, SAMPLE_EVENT_ID, payload);
#endif
    
    app->start();
}