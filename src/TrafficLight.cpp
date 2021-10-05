#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> unique_lock(_mutex);
    _condition.wait(unique_lock,[this] {return !_queue.empty();});
    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;

}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::lock_guard<std::mutex> lock_guard(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    _messageQueue=std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
        TrafficLightPhase received = _messageQueue->receive();
        if(received==green) break;
    }
    return;
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

// virtual function which is executed in a thread
void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases,this));
}

void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    std::chrono::high_resolution_clock::time_point t1;
    std::chrono::high_resolution_clock::time_point t2;
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<int> dist(4000,6000);
    double cycleDuration = dist(eng);
    t1 = std::chrono::high_resolution_clock::now();
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        t2 = std::chrono::high_resolution_clock::now();
        auto timeBetweenTwoLoopCycles = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if(timeBetweenTwoLoopCycles>=cycleDuration)
        {
            if(_currentPhase==red)
            {
                _currentPhase=green;
            }
            else
            {
                _currentPhase=red;
            }
            t1 = std::chrono::high_resolution_clock::now();
        }
        _messageQueue->send(std::move(_currentPhase));
    }
}
