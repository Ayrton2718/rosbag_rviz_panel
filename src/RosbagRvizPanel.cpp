#include "rosbag_rviz_panel/RosbagRvizPanel.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <rclcpp/logger.hpp>
#include <rviz_common/display_context.hpp>

#include "rosbag_rviz_panel/BagPlayerWidget.hpp"

namespace rosbag_rviz_panel {
RosbagRvizPanel::RosbagRvizPanel(QWidget* parent) : rviz_common::Panel(parent)
{
    _widget = std::make_unique<BagPlayerWidget>();

    setupPanelLayout();
}

RosbagRvizPanel::~RosbagRvizPanel() {}

void RosbagRvizPanel::setupPanelLayout()
{
    if (!_widget) {
        RCLCPP_WARN_STREAM(rclcpp::get_logger("global_logger"), "Widget is null!");
        return;
    }

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(_widget.get());

    setLayout(layout);
}

void RosbagRvizPanel::load(const rviz_common::Config& config)
{
    rviz_common::Panel::load(config);

    // PluginContext から ROS2 ノードを取得
    auto node_ptr = this->getDisplayContext()->getRosNodeAbstraction();
    
    if(auto node_lock = node_ptr.lock()->get_raw_node())
    {
        node_lock->declare_parameter("rosbag_panel_bagfile", "");

        std::string file_name;
        if (node_lock->get_parameter("rosbag_panel_bagfile", file_name)) {
            if(file_name != "" && file_name != "None")
            {
                RCLCPP_INFO(node_lock->get_logger(), "Open bag_file: %s", file_name.c_str());

                QFileInfo file_info = QFileInfo(QString::fromStdString(file_name));
                if (_widget->initialize_load_bag(file_info)){
                    _widget->start_playing();
                }
            }
        }
    }
}

void RosbagRvizPanel::save(rviz_common::Config config) const
{
    rviz_common::Panel::save(config);
}
} // namespace rosbag_rviz_panel

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(rosbag_rviz_panel::RosbagRvizPanel, rviz_common::Panel)