#include <math.h>
#include "blmc_robots/teststand.hpp"

namespace blmc_robots{

Teststand::Teststand()
{
  /**
    * Motor data
    */
  motor_positions_.setZero();
  motor_velocities_.setZero();
  motor_currents_.setZero();
  motor_torques_.setZero();
  motor_inertias_.setZero();
  motor_encoder_indexes_.setZero();
  motor_target_currents_.setZero();
  motor_target_torques_.setZero();
  motor_torque_constants_.setZero();
  target_motor_current_tmp_.setZero();

  for(unsigned i=0 ; i<motor_enabled_.size(); ++i)
  {
    motor_enabled_[i] = false;
    motor_ready_[i] = false;
  }

  for(unsigned i=0 ; i<motor_board_enabled_.size(); ++i)
  {
    motor_board_enabled_[0] = false;
    motor_board_errors_[0] = 0;
  }
  
  /**
    * Joint data
    */
  joint_positions_.setZero();
  joint_velocities_.setZero();
  joint_torques_.setZero();
  joint_target_torques_.setZero();
  joint_gear_ratios_.setZero();
  joint_zero_positions_.setZero();

  /**
    * Additional data
    */
  contact_sensors_states_.setZero();
  slider_positions_.setZero();
  motor_max_current_.setZero();

  /**
    * Setup some known data
    */

  // Max current in Amp
  motor_max_current_.fill(12);
  motor_torque_constants_.fill(0.025);
  motor_inertias_.fill(0.045);
  joint_gear_ratios_.fill(9.0);
  joint_max_torque_ = motor_max_current_.array() *
                      motor_torque_constants_.array() *
                      joint_gear_ratios_.array();
}


void Teststand::initialize()
{
  // @todo Create a modular way of adding cards
  // initialize the communication with the can cards
  can_buses_[0] = std::make_shared<blmc_drivers::CanBus>("can0");
  can_motor_boards_[0] = std::make_shared<blmc_drivers::CanBusMotorBoard>(can_buses_[0]);

  can_buses_[1] = std::make_shared<blmc_drivers::CanBus>("can1");
  can_motor_boards_[1] = std::make_shared<blmc_drivers::CanBusMotorBoard>(can_buses_[1]);

  sliders_[0] =
      std::make_shared<blmc_drivers::AnalogSensor>(can_motor_boards_[1], 0);
  sliders_[1] =
      std::make_shared<blmc_drivers::AnalogSensor>(can_motor_boards_[1], 1);
      
  contact_sensors_[0] =
      std::make_shared<blmc_drivers::AnalogSensor>(can_motor_boards_[0], 0);
  height_sensors_[0] =
      std::make_shared<blmc_drivers::AnalogSensor>(can_motor_boards_[0], 1);

  // can 0
  // MOTOR_HFE
  motors_[0] = std::make_shared<blmc_drivers::SafeMotor> (
                 can_motor_boards_[0], 1, motor_max_current_[0]);
  // MOTOR_KFE
  motors_[1] = std::make_shared<blmc_drivers::SafeMotor> (
                 can_motor_boards_[0], 0, motor_max_current_[1]);

  // Call the method to sync the max current with the max torques.
  set_max_current(motor_max_current_);

  // ATI sensor initialization.
  ati_sensor_.initialize();

  // Wait to make sure there is a first package when acquire_sensors() later.
  real_time_tools::Timer::sleep_sec(0.5);

  // Calibrate the zeros of the ati sensor given the current measurement.
  ati_sensor_.setBias();

  // wait until all board are ready and connected
  
  // can 2
  can_motor_boards_[0]->wait_until_ready();
  // can 3
  can_motor_boards_[1]->wait_until_ready();
}

bool Teststand::acquire_sensors()
{
  try{
    /**
      * Motor data
      */
    for (unsigned i=0 ; i<motors_.size() ; ++i)
    {
      // acquire the motors positions
      motor_positions_(i) =
          motors_[i]->get_measurement(mi::position)->newest_element()
          - joint_zero_positions_(i) * joint_gear_ratios_(i) ;
      // acquire the motors velocities
      motor_velocities_(i) =
          motors_[i]->get_measurement(mi::velocity)->newest_element();
      // acquire the motors current
      motor_currents_(i) =
          motors_[i]->get_measurement(mi::current)->newest_element();
      // acquire the last sent current sent
      motor_target_currents_(i) =
          motors_[i]->get_sent_current_target()->newest_element();
      // acquire the encoder indexes
      motor_encoder_indexes_(i) =
          motors_[i]->get_measurement(mi::encoder_index)->length() > 0 ?
            motors_[i]->get_measurement(mi::encoder_index)->newest_element():
            std::nan("");
    }
    // acquire the actual motor torques
    motor_torques_ = motor_currents_.array() * motor_torque_constants_.array();
    // acquire the last sent motor torques
    motor_target_torques_ = motor_target_currents_.array() *
                            motor_torque_constants_.array();

    /**
      * Joint data
      */
    // acquire the joint position
    joint_positions_ = motor_positions_.array() / joint_gear_ratios_.array();
    // acquire the joint velocities
    joint_velocities_ = motor_velocities_.array() / joint_gear_ratios_.array();
    // acquire the joint torques
    joint_torques_ = motor_torques_.array() * joint_gear_ratios_.array();
    // acquire the tqrget joint torques
    joint_target_torques_ = motor_target_torques_.array() *
                            joint_gear_ratios_.array();
    joint_encoder_index_ = motor_encoder_indexes_.array() /
                            joint_gear_ratios_.array();


    /**
      * Additional data
      */
    // acquire the slider positions

    for (unsigned i=0 ; i < slider_positions_.size() ; ++i)
    {
      // acquire the slider
      slider_positions_(i) = sliders_[i]->get_measurement()->newest_element();
    }


    for (unsigned i=0 ; i < contact_sensors_states_.size() ; ++i)
    {
      // acquire the current contact states
      contact_sensors_states_(i) =
          contact_sensors_[i]->get_measurement()->newest_element();
      // acquire the height sensor.
      // Transforms the measurement into a rough height measurement of the hip
      // mounting point above the table.
      height_sensors_states_(i) =
          1.075 - height_sensors_[i]->get_measurement()->newest_element();
    }

    /**
     * Ati sensor readings.
     */
    ati_sensor_.getFT(&ati_force_(0), &ati_torque_(0));

    // Rotate the force and torque values, such that pressing on the force
    // sensor creates a positive force.
    ati_force_(0) *= -1;
    ati_force_(2) *= -1;
    ati_torque_(0) *= -1;
    ati_torque_(2) *= -1;
  }catch(std::exception ex)
  {
    rt_printf("HARDWARE: Something went wrong during the sensor.\n");
    rt_printf("error is: %s\n", ex.what());
    return false;
  }
  return true;
}

bool Teststand::send_target_motor_current(
    const Eigen::Ref<Vector2d> target_motor_current)
{
  try{
    // set up the target current
    for(unsigned i=0 ; i<motors_.size() ; ++i)
    {
      motors_[i]->set_current_target(target_motor_current(i));
    }

    // actually send the torques to the robot
    for(unsigned i=0 ; i<motors_.size() ; ++i)
    {
      motors_[i]->send_if_input_changed();
    }
  }catch(std::exception ex)
  {
    rt_printf("HARDWARE: Something went wrong when sending the control.\n");
    rt_printf("error is: %s\n", ex.what());
    return false;
  }
  return true;
}

bool Teststand::send_target_joint_torque(
    const Eigen::Ref<Vector2d> target_joint_torque)
{
  target_motor_current_tmp_ = target_joint_torque.array() /
                              joint_gear_ratios_.array() /
                              motor_torque_constants_.array();
  return send_target_motor_current(target_motor_current_tmp_);
}

} // namespace blmc_robots
