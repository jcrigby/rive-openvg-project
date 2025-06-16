#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>

// Include Rive headers
#include "rive/file.hpp"
#include "rive/animation/linear_animation_instance.hpp"
#include "rive/factory.hpp"
#include "utils/no_op_factory.hpp"

int main(int argc, char* argv[]) {
    std::string riveFile = "fire_button.riv";
    if (argc > 1) {
        riveFile = argv[1];
    }
    
    std::cout << "Rive Console Performance Benchmark" << std::endl;
    std::cout << "Loading: " << riveFile << std::endl;
    
    try {
        // Load the .riv file
        std::ifstream file(riveFile, std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open Rive file: " << riveFile << std::endl;
            return -1;
        }
        
        file.seekg(0, std::ios::end);
        size_t length = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<uint8_t> bytes(length);
        file.read(reinterpret_cast<char*>(bytes.data()), length);
        file.close();
        
        // Import the Rive file
        rive::NoOpFactory factory;
        auto riveFilePtr = rive::File::import(rive::Span<const uint8_t>(bytes.data(), bytes.size()), &factory);
        if (!riveFilePtr) {
            std::cerr << "Failed to import Rive file" << std::endl;
            return -1;
        }
        
        // Get the artboard
        auto artboard = riveFilePtr->artboardDefault();
        if (!artboard) {
            std::cerr << "No artboard found in Rive file" << std::endl;
            return -1;
        }
        
        std::cout << "Artboard loaded: " << artboard->name() << std::endl;
        std::cout << "Dimensions: " << artboard->width() << " x " << artboard->height() << std::endl;
        std::cout << "Animation count: " << artboard->animationCount() << std::endl;
        
        // Get the first animation
        std::unique_ptr<rive::LinearAnimationInstance> animation;
        if (artboard->animationCount() > 0) {
            animation = artboard->animationAt(0);
            animation->time(0);
            animation->apply();
            std::cout << "Animation loaded: " << artboard->animation(0)->name() << std::endl;
            std::cout << "Duration: " << animation->durationSeconds() << " seconds" << std::endl;
        }
        
        // Performance test without renderer
        std::cout << "\nRunning 5-second CPU performance test..." << std::endl;
        std::cout << "This tests pure Rive animation processing speed" << std::endl;
        
        int frameCount = 0;
        double totalTime = 0.0;
        double minFrameTime = std::numeric_limits<double>::max();
        double maxFrameTime = 0.0;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        auto testDuration = std::chrono::seconds(5);
        
        while ((std::chrono::high_resolution_clock::now() - startTime) < testDuration) {
            auto frameStart = std::chrono::high_resolution_clock::now();
            
            // Update animation
            if (animation) {
                animation->advance(1.0 / 60.0); // Advance by 1/60th of a second
                animation->apply();
            }
            
            // Process artboard (CPU work only, no rendering)
            // This simulates the CPU processing that would happen before GPU/OpenVG rendering
            artboard->advance(1.0 / 60.0);
            
            auto frameEnd = std::chrono::high_resolution_clock::now();
            double frameTime = std::chrono::duration<double>(frameEnd - frameStart).count();
            
            totalTime += frameTime;
            frameCount++;
            minFrameTime = std::min(minFrameTime, frameTime);
            maxFrameTime = std::max(maxFrameTime, frameTime);
            
            // Print progress every 60 frames (once per second at 60fps)
            if (frameCount % 60 == 0) {
                double currentFPS = frameCount / totalTime;
                std::cout << "Frame " << frameCount << " | FPS: " << (int)currentFPS 
                         << " | Avg Frame Time: " << (totalTime / frameCount) * 1000 << "ms" << std::endl;
            }
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        double actualDuration = std::chrono::duration<double>(endTime - startTime).count();
        
        std::cout << "\n=== CPU PERFORMANCE RESULTS ===" << std::endl;
        std::cout << "Test Duration: " << actualDuration << " seconds" << std::endl;
        std::cout << "Total Frames: " << frameCount << std::endl;
        std::cout << "Average FPS: " << frameCount / actualDuration << std::endl;
        std::cout << "Min Frame Time: " << minFrameTime * 1000 << " ms" << std::endl;
        std::cout << "Max Frame Time: " << maxFrameTime * 1000 << " ms" << std::endl;
        std::cout << "Average Frame Time: " << (totalTime / frameCount) * 1000 << " ms" << std::endl;
        std::cout << "===============================" << std::endl;
        
        std::cout << "\nThis shows pure CPU animation processing speed." << std::endl;
        std::cout << "GPU/OpenVG rendering would add additional time on top of these numbers." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}