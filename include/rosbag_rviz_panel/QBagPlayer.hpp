#pragma once

#include <QObject>
#include <QString>
#include <chrono>
#include <memory>
#include <mutex>
#include <rclcpp/rclcpp.hpp>
#include <rcpputils/shared_library.hpp>
#include <rosbag2_cpp/converter_options.hpp>
#include <rosbag2_cpp/readers/sequential_reader.hpp>
#include <rosbag2_cpp/storage_options.hpp>
#include <rosgraph_msgs/msg/clock.hpp>
#include <string>
#include <thread>

namespace rosbag_rviz_panel {

/**
 * @brief GenericPublisher.
 *
 * This class serves as a flexible rclcpp::Publisher, for
 * when you don't know what type of message you're
 * working with at compilation time.
 *
 */
class GenericPublisher : public rclcpp::PublisherBase
{
  public:
    GenericPublisher(
            rclcpp::node_interfaces::NodeBaseInterface* node_base,
            const rosidl_message_type_support_t&        type_support,
            const std::string&                          topic_name,
            const rclcpp::QoS&                          qos);

    virtual ~GenericPublisher() = default;

    void publish(std::shared_ptr<rmw_serialized_message_t> message);
};

/**
 * @brief QBagPlayer.
 *
 * This custom QOBject opens a rosbag and plays it, forward or
 * backwards, at different speed rates.
 *
 */
class QBagPlayer : public QObject
{
    Q_OBJECT

  public:
    /**
     * @brief Constructor of the QBagPlayer class.
     *
     * @param parent A parent QObject, if there is one.
     */
    explicit QBagPlayer(QObject* parent = nullptr);

    /**
     * @brief Destructor of the QBagPlayer class.
     */
    virtual ~QBagPlayer();

  private:
    /**
     * @brief Main loop to play rosbags, forwards or backwards.
     */
    void run(void);

    /**
     * @brief Converts the size of a rosbag into
     * {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"}.
     *
     * @param size uint64_t with the size of the rosbag.
     *
     */
    void sizeToStr(const uint64_t size);

    /**
     * @brief Resets some variables to restart bag replay.
     */
    void reset(void);

    /**
     * @brief Resets all UI text labels.
     */
    void resetTxt(void);

    /**
     * @brief Calculate the time since epoch to send the player's
     * thread to sleep until that time stamp has been reached.
     *
     * @param msg_time int64_t with the
     *        last proccessed message time stamp.
     *
     * @return std::chrono::_V2::system_clock::time_point
     *         with the time stamp to sleep until.
     */
    std::chrono::_V2::system_clock::time_point realTimeDuration(const int64_t msg_time);

    /**
     * @brief Calculate the time stamp to start playing from
     * the clicked progress bar value.
     *
     * @param progress Int value of the progress bar [0, 100]
     *
     * @return std::chrono::nanoseconds with the calculated
     *         time stamp.
     */
    std::chrono::nanoseconds getProgressTime(const int progress);

    /**
     * @brief Creates a generic ros publisher for when you don't
     * know the message types during compilation.
     *
     * @param topic std::string with the topic's name
     *
     * @param type std::string with the message type
     *
     * @param qos rclcpp::QoS for the publisher
     *
     * @return std::shared_ptr<GenericPublisher> with the
     *         instantiated publisher.
     */
    std::shared_ptr<GenericPublisher> createGenericPublisher(
            const std::string& topic,
            const std::string& type,
            const rclcpp::QoS& qos);

  Q_SIGNALS:
    /**
     * @brief Q_SIGNAL to notify that the bag is completed.
     */
    void sendBagFinished(void);

    /**
     * @brief Q_SIGNAL that sends the total size of the bag.
     *
     * @param size QString that contains the total bag size.
     */
    void sendBagSize(const QString size);

    /**
     * @brief Q_SIGNAL that sends the actual stamp time.
     *
     * @param stamp_label QString that contains the time stamp.
     */
    void sendStampLabel(const QString stamp_label);

    /**
     * @brief Q_SIGNAL that sends the actual stamp in a human-readable
     *        date, in a QString.
     *
     * @param human_date QString that contains the time stamp in a
     *        human-redable format.
     */
    void sendDateLabel(const QString human_date);

    /**
     * @brief Q_SIGNAL that sends the playback speed.
     *
     * @param speed QString that contains the actual playback speed.
     */
    void sendPlayspeedLabel(const QString speed);

    /**
     * @brief Q_SIGNAL that sends the number of seconds the current playhead
     *        location is from the beginning of the bag.
     *
     * @param seconds QString that contains the current playhead location
     *        in seconds.
     */
    void sendSecondsLabel(const QString seconds);

    /**
     * @brief Q_SIGNAL that sends a text to notify the user about something.
     *
     * @param status QString that contains the status message to be shown.
     */
    void sendStatusText(const QString status);

    /**
     * @brief Q_SIGNAL that enables or disables the action buttons from the
     *        parent QWidget.
     *
     * @param enable Bool to enable or disable all action buttons.
     */
    void sendEnableActionButtons(const bool enable);

    /**
     * @brief Q_SIGNAL that sends a value [0, 100] to set the progress bar.
     *
     * @param progress Int value with the calculated progress.
     */
    void sendPlayheadProgress(const int progress);

  public Q_SLOTS:
    /**
     * @brief Q_SLOT to receive the absolute file path of the selected
     *        rosbag to be loaded.
     *
     * @param filename QString that contains the absolute file path of
     *        the rosbag.
     */
    void receiveLoadBag(const QString filename);

    /**
     * @brief Q_SLOT to set the time stamp for the beginning of
     *        the bag.
     *
     * @param start int64_t with the desired time stamp in nanoseconds.
     */
    void receiveSetStart(const int64_t start);

    /**
     * @brief Q_SLOT to set the time stamp for the end of
     *        the bag.
     *
     * @param end int64_t with the desired time stamp in nanoseconds.
     */
    void receiveSetEnd(const int64_t end);

    /**
     * @brief Q_SLOT to change the playback speed.
     *
     * @param change Float with the value to increase or decrease
     *        the playback speed.
     */
    void receiveChangeSpeed(const float change);

    /**
     * @brief Q_SLOT to pause the bag reproduction, if it is being
     *        played during the signal reception.
     */
    void receiveSetPause(void);

    /**
     * @brief Q_SLOT to start playing the loaded rosbag, if it is
     *        not being already played during the signal reception.
     */
    void receiveStartPlaying(void);

    /**
     * @brief Q_SLOT to set the current playhead to the beginning
     *        of the loaded rosbag.
     */
    void receiveGotoBegin(void);

    /**
     * @brief Q_SLOT to set the current playhead to the end
     *        of the loaded rosbag.
     */
    void receiveGotoEnd(void);

    /**
     * @brief Q_SLOT to set the new start ros time samp from the
     * clicked progress bar value
     *
     * @param value Int with the value [0, 100] from the
     *        progress bar.
     */
    void receiveClickedProgress(int value);

  private:
    std::shared_ptr<rclcpp::Node>                                      _nh;
    std::shared_ptr<rcpputils::SharedLibrary>                          _library_generic_publisher;
    std::unique_ptr<rosbag2_cpp::readers::SequentialReader>            _reader;
    std::unordered_map<std::string, std::shared_ptr<GenericPublisher>> _pubs;

    rclcpp::Publisher<rosgraph_msgs::msg::Clock>::SharedPtr _clock_publisher;

    rosbag2_cpp::StorageOptions   _storage_options;
    rosbag2_cpp::ConverterOptions _converter_options;

    std::thread                                        _play_thread;
    std::chrono::time_point<std::chrono::system_clock> _play_start;

    std::chrono::nanoseconds _bag_control_start;
    std::chrono::nanoseconds _bag_control_end;
    std::chrono::nanoseconds _full_bag_start, _full_bag_end;
    std::chrono::nanoseconds _last_message_time;

    double _playback_speed{1.0};
    bool   _pause{false};
    bool   _thread_running{false};
    bool   _playback_direction_changed{false};

    std::mutex _playback_mutex;
    std::mutex _pause_mutex;
    std::mutex _thread_mutex;
};

} // namespace rosbag_rviz_panel