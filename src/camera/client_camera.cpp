#include <carla/client/Client.h>
#include <carla/client/World.h>
#include <carla/client/BlueprintLibrary.h>
#include <carla/client/Actor.h>
#include <carla/client/ActorBlueprint.h>
#include <carla/client/Vehicle.h>
#include <carla/client/Sensor.h>
#include <carla/client/ActorList.h>

#include <carla/sensor/data/Image.h>

#include <carla/geom/Transform.h>
#include <carla/geom/Location.h>
#include <carla/geom/Rotation.h>

#include <opencv2/opencv.hpp>

#include <iostream>
#include <chrono>
#include <thread>
#include <memory>

void ProcessImage(boost::shared_ptr<carla::sensor::data::Image> image) {
  int width = image->GetWidth();
  int height = image->GetHeight();

  cv::Mat img(height, width, CV_8UC4, const_cast<void*>(static_cast<const void*>(image->data())));

  cv::Mat bgr_image;
  cv::cvtColor(img, bgr_image, cv::COLOR_BGRA2BGR);

  cv::imshow("Kamera", bgr_image);
  cv::waitKey(1); 
  std::cout << "Görüntü alindi " << image->GetWidth() << "x" << image->GetHeight() << std::endl;
}

int main() {
    try {
        carla::client::Client client("localhost", 2000);
        client.SetTimeout(std::chrono::seconds(10));
        auto world = client.GetWorld();

        auto blueprint_library = world.GetBlueprintLibrary();

        auto actors = world.GetActors();
        const int vehicle_id = 203; //manual_control.py ile elde edilen araç ID'si
        auto vehicle_actor = actors->Find(vehicle_id);
        auto vehicle = boost::dynamic_pointer_cast<carla::client::Vehicle>(vehicle_actor);
        if (!vehicle) {
            std::cerr << "Araç bulunamadi! ID: " << vehicle_id << std::endl;
            return 1;
        }

        auto camera_bp = (*blueprint_library->Find("sensor.camera.rgb"));
        camera_bp.SetAttribute("image_size_x", "800");
        camera_bp.SetAttribute("image_size_y", "600");
        camera_bp.SetAttribute("fov", "90"); 

        carla::geom::Transform cam_transform(
            carla::geom::Location(0.8f, 0.0f, 1.7f),
            carla::geom::Rotation(0.0f, 0.0f, 0.0f)
        );

        auto camera = boost::static_pointer_cast<carla::client::Sensor>(world.SpawnActor(camera_bp, cam_transform, vehicle.get()));

        camera->Listen([](auto data) {
            auto image = boost::static_pointer_cast<carla::sensor::data::Image>(data);
            ProcessImage(image);
        });

        std::cout << "Görüntü aliniyor..." << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Hata: " << e.what() << std::endl;
        return 1;
    }
}
