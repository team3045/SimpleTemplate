#include "WPILib.h"
#include "NetworkTables/NetworkTable.h"

#include <Timer.h>

// Nathan Amarandos - Team 3045
// With meddling from some others...

// we maybe should try to parameterize this using NetworkTables

#define autonomous_Forward_Speed -1.0	// how fast to move
#define autonomous_Forward_Time 0.5		// how long to move
#define autonomous_Settle_Time 0.25		// how long to settle down before firing

class File2014 : public SimpleRobot
{
	RobotDrive myRobot;
	Joystick stickA;	// "Adam's" joystick
	Joystick stickN;	// "Nathan's" joystick
	
	Victor shooter; 
	Victor forklift; 
	Victor catcher; 

	DigitalInput stopCocking;
	DigitalInput shoot;
	DigitalInput catcherTop;
	DigitalInput catcherBottom;
	DigitalInput pickerStop;
	
	Compressor compressor;
	DoubleSolenoid shooterSole;
	
	Timer autonTimer;
	
public:
	NetworkTable *table;
	
	File2014():
		
	
		myRobot(1, 2),	
		stickA(1),		
		stickN(2),
		
		shooter(6),
		forklift(10),
		catcher(9),
		
		stopCocking(10),
		shoot(3),
		catcherTop(2),
		catcherBottom(5),
		pickerStop(1),
		
		compressor(4,1),//switch, relay
		shooterSole(2,3)
	{
		myRobot.SetExpiration(0.1);
		table = NetworkTable::GetTable("3045RobotVision");
		table->PutBoolean("hotTarget", false);
		table->PutBoolean("coldTarget", true);
		table->PutNumber("throttle", 0.0);
		printf("putBoolean\n");
	}

// factor motor directions into a common function
// inverting all true for the bagged robot
	void SetMotorDirections()
	{
		myRobot.SetInvertedMotor(myRobot.kRearLeftMotor, true);
		myRobot.SetInvertedMotor(myRobot.kRearRightMotor, true);
		myRobot.SetInvertedMotor(myRobot.kFrontLeftMotor, true);
		myRobot.SetInvertedMotor(myRobot.kFrontRightMotor, true);
	}
	
	void Autonomous()
	{
//		we maybe need to figure out how to run this safely with "true", but for now...
		myRobot.SetSafetyEnabled(false);
		//GetWatchdog().SetEnabled(true);

		SetMotorDirections();

		shooterSole.Set(shooterSole.kForward);

		compressor.Start();
		
		printf("AutonomousStart - new\n");

#if 0
		double cumTime = 0.0;
		double startTime = 0.0;
		autonTimer.Start();

// drive for autonomous_Forward_Time
		startTime = autonTimer.Get();
		table->PutNumber("startTime", startTime);
		cumTime = autonTimer.Get();
		while ((cumTime-startTime) < autonomous_Forward_Time) {
			myRobot.Drive(autonomous_Forward_Speed, 0.0);
			cumTime = autonTimer.Get();
			table->PutNumber("cumTime", cumTime);
			printf("time: %g\n", cumTime);
		}
		printf("time: %g\n", cumTime);

// settle for autonomous_Settle_Time
		startTime = autonTimer.Get();
		table->PutNumber("startTime", startTime);
		cumTime = autonTimer.Get();
		while ((cumTime-startTime) < autonomous_Settle_Time) {
			myRobot.Drive(0.0, 0.0);
			cumTime = autonTimer.Get();
			table->PutNumber("cumTime", cumTime);
			printf("time: %g\n", cumTime);
		}
		printf("time: %g\n", cumTime);
#endif				

#if 1
		printf("start loop\n");
		double throttle = 0.5;
		table->PutNumber("throttle", throttle);
		bool firstTime = true;
		while (true) {
			if (firstTime) {
				printf("start inner loop\n");
				firstTime = false;
			}
			myRobot.Drive(0.0, 0.0);
			throttle = table->GetNumber("throttle");
			if (throttle != 0.0) {
				table->PutNumber("throttle", 0.0);

				double cumTime = 0.0;
				double startTime = 0.0;
				autonTimer.Start();

		// drive for autonomous_Forward_Time
				startTime = autonTimer.Get();
				table->PutNumber("startTime", startTime);
				cumTime = autonTimer.Get();
				printf("time start: %g\n", cumTime);
				while ((cumTime-startTime) < 1.00) {
					myRobot.Drive(throttle, 0.0);
					cumTime = autonTimer.Get();
					table->PutNumber("cumTime", cumTime);
									}
				printf("time end: %g\n", cumTime);
	// stop
				myRobot.Drive(0.0, 0.0);
			}
		}
#endif				
		
		table->GetBoolean("hotTarget");

#if 0		
		printf("drive auton one half\n");
		myRobot.Drive(-1,0);
		Wait(0.5);
		myRobot.Drive(0,0); 

		bool stop = true;
		if (shoot.Get())
			stop = false;
		
		while(stop){
		shooter.SetSpeed(-1);
		if (shoot.Get())
			stop = false;
		}
		
		shooter.SetSpeed(0);
		//Check if Goal is hot
		Wait(0.1);
		shooterSole.Set(shooterSole.kReverse);//Shoots the ball
		Wait(0.5);
		myRobot.Drive(-1,0);
		Wait(0.5);
		myRobot.Drive(0,0); 
		printf("drive auton two half\n");
#endif
	}
	
	void OperatorControl()
	{
		myRobot.SetSafetyEnabled(true);
		GetWatchdog().SetEnabled(true);
		compressor.Start();
		SetMotorDirections();
		shooterSole.Set(shooterSole.kForward);
	
		printf("OperatorControlStart - new\n");
		
		while (true)
		{
			GetWatchdog().Feed();
			myRobot.ArcadeDrive(stickA);
			//Forklift Code
			double forkliftSpeed = 0;
			if (stickA.GetRawButton(5)){ // 5 is left bumper; 6 is right bumper
				forkliftSpeed = .5;
			}
						
			if (stickA.GetRawButton(6)){
				forkliftSpeed = -.5;
			}
		
			if (!(stickA.GetRawButton(5)||stickA.GetRawButton(6))){
				forkliftSpeed = 0;
			}

			if (pickerStop.Get()){
				forkliftSpeed = 0;
			}
			if (pickerStop.Get()&&stickA.GetRawButton(6)){
				forkliftSpeed = -.5;
			}
			
			forklift.SetSpeed(forkliftSpeed);
	
						
			//Shooter Motor Code
			double shooterSpeed = 0;
			
			if (stickN.GetRawButton(4)) {	// 4 'A' button; 2 "Y" button
				shooterSpeed = 1;
			}
			if (stickN.GetRawButton(2)){
				shooterSpeed = -1;
			}
			if (!(stickN.GetRawButton(2)||stickN.GetRawButton(4))){
				shooterSpeed = 0;
			}

			if (shoot.Get()){
				shooterSpeed = 0;
			}
			
			if (stopCocking.Get()){
				shooterSpeed = 0;
			}
			
			if (stopCocking.Get()&&stickN.GetRawButton(2)){
				shooterSpeed = -1;
			}
						
			if (shoot.Get()&&stickN.GetRawButton(4)){
				shooterSpeed = 1;
			}
			
			shooter.SetSpeed(shooterSpeed);
			
			//Shooter Pneumatic Code
			if(stickN.GetRawButton(1)){	// 1 X 3 is B; 7 8 l and r triggers.
				shooterSole.Set(shooterSole.kForward);
			}
			if(stickN.GetRawButton(8)){
				shooterSole.Set(shooterSole.kReverse);
			}
			
			//Catcher Code
			double catcherSpeed = 0;
			if (stickN.GetRawButton(5)){
				catcherSpeed = 1;
			}
						
			if (stickN.GetRawButton(6)){
				catcherSpeed = -1;
			}
						
			if (!(stickN.GetRawButton(5)||stickN.GetRawButton(6))){
				catcherSpeed = 0;
			}
			
			if (catcherTop.Get()){
				catcherSpeed = 0;
			}
			
			if (catcherBottom.Get()){
				catcherSpeed = 0;
			}
			
			if (catcherTop.Get()&&stickN.GetRawButton(5)){
				catcherSpeed = 1;
			}
			
			if (catcherBottom.Get()&&stickN.GetRawButton(6)){
				catcherSpeed = -1;
			}
			
			catcher.SetSpeed(catcherSpeed);
			
			if (stickN.GetRawButton(7)){
				bool clicked = true;
				while(clicked){
					shooterSole.Set(shooterSole.kForward);
					myRobot.Drive(1.0, 1.2);//power  of drive train, radians
					Wait(0.05);
					myRobot.Drive(1.0, -1.2);
					Wait(0.05);
					myRobot.Drive(0.0, 0.0);
					shooterSole.Set(shooterSole.kReverse);
					clicked = false;
				}
			}
#if 0
			if (stickN.GetRawButton(8)) {
				bool clicked = true;
				while (clicked) {
					shooterSole.Set(shooterSole.kForward);
					myRobot.Drive(1.0, -1.2);//power  of drive train, radians
					Wait(0.05);
					myRobot.Drive(1.0, 1.2);
					Wait(0.05);
					myRobot.Drive(0.0, 0.0);
					shooterSole.Set(shooterSole.kReverse);
					clicked = false;
				}
			}
#endif
		}//Close Loop
	}//Close Operator Control Method


	void Test() {

		}
	};

START_ROBOT_CLASS(File2014);

