#include <iostream>
#include <random>
#include "TrafficLight.h"

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a
    std::unique_lock<std::mutex> lock(_mutex);
    _cond.wait(lock, [this] { return !_queue.empty(); });
    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.push_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */
 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b
    while (true)
    {
        if (TrafficLightPhase::green == _messageQueue.receive())
            return;
        
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b
    _threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a
    // took inspiration from these repos to implement this method, I was stuck on it for a 
    // while, had issues with finding the right methods for timestamping and cycle duration
    // of each Phase (red, green) and also how to randomize the cycleDuration
    // https://github.com/patrickjmcgoldrick/CppND-Concurrent-Traffic-Simulation/blob/master/src/TrafficLight.cpp
    //https://github.com/FarruhShahidi/CppND-Concurrent-Traffic-Simulation/blob/master/src/TrafficLight.cpp
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distribution(2000.0, 9000.0);
    auto timestamp = std::chrono::system_clock::now();
    double cycleDuration = distribution(gen); 

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - timestamp).count();
        
        if (elapsed >= cycleDuration)
        {
            _currentPhase = 
                (_currentPhase == TrafficLightPhase::green) ? TrafficLightPhase::red : TrafficLightPhase::green;
    
            _messageQueue.send(std::move(_currentPhase));
            timestamp = std::chrono::system_clock::now();
        }
    }
}
