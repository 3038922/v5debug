#include "main.h"
#include "ncrapi/ncrapi.hpp"
#include "userconfig.hpp"
using namespace ncrapi;
//全局变量和类
std::unique_ptr<ncrapi::VisionData> visionData = nullptr; //视觉传感器数据类
//全局初始化构造函数
//部件类初始化
std::shared_ptr<pros::Controller> joy1 = nullptr;
std::shared_ptr<pros::Controller> joy2 = nullptr;

std::unique_ptr<pros::Task> loggerTask = nullptr;

std::shared_ptr<ncrapi::Generic> roulette = nullptr; //吸吐
std::shared_ptr<ncrapi::Generic> shooter = nullptr;  //发射器
pros::Task *autoTask = nullptr;
void initialize()
{
    Logger::initLogger();               //logger 单例实现
    UserDisplay::initUserDisplay();     //显示初始化 单例实现
    NcrSystem::initNcrSystem(userData); //系统数据初始化 单例实现

    userDisplay->init(); //设置当前背景颜色
    //遥控器初始化
    joy1 = std::make_shared<pros::Controller>(CONTROLLER_MASTER);  //主遥控器
    joy2 = std::make_shared<pros::Controller>(CONTROLLER_PARTNER); //副遥控器

    // 视觉传感器初始化
    //visionData = std::make_unique<VisionData>(I18N_VISION_FRONT, ncrSys->jsonVal[I18N_VISION_INFO]); //视觉初始化

    // 底盘初始化
    Chassis::initChassis(ncrSys->jsonVal[I18N_CHASSIS]);
    shooter = std::make_shared<ncrapi::Generic>("发射器", ncrSys->jsonVal["发射器"]);
    roulette = std::make_shared<ncrapi::Generic>("吸吐", ncrSys->jsonVal["吸吐"]);
    logger->info({"机器人初始化完毕"});
    userDisplay->terminal->style_p->body.main_color = LV_COLOR_BLACK; //控制台还原为黑色
    lv_obj_del(userDisplay->logoObj);
}

void disabled()
{
    userDisplay->delTasks();
    userDisplay->delObjs();
    userDisplay->createUserObj(OBJ_DISABLED, "obj_disabled", nullptr, "场控关闭状态");
    for (auto &it : ncrSys->obj)
        it->setMode(0); //将所有部件都重置为0模式
    if (autoTask != nullptr)
    {
        if (autoTask->get_state() != 4)
        {
            autoTask->remove();
            delete autoTask;
            logger->debug({"自动赛线程 自动赛外 删除成功!"});
        }
        autoTask = nullptr;
    }
}

void competition_initialize() { userDisplay->createCompe(); }

void autonomous()
{
    ncrapi::Timer autonomousTimer;
    autonomousTimer.placeMark();
    if (autoTask == nullptr)
        autoTask = new pros::Task((pros::task_fn_t)taskAuto, nullptr, TASK_PRIORITY_DEFAULT + 1, TASK_STACK_DEPTH_DEFAULT, "task_auto");
    if (pros::competition::get_status() == COMPETITION_CONNECTED)
    {
        userDisplay->delTasks();
        userDisplay->delObjs();
        userDisplay->createUserObj(OBJ_AUTONOMOUS, "自动赛", nullptr, "自动赛进行中");
    }
    logger->debug({"自动赛开始:", std::to_string(autonomousTimer.getDtFromMark().convert(ncrapi::millisecond))});
    chassis->setMode(ncrSys->jsonVal["底盘"]["参数"]["模式"]);
    if (!ncrSys->jsonVal["自动赛"]["自动赛&纯自动"])
        userauto(ncrSys->jsonVal["自动赛"], autonomousTimer);
    else
        skillAuto(ncrSys->jsonVal["自动赛"], autonomousTimer);
    ncrSys->stopAllObj();
    if (autoTask != nullptr) //如果shoot类不为空 则创建一个task
    {
        autoTask->remove();
        delete autoTask;
        autoTask = nullptr;
        logger->debug({"自动赛线程 自动赛内 删除成功!"});
    }
    logger->debug({"自动赛结束:", std::to_string(autonomousTimer.getDtFromMark().convert(ncrapi::millisecond))});
}

void opcontrol()
{

    if (autoTask != nullptr)
    {
        if (autoTask->get_state() != 4)
        {
            autoTask->remove();
            delete autoTask;
            logger->debug({"自动赛线程 手动赛处 删除成功!"});
        }
        autoTask = nullptr;
    }
    userDisplay->createOpObj();
    uint32_t lastTime = pros::millis();
    uint32_t nowTime = pros::millis();
    std::shared_ptr<pros::Controller> joy = joy1;
    joy1 = std::make_shared<pros::Controller>(CONTROLLER_MASTER); //主遥控器
    while (true)
    {
        // std::cout << "errorCode:" << strerror(errno) << std::endl; //打印错误码
        userDisplay->loopTime = pros::millis() - lastTime;
        lastTime = pros::millis();
        if (userDisplay->loopTime > userDisplay->maxLoopTime)
            userDisplay->maxLoopTime = userDisplay->loopTime;
        if (userDisplay->loopTime < userDisplay->minLoopTime)
            userDisplay->minLoopTime = userDisplay->loopTime;
        if (ncrSys->isOPcontrol)
        {
            if (joy1->get_digital_new_press(DIGITAL_RIGHT)) //切换单人双人
            {
                if (joy == joy1)
                    joy = joy2;
                else
                    joy = joy1;
                joy->rumble(". ");
            }

            chassis->arcade(joy1, ANALOG_LEFT_Y, ANALOG_RIGHT_X);
            roulette->joyControl(joy, DIGITAL_R1, DIGITAL_R2);
            shooter->joyControl(joy, DIGITAL_L1, DIGITAL_L2);
        }
        else
        {
            ncrSys->stopAllObj();
            if (joy1->get_digital_new_press(DIGITAL_A))
                ncrSys->testAction(&autonomous, &customTest, joy1);
        }
        pros::Task::delay_until(&nowTime, 10);
    }
}
