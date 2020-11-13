#pragma once
#include "ncrapi/device/motor.hpp"
#include "ncrapi/system/circular_buffer.hpp"
#include "ncrapi/system/json.hpp"
#include "ncrapi/system/object.hpp"
#include "ncrapi/userDisplay/userDisplay.hpp"
#include "ncrapi/util/timer.hpp"
#include "pros/misc.hpp"

namespace ncrapi {
/**
 * 机器人功能基础类 开环控制
 * @param a    马达对象
 * @param hold 悬停值
 */
class Generic : public Obj
{

  public:
    Generic(const std::string &name, const json &pragma);
    /**
     * @brief 获取当前部件悬停值
     * 
     * @return int 悬停值
     */
    virtual int getHoldingVal();
    /**
     * 普通的占空比控制  开环
     * @param pwm 占空比+-127
     */
    virtual void set(const int pwm);
    /**
     * @brief 普通的占空比控制  开环
     * 
     * @param nums 要控制的马达号 从0开始 
     * @param pwm 占空比+-127
     */
    virtual void set(const uint32_t nums, const int pwm);
    /**
     * 普通的电压控制 开环     
     * @param vol +-120
     */
    virtual void moveVoltage(const double vol);
    /**
     * @brief 部件马达停止运转
     * 
     */
    virtual void stop() override;
    /**
    *设置要移动到的电机的目标绝对位置。
    *
    *此运动相对于初始化时电机的位置或最近使用pros :: Motor :: set_zero_position（）复位的位置。
    *\注意此功能只是设置电机的目标，它不会阻止程序执行直到运动结束。
   * @param position 要移动的位置 注意电机一圈360
   * @param velocity 速度 红齿轮上限100 绿齿轮200 蓝齿轮600 
   */
    virtual void moveAbsolute(const double position, const std::int32_t velocity);

    /**
    *设置要移动到的电机的相对目标位置。
    *
    *此运动与pros :: Motor :: motor_get_position（）中给出的电机当前位置有关。提供10.0作为位置参数将导致电机顺时针移动10个单位，无论当前位置如何。
    *
    *\注意此功能只是设置电机的目标，它不会阻止
    *程序执行直到运动结束。
   * @param position 要移动的位置 注意电机一圈360
   * @param velocity 速度 红齿轮上限100 绿齿轮200 蓝齿轮600 
     */
    virtual void moveRelative(const double position, const std::int32_t velocity);
    /**
    设定电机的速度。
    *
    *
    *当达到错误状态时，此函数使用以下errno值：
    *EACCES  -另一个资源目前正在尝试访问该端口。
    *
    *\ param velocity 新电机速度为 - +  - 100，+ -200或+ -600，具体取决于电机的齿轮组
    *
    *\如果操作成功则返回1或如果操作则返回PROS_ERR
    *失败，设置错误。
     */
    virtual void moveVelocity(const std::int32_t velocity);
    /**
     * @brief 获取当前部件状态
     * 
     * @return int 各个部件定义的_state不同
     */
    virtual int getState();
    /**
     * @brief 悬停逻辑状态都写这里
     * 
     */
    virtual void holding();

    /**
     *设置模式   
     * @param flag 1:系统正转模式 0:马达悬停 -1:系统反转模式
     */
    virtual void setMode(const int flag) override;
    /**
     * 获取当前模式状态      
     * @return 1是系统正转状态 0悬停状态 -1 是系统反转状态
     */
    virtual int getMode();

    virtual void joyControl(std::shared_ptr<pros::Controller> joy, pros::controller_digital_e_t up, pros::controller_digital_e_t down);
    /**
    * 重置马达编码器
    */
    virtual void resetEnc();
    /**
 * @brief 设置编码器当前值
 * 
 * @param pos 要设置的值
 */
    virtual void setEnc(const double pos);
    /**
     * @brief 获取开环控制占空比(pwm)
     * 
     */
    virtual int getOpenLoopVal();
    /**
     * @brief 设置编码器移动的位置 
     * 
     * @param target 编码器移动的位置
     */
    virtual void setTarget(const int target);
    /**
     * 获取编码器平均值
     * @return 返回编码器的平均值
     */
    virtual double getEnc();
    /**
     * @brief 获取单独每个马达编码器值
     * 
     * @param nums 马达编号 从0开始
     * @return double 返回编码器值
     */
    virtual double getEnc(const uint32_t nums);
    /**
     * 获取速度
     * @return 返回速度
     */
    virtual double getSpeed();
    /**
     * 获取电机瓦数
     * @return 返回电机瓦数
     */
    virtual double getPower();
    /**
     * 获取电机以mA为单位的电流平均值。
     * @return 返回电机以mA为单位的电流平均值。
     */
    virtual int32_t getCurrent();
    /**
     * @brief 获取电机以mA为单位的电流单个值。
     * 
     * @param nums 电机编号
     * @return 返回电机以mA为单位的电流单个值
     */
    virtual int32_t getCurrent(const uint32_t nums);
    /**
     * 以毫伏为单位获取输送到电机的电压。
     * @return如果操作失败，则以mV或PROS_ERR_F为单位返回电机电压，设置errno。
     */
    virtual int32_t getVoltage();
    /**
     * 获取电机效率
     * @return 以百分比形式返回电机效率
     */
    virtual double getEfficiency();
    /**
     * 获取电机扭矩
     * @return 返回电机扭矩
     */
    virtual double getTorque();
    /**
     * 获取电机平均温度
     * @return 返回电机平均温度
     */
    virtual double getTemperature();
    /**
     * 检测部件是否堵转
     * @return 返回false 没有堵转,返回true 堵转了
     */
    virtual bool isSafeMode() override;
    /**
     * 重置所有相关传感器
     */
    virtual void resetAllSensors() override;

    /**
     * 设置马达制动模式 使用会导致马达端口烧掉
     * @param mode 马达制动的模式 
     */
    virtual void setBrakeMode(pros::motor_brake_mode_e_t mode);
    /**
     * 显示传感器数据到屏幕 ostringstream ostr流
     */
    virtual void showSensor() override;
    virtual const std::string showName() const override;
    virtual void showDetailedInfo() override;
    virtual const std::vector<Motor> getMotorInfo() const override;

  protected:
    std::shared_ptr<pros::Controller> _thisJoy = nullptr;
    std::vector<Motor> _motorList;
    std::vector<circular_buffer<double, 3>> _encVal;
    const std::string _name;
    int _holdVal;
    float _holdingFlag = 0;
    int _openLoopVal = 0; //开环控制设置值
    size_t _safeModeFlags = 0;
    int _state = 0;       //-1降 0 悬停 1 升
    int _mode = 0;        //1 系统正传 0悬停 -1 系统反转 2使用原厂PID控制
    size_t _gearing;      //齿轮最大速度
    size_t _nums = 0;     //马达总数
    bool _isInit = false; //是否初始化
    int _target = 0;
    bool _isSafeMode = 0;

  private:
}; // namespace ncrapi
} // namespace ncrapi