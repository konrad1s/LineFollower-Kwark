#include "qledindicatorplugin.h"
#include "qledindicator.h"

#include <QtPlugin>

QLedIndicatorPlugin::QLedIndicatorPlugin(QObject *parent)
    : QObject(parent), initialized(false)
{
}

void QLedIndicatorPlugin::initialize(QDesignerFormEditorInterface *)
{
    if (initialized)
        return;

    initialized = true;
}

bool QLedIndicatorPlugin::isInitialized() const
{
    return initialized;
}

QWidget *QLedIndicatorPlugin::createWidget(QWidget *parent)
{
    return new QLedIndicator(parent);
}

QString QLedIndicatorPlugin::name() const
{
    return "QLedIndicator";
}

QString QLedIndicatorPlugin::group() const
{
    return "Indicators";
}

QIcon QLedIndicatorPlugin::icon() const
{
    return QIcon();
}

QString QLedIndicatorPlugin::toolTip() const
{
    return "";
}

QString QLedIndicatorPlugin::whatsThis() const
{
    return "";
}

bool QLedIndicatorPlugin::isContainer() const
{
    return false;
}

QString QLedIndicatorPlugin::domXml() const
{
    return R"(
        <widget class="QLedIndicator" name="ledIndicator">
         <property name="geometry">
          <rect>
           <x>0</x>
           <y>0</y>
           <width>100</width>
           <height>100</height>
          </rect>
         </property>
         <property name="toolTip">
          <string>Led indicator/button</string>
         </property>
         <property name="whatsThis">
          <string>Led indicator/button with customizable color and state</string>
         </property>
         <property name="on">
          <bool>false</bool>
         </property>
         <property name="color">
          <color>#00ff00</color>
         </property>
        </widget>
    )";
}

QString QLedIndicatorPlugin::includeFile() const
{
    return "qledindicator.h";
}
