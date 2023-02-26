#include <arcirk.hpp>
#include "lib/include/task.hpp"
#include <boost/thread.hpp>
#include <memory>
#include <functional>
#include <boost/filesystem.hpp>

//std::function<void(boost::uuids::uuid)> run_task;
//std::function<void(boost::uuids::uuid)> stop_task;
//std::function<void()> stop_tasks_io;

void task_tik(const arcirk::services::task_options& details){
    std::string threadId = boost::lexical_cast<std::string>(boost::this_thread::get_id());
    std::cout << "task_tik: " << details.uuid << " " << details.name << " " << threadId << std::endl;
    //boost::this_thread::sleep_for(boost::chrono::seconds{20});
}

nlohmann::json read_options(){
    using namespace boost::filesystem;
    path conf = "task_options.json";
    try {
        if(exists(conf)){
            std::ifstream file(conf.string(), std::ios_base::in);
            std::string str{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
            if(!str.empty()){
                return nlohmann::json::parse(str);
            }
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return {};
}

void write_conf(std::vector<arcirk::services::task_options> vec){

    using namespace boost::filesystem;
    nlohmann::json tasks_conf{};
    for (auto itr : vec) {
        tasks_conf += pre::json::to_json(itr);
    }
    try {
        std::string result = tasks_conf.dump();
        std::ofstream out;
        path conf_file = "task_options.json";
        out.open(arcirk::local_8bit(conf_file.string()), std::ios_base::out);
        if (out.is_open()) {
            out << result;
            out.close();
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {

    setlocale(LC_ALL, "Russian");

//    std::size_t threads = 4, capacity = 8;
//    thread_pool workers(threads, capacity);
    auto task_manager = std::make_shared<arcirk::services::task_scheduler>();
//    run_task = task_manager->get_run_task_ptr();
//    stop_task = task_manager->get_stop_task_ptr();
//    stop_tasks_io = task_manager->get_stop_io_ptr();

    auto options = read_options();
    std::vector<arcirk::services::task_options> vec;
    if(options.empty()){
        arcirk::services::task_options opt{};
        opt.end_task = 0;
        opt.start_task = 0;
        opt.interval = 10;
        opt.name = "task 10 sec";
        opt.uuid = boost::to_string(arcirk::uuids::random_uuid());
        opt.allowed = true;
        arcirk::services::task_options opt1{};
        opt1.end_task = 0;
        opt1.start_task = 0;
        opt1.interval = 30;
        opt1.name = "task 30 sec";
        opt1.uuid = boost::to_string(arcirk::uuids::random_uuid());
        opt1.allowed = true;
        vec.push_back(opt);
        vec.push_back(opt1);
        write_conf(vec);
    }else{
        for (auto itr = options.begin(); itr != options.end(); ++itr) {
            auto opt = pre::json::from_json<arcirk::services::task_options>(*itr);
            vec.push_back(opt);
        }
    }

    for (const auto& itr : vec) {
//        if(itr.allowed)
//            task_manager->add_task(itr.name, arcirk::uuids::string_to_uuid(itr.uuid), std::bind(&task_tik, std::placeholders::_1), itr.interval);
        if(itr.allowed)
            task_manager->add_task(itr, std::bind(&task_tik, std::placeholders::_1));
    }
//    task_manager->add_task("task 10 sec", arcirk::uuids::random_uuid(), std::bind(&task_tik), 10);
//    task_manager->add_task("task 30 sec", arcirk::uuids::random_uuid(), std::bind(&task_tik), 30);

    auto tr = boost::thread([&task_manager](){
        task_manager->run();
    });

    tr.detach();

    std::string line;
    while (getline(std::cin, line)) {
        if (line.empty()) {
            continue;
        }
        else if (line == "stop")
        {
            task_manager->stop();
        }else if(line == "exit")
            break;
        else if (line == "send")
        {
            //m_client->send_message("test message");
        }else if (line == "start") {
            task_manager->clear();
            task_manager = std::make_shared<arcirk::services::task_scheduler>();
            for (const auto& itr : vec) {
                if(itr.allowed)
                    task_manager->add_task(itr, std::bind(&task_tik, std::placeholders::_1));
            }
            task_manager->run();
        }
        else if (line == "add_task") {
            arcirk::services::task_options opt{};
            opt.end_task = 0;
            opt.start_task = 0;
            opt.interval = 25;
            opt.name = "task 25 sec";
            opt.uuid = boost::to_string(arcirk::uuids::random_uuid());
            opt.allowed = true;
            vec.push_back(opt);
            task_manager->add_task(opt, std::bind(&task_tik, std::placeholders::_1));
        }
        else {
            if(line.empty())
                continue;
            arcirk::T_vec vec = arcirk::split(line , " ");
            if(vec.size() == 2){
                if(vec[0] == "stop_task"){
                    task_manager->stop_task(arcirk::uuids::string_to_uuid(vec[1]));
                }else if(vec[0] == "start_task"){
                    task_manager->start_task(arcirk::uuids::string_to_uuid(vec[1]));
                }
            }

        }
    }
    return EXIT_SUCCESS;

}