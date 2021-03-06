/**
 * \file test_bench_8_motors.hpp
 * \brief The hardware wrapper of the test bench with 8 blmc motors
 * \author Maximilien Naveau
 * \date 2018
 *
 * This file declares the TestBench8Motors class which defines the test
 * bench with 8 motors.
 */

#ifndef TEST_BENCH_8_MOTORS_HPP
#define TEST_BENCH_8_MOTORS_HPP

#include <blmc_robots/common_header.hpp>

namespace blmc_robots
{
/**
 * @brief The TestBench8Motors class implements the control of the test bench
 * containing 8 motors and 8 sliders using the blmc drivers.
 */
class TestBench8Motors
{
public:
    /**
     * @brief mi this typdef is used to get the measurements form the blmc api
     */
    typedef blmc_drivers::MotorInterface::MeasurementIndex mi;

    /**
     * @brief TestBench8Motors is the constructor of the class.
     */
    TestBench8Motors();

    /**
     * @brief initialize the robot by setting aligning the motors and calibrate
     * the sensors to 0
     */
    void initialize();

    /**
     * @brief send_target_torques sends the target currents to the motors
     */
    void send_target_current(const Vector8d target_currents);

    /**
     * @brief acquire_sensors acquire all available sensors, WARNING !!!!
     * this method has to be called prior to any getter to have up to date data.
     */
    void acquire_sensors();

    /**
     * @brief get_motor_positions
     * @return the current motors positions (rad).
     * WARNING !!!!
     * The method <acquire_sensors>"()" has to be called
     * prior to any getter to have up to date data.
     */
    const Vector8d get_motor_positions()
    {
        return motor_positions_;
    }

    /**
     * @brief get_motor_velocities
     * @return the current motors velocities (rad/s).
     * WARNING !!!!
     * The method <acquire_sensors>"()" has to be called
     * prior to any getter to have up to date data.
     */
    const Vector8d get_motor_velocities()
    {
        return motor_velocities_;
    }

    /**
     * @brief get_motor_currents
     * @return the current motors currents in (Ampere: A).
     * WARNING !!!!
     * The method <acquire_sensors>"()" has to be called
     * prior to any getter to have up to date data.
     */
    const Vector8d get_motor_currents()
    {
        return motor_currents_;
    }

    /**
     * @brief get_motor_encoder_indexes
     * @return TODO: ask Manuel what is this exactly.
     * WARNING !!!!
     * The method <acquire_sensors>"()" has to be called
     * prior to any getter to have up to date data.
     */
    const Vector8d get_motor_encoder_indexes()
    {
        return motor_encoder_indexes_;
    }

    /**
     * @brief get_slider_positions
     * @return the current sliders positions.
     * WARNING !!!!
     * The method <acquire_sensors>"()" has to be called
     * prior to any getter to have up to date data.
     */
    const Vector8d get_slider_positions()
    {
        return slider_positions_;
    }

    /**
     * @brief get_max_current
     * @return the maximum current to apply to the motors
     */
    double get_max_current()
    {
        return max_current_;
    }

    /**
     * @brief get_max_current
     * @return the maximum current to apply to the motors
     */
    double get_max_range()
    {
        return max_range_;
    }

private:
    Vector8d motor_positions_;
    Vector8d motor_velocities_;
    Vector8d motor_currents_;
    Vector8d motor_encoder_indexes_;
    Vector8d slider_positions_;
    Vector8d target_currents;

    std::shared_ptr<blmc_drivers::CanBus> can_bus0_;
    std::shared_ptr<blmc_drivers::CanBus> can_bus1_;
    std::shared_ptr<blmc_drivers::CanBus> can_bus2_;
    std::shared_ptr<blmc_drivers::CanBus> can_bus3_;

    std::shared_ptr<blmc_drivers::CanBusMotorBoard> board0_;
    std::shared_ptr<blmc_drivers::CanBusMotorBoard> board1_;
    std::shared_ptr<blmc_drivers::CanBusMotorBoard> board2_;
    std::shared_ptr<blmc_drivers::CanBusMotorBoard> board3_;

    double max_current_;
    double max_range_;

    // we have 4 board with each possessing 2 motors and 2 sliders
    SafeMotor_ptr board0_motor0_;
    SafeMotor_ptr board0_motor1_;
    Slider_ptr board0_slider0_;
    Slider_ptr board0_slider1_;

    SafeMotor_ptr board1_motor0_;
    SafeMotor_ptr board1_motor1_;
    Slider_ptr board1_slider0_;
    Slider_ptr board1_slider1_;

    SafeMotor_ptr board2_motor0_;
    SafeMotor_ptr board2_motor1_;
    Slider_ptr board2_slider0_;
    Slider_ptr board2_slider1_;

    SafeMotor_ptr board3_motor0_;
    SafeMotor_ptr board3_motor1_;
    Slider_ptr board3_slider0_;
    Slider_ptr board3_slider1_;
};

}  // namespace blmc_robots

#endif  // TEST_BENCH_8_MOTORS_HPP
