
#include "ncrapi/system/sysBase.hpp"
#include "ncrapi/system/logger.hpp"

namespace ncrapi
{
SysBase::SysBase(const json &pragam)
{
    if (!readSDcard(pragam))
    {
        logger->error({"从userConfig读取配置"});
        jsonVal = pragam;
        if (!saveData())
            logger->error({"config.json无法保存, 请检查SD卡"});
    }
    //系统信息录入
    robotInfo = jsonVal["系统信息"]["机器人类型"].get<std::string>() + " " + jsonVal["系统信息"]["队伍编号"].get<std::string>() + " " + jsonVal["系统信息"]["用户"].get<std::string>();
    for (size_t i = 1; i <= 21; i++)
        _i2cPort.push_back(i); //1-21I2C端口号
    for (size_t i = 1; i <= 8; i++)
        _adiPort.push_back(i); //1-8adi端口号
    logger->info({"系统基类构造成功"});
}

/*
 * 从SD中读取文件 存入数据容器
 * @tparam DATA 数据容器类型
 * @param data 数据容器
 * @param filePath 路径
 * @param name 名字
 */
bool SysBase::readSDcard(json pragam)
{
    FILE *file = fopen("/usd/config.json", "r");
    if (file == nullptr)
    {
        logger->error({"json 文件打开错误"});
        return false;
    }
    char buf[1024];
    std::string line;
    while (fgets(buf, 1024, file) != nullptr) //读取一行
        line += buf;
    jsonVal = json::parse(line);
    if (jsonVal["系统信息"]["用户"].get<std::string>() != pragam["系统信息"]["用户"].get<std::string>())
    {
        logger->error({"用户输入错误！请修改robotSet.hpp"});
        logger->error({"SD卡上用户名为:", jsonVal["系统信息"]["用户"].get<std::string>()});
        logger->error({"robotSet用户名为:", pragam["系统信息"]["用户"].get<std::string>()});
        while (1)
            ;
    }
    std::stringstream oss;     //主要为了去掉小数点后多余的0 默认3位
    oss.setf(std::ios::fixed); //用定点格式显示浮点数,不会用科学计数法表示
    oss.precision(1);          //由于用了定点格式，设置变为了保留1位小数
    oss << pragam["json版本号"];
    if (jsonVal["json版本号"] < pragam["json版本号"])
    {
        upDateJson(jsonVal, pragam);
        oss >> jsonVal["json版本号"];
        logger->debug({"json版本升级为:", oss.str().c_str()});
        saveData();
    }
    else
        logger->info({"json当前为最新版本:", oss.str().c_str()});
    fclose(file);
    return true;
}
void SysBase::upDateJson(json &source, const json &target)
{
    if (source.type() == target.type())
        switch (source.type())
        {
            case json::value_t::object:
            {
                //第一遍：遍历机器人文件的的元素
                for (auto it = source.begin(); it != source.end(); ++it)
                {
                    if (target.find(it.key()) != target.end())
                    {
                        //递归调用以比较对象的对象值
                        upDateJson(it.value(), target[it.key()]);
                    }
                    else
                    {
                        //找到一个不在o中的键 ->删除它
                        source.erase(it);
                        logger->debug({"删除: ", it.key()});
                    }
                }
                //第二遍：遍历用户数据里的元素
                for (auto it = target.begin(); it != target.end(); ++it)
                {
                    if (source.find(it.key()) == source.end()) //如果机器人文件没找到和目标文件里同名的元素
                    {
                        //添加他
                        source[it.key()] = it.value();
                        logger->debug({"添加: ", it.key()});
                    }
                }
                break;
            }
            default:
                break;
        }
    return;
}
/**
 * 以vector 容器为基础修改保存文件
 * @tparam T 数据的类型
 * @param data 数据的名称
 * @param filePath  数据的路径
 * @return true 保存成功
 * @return false 保存失败
 */

bool SysBase::saveData()
{
    FILE *file = fopen("/usd/config.json", "w");
    if (file == nullptr)
    {
        logger->error({"json 保存失败"});
        return false;
    }
    fprintf(file, "%s", jsonVal.dump(4).c_str()); //保存
    logger->info({"json 保存成功"});
    fclose(file);
    return true;
}
void SysBase::i2cCheck(const int port, const std::string name)
{
    for (auto it = _i2cPort.begin(); it != _i2cPort.end(); it++)
    {
        if (*it == port)
        {
            _i2cPort.erase(it);
            break;
        }
        if (it == _i2cPort.end())
            logger->error({name, "端口:", std::to_string(port), "冲突!,请检查配置"});
    }
}
void SysBase::adiCheck(const int port, const std::string name)
{
    for (auto it = _adiPort.begin(); it != _adiPort.end(); it++)
    {
        if (*it == port)
        {
            _adiPort.erase(it);
            break;
        }
        if (it == _adiPort.end())
            logger->error({name, "端口:", std::to_string(port), "冲突!,请检查配置"});
    }
}
void SysBase::adiCheck(const std::pair<int, int> port, const std::string name)
{
    int count = 0;
    for (auto it = _adiPort.begin(); it != _adiPort.end(); it++)
    {
        if (*it == port.first || *it == port.second)
        {
            _adiPort.erase(it);
            count++;
            if (count == 2)
                break;
        }
        if (it == _adiPort.end())
            logger->error({name, "端口:", std::to_string(port.first), " ", std::to_string(port.second), "冲突!,请检查配置"});
    }
}
/**
    *增加部件名字 
    * @param str 部件的名字
    */
void SysBase::addObj(Obj *generic)
{
    obj.push_back(generic);
}
/**
     *获取当前机器人部件总数 
     * @return size_t 部件总数
     */
size_t SysBase::getObjNums()
{
    return obj.size();
}
/**
     *停止所有部件运作
     */
void SysBase::stopAllObj()
{
    for (auto &it : obj)
        it->stop();
}
void SysBase::print(const json &pragma, std::string_view ignore)
{
    for (auto &it : pragma.items())
        if (it.key() != ignore)
            for (auto &it1 : it.value().items())
            {
                std::cout << it1.key() << " ";
                for (auto &it2 : it1.value().items())
                    std::cout << it2.key() << ":" << it2.value() << " ";
                std::cout << std::endl;
            }
}
//递归打印
void SysBase::recursionPrint(const json &pragma, std::string_view ignore)
{
    for (auto &[key, val] : pragma.items())
    {
        if (key != ignore)
        {
            if (val.is_structured())
            {
                std::cout << "\n"
                          << key << ": "; //这里可以右移
                recursionPrint(val, ignore);
            }
            else
            {
                std::cout << key << ":" << val << " ";
            }
        }
    }
    return;
}

} // namespace ncrapi
