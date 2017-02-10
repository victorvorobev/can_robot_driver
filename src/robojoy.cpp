#include <ros/ros.h>
#include <sensor_msgs/Joy.h>
//#include <can_robot_driver/CanDriver.h>
#include <geometry_msgs/Twist.h>

class RoboJoy	//класс, описывающий взаимодействие джойстика с роботом
{
public:
	RoboJoy();	//конструктор

private:
	void JoyCallback(const sensor_msgs::Joy::ConstPtr& joy);	//реакция на нажатие кнопок
	ros::NodeHandle n;	//создание узла ros
	
//	int16_t rightEngine;	//переменные для расчета скорости правого
//	int16_t leftEngine;	//и левого двигателя
	int16_t linearSpeed;	//переменная для записи линейной скорости
	int16_t angularSpeed;	//переменная для записи угловой скорости
	int16_t throttleValue;	//переменная для записи максимальной скорости
    int16_t prongSpeed;     //переменная для записи скорости рогов
	
	double cmdLinear, cmdAngular;	//переменные для формирования сообщения в топик cmd_vel
	
    int linear, angular, throttle, prong;	//какая ось джойстика за что отвечает
	double l_scale = 100;	//пропорциональные коээффициенты для осей
	double a_scale = 100;	//срезали угловую скорость, чтобы не очень резко поворачивал
    double p_scale = 100;
	double t_scale = -50;	//минус - поскольку ось на джойстике инвертирована
//	ros::Publisher commandPub;
	ros::Publisher cmdVelPub;
    ros::Publisher prongVelPub;
	ros::Subscriber joySub;
};

RoboJoy::RoboJoy():
  linear(1),
  angular(0),
  throttle(2),
  prong(5)
{
	n.param("axis_linear", linear, linear);
	n.param("axis_angular", angular, angular);
	n.param("scale_angular", a_scale, a_scale);
	n.param("scale_linear", l_scale, l_scale);
	n.param("axis_throttle", throttle, throttle);
	n.param("scale_throttle", t_scale, t_scale);
    n.param("axis_prong", prong, prong);
    n.param("scale_prong", p_scale, p_scale);

//	commandPub = n.advertise<can_robot_driver::CanDriver>("command",1);
	cmdVelPub = n.advertise<geometry_msgs::Twist>("cmd_vel", 10);
	joySub = n.subscribe<sensor_msgs::Joy>("joy", 10, &RoboJoy::JoyCallback, this);
    prongVelPub = n.advertise<geometry_msgs::Twist>("prong_vel", 10);
}

void RoboJoy::JoyCallback(const sensor_msgs::Joy::ConstPtr& joy)
{
//	can_robot_driver::CanDriver msg;
	geometry_msgs::Twist cmdVelMsg;
    geometry_msgs::Twist prongMsg;

	linearSpeed = l_scale*joy->axes[linear];	//получаем значение с джойстика
	angularSpeed = a_scale*joy->axes[angular];	//сразу пропорционально
	throttleValue = t_scale*joy->axes[throttle] + 50;	//сдвигаем так, чтобы полученное значение было от 0 до 100
    prongSpeed = p_scale*joy->axes[prong];
    if(prongSpeed > 100) prongSpeed = 100;
    if(prongSpeed < -100) prongSpeed = -100;
	
	cmdLinear = linearSpeed*throttleValue/100;
	cmdAngular = angularSpeed*throttleValue/100;

        if(cmdLinear > 100) cmdLinear = 100;
        if(cmdLinear < -100) cmdLinear = -100;
        if(cmdAngular > 100) cmdAngular = 100;
        if(cmdAngular < -100) cmdAngular = -100;

	cmdVelMsg.linear.x = cmdLinear;
	cmdVelMsg.angular.z = cmdAngular;
    prongMsg.linear.x = prongSpeed;
//	msg.right = (linearSpeed + angularSpeed)*throttleValue/100;
//	msg.left = (linearSpeed - angularSpeed)*throttleValue/100;

//	if(msg.right > 100) msg.right = 100;
//	if(msg.right < -100) msg.right = -100;
//	if(msg.left > 100) msg.left = 100;
//	if(msg.left < -100) msg.left = -100;

//	commandPub.publish(msg);
	cmdVelPub.publish(cmdVelMsg);
    prongVelPub.publish(prongMsg);
}

int main(int argc, char** argv)
{
	ros::init(argc, argv, "robo_joy");
	RoboJoy a;
	ros::spin();
}
