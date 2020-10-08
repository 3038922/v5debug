#pragma once
#include "ncrapi/chassis/chassisAutoAiming.hpp"
#include "ncrapi/generic/roulette.hpp"
#include "ncrapi/generic/shooter.hpp"
#include "ncrapi/system/sysUser.hpp"
#include "ncrapi/system/visionData.hpp"
#include "userConfig/robotSet.hpp"

#define NCR_VERSION_STRING "10.1.1"

//通用部件
extern std::shared_ptr<pros::Controller> joy1;
extern std::shared_ptr<pros::Controller> joy2;
extern std::shared_ptr<ncrapi::ChassisOdom> chassis; //底盘

#if defined(no8982A) || defined(no8982C) || defined(no7671D)                                           // 坦克机|| defined(xxx)
extern std::shared_ptr<ncrapi::Roulette> roulette;                                                     //吸吐
extern std::shared_ptr<ncrapi::SeparateShooter> shooter;                                               //分离式发射器
#elif defined(no8982B) || defined(no7671C)                                                             //大炮机 || defined(xxx)
extern std::shared_ptr<ncrapi::Roulette> roulette; //吸吐
extern std::shared_ptr<ncrapi::Shooter> shooter;   //联动式发射器
#elif defined(no7671A) || defined(no7671B) || defined(no8982D) || defined(no7995A) || defined(no7925A) //赛博朋克2077 || defined(xxx)
extern std::shared_ptr<ncrapi::Roulette> roulette;           //吸吐
extern std::shared_ptr<ncrapi::DistributionShooter> shooter; //分球式发射器
#elif                                                                                                  //缝合怪 || defined(xxx)
#endif

extern pros::Task *autoTask;

static void taskAuto(void *para)
{
    uint32_t now = pros::millis();
    while (true)
    {
        roulette->holding();
        shooter->holding();
        pros::Task::delay_until(&now, 10);
    }
}
/**
  * 用于机器人姿态计算的多线程启动函数
 * @param para NULL
 */
static void taskOdom(void *para)
{
    chassis->odomLoop();
}
void skillAuto(const json &jsonVal, ncrapi::Timer &autoTimer);
void userauto(const json &jsonVal, ncrapi::Timer &autoTimer);
void customTest();