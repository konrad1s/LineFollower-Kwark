#ifndef QLEDINDICATORPLUGIN_H
#define QLEDINDICATORPLUGIN_H

#include <QObject>
#include <QDesignerCustomWidgetInterface>
#include <QIcon>
#include <QWidget>

class QLedIndicatorPlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface" FILE "qledindicatorplugin.json")

public:
    QLedIndicatorPlugin(QObject *parent = nullptr);

    bool isContainer() const override;
    bool isInitialized() const override;
    QIcon icon() const override;
    QString domXml() const override;
    QString group() const override;
    QString includeFile() const override;
    QString name() const override;
    QString toolTip() const override;
    QString whatsThis() const override;
    QWidget *createWidget(QWidget *parent) override;
    void initialize(QDesignerFormEditorInterface *core) override;

private:
    bool initialized;
};

#endif // QLEDINDICATORPLUGIN_H
