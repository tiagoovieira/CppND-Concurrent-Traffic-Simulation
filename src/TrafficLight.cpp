#include <iostream>
#include <random>
#include "TrafficLight.h"

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] { return !_queue.empty(); });
    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a
    std::lock_guard<std::mutex> uLock(_mutex);
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
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto message = _messageQueue.receive();

        if (message == TrafficLightPhase::green)
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
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distribution(2000.0, 9000.0);
    auto lastUpdate = std::chrono::system_clock::now();
    double cycleDuration = distribution(gen); 

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        
        if (timeSinceLastUpdate >= cycleDuration)
        {
            _currentPhase = 
                (_currentPhase == TrafficLightPhase::green) ? TrafficLightPhase::red : TrafficLightPhase::green;
    
            _messageQueue.send(std::move(_currentPhase));
            lastUpdate = std::chrono::system_clock::now();
        }
    }
}
