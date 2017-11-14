#include "plotwidget.h"
#include "plot.h"
#include "signalhandler.h"
#include "signaldata.h"
#include "setscaledialog.h"
#include "selectsignaldialog.h"
#include "signaldescription.h"
#include "pythonchannelsubscribercollection.h"

#include <limits>
#include <cmath>
#include <qlabel.h>
#include <qlayout.h>
#include <QDoubleSpinBox>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QTimer>
#include <QPushButton>
#include <QColorDialog>
#include <QDebug>

void PlotWidget::showXaxis()
{
  d_plot->enableAxis(QwtPlot::xBottom, true);
}

PlotWidget::PlotWidget(PythonChannelSubscriberCollection* subscribers, QWidget *parent):
    QWidget(parent),
    mSubscribers(subscribers)
{

  d_plot = new Plot(this);
  d_plot->enableAxis(QwtPlot::xBottom, false);
  mColors << Qt::red
          << Qt::green
          << Qt::blue
          << Qt::cyan
          << Qt::magenta
          << Qt::darkYellow
          << QColor(139, 69, 19) // brown
          << Qt::darkCyan
          << Qt::darkGreen
          << Qt::darkMagenta
          << Qt::black;

  QDoubleSpinBox* yScaleSpin = new QDoubleSpinBox;
  yScaleSpin->setSingleStep(0.1);


  QVBoxLayout* vLayout1 = new QVBoxLayout();


  mSignalListWidget = new QListWidget(this);
  vLayout1->addWidget(mSignalListWidget);

  mSignalInfoLabel = new QLabel(this);
  vLayout1->addWidget(mSignalInfoLabel);
  vLayout1->addStretch(10);


  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->addWidget(d_plot, 10);
  layout->addLayout(vLayout1);

  // connect(yScaleSpin, SIGNAL(valueChanged(double)),
  //        d_plot, SLOT(setYScale(double)));
  // yScaleSpin->setValue(10.0);

  this->setContextMenuPolicy(Qt::CustomContextMenu);
  this->connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
      SLOT(onShowContextMenu(const QPoint&)));

  //mSignalListWidget->setDragDropMode(QAbstractItemView::DragDrop);
  //mSignalListWidget->setDragEnabled(true);
  //mSignalListWidget->setDefaultDropAction(Qt::MoveAction);
  mSignalListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

  mSignalListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  this->connect(mSignalListWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
      SLOT(onShowSignalContextMenu(const QPoint&)));

  this->connect(mSignalListWidget, SIGNAL(itemChanged(QListWidgetItem *)), SLOT(onSignalListItemChanged(QListWidgetItem*)));

  labelUpdateTimer = new QTimer(this);
  this->connect(labelUpdateTimer, SIGNAL(timeout()), SLOT(updateSignalInfoLabel()));

  labelUpdateTimer->start(100);

  rescalingTimer = new QTimer(this);
  this->connect(rescalingTimer, SIGNAL(timeout()), SLOT(resetYAxisMaxScale()));
  this->yRange[0] = std::numeric_limits<double>::max();
  this->yRange[1] = -std::numeric_limits<double>::max();
}

void PlotWidget::onShowSignalValueLabel(bool show)
{
  if (show){
    this->connect(labelUpdateTimer, SIGNAL(timeout()), SLOT(updateSignalValueLabel()));
  } else {
    this->disconnect(labelUpdateTimer,SIGNAL(timeout()),this, SLOT(updateSignalValueLabel()));
    setSignalLabelText();
  }
}

void PlotWidget::onShowContextMenu(const QPoint& pos)
{

  QPoint globalPos = this->mapToGlobal(pos);
  // for QAbstractScrollArea and derived classes you would use:
  // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos); 

  QMenu myMenu;
  myMenu.addAction("Add signal");
  myMenu.addSeparator();
  myMenu.addAction("Reset Y axis scale");
  myMenu.addAction("Set Y axis scale");
  myMenu.addSeparator();
  myMenu.addAction("Remove plot");

  QAction* selectedItem = myMenu.exec(globalPos);
  if (!selectedItem)
  {
    return;
  }

  QString selectedAction = selectedItem->text();

  if (selectedAction == "Remove plot")
  {
    emit this->removePlotRequested(this);
  }
  else if (selectedAction == "Add signal")
  {
    emit this->addSignalRequested(this);
  }
  else if (selectedAction == "Reset Y axis scale")
  {
    this->onResetYAxisScale();
  }
  else if (selectedAction == "Set Y axis scale")
  {

    SetScaleDialog dialog(this);
    QwtInterval axisInterval = d_plot->axisInterval(QwtPlot::yLeft);
    dialog.setUpper(axisInterval.maxValue());
    dialog.setLower(axisInterval.minValue());

    int result = dialog.exec();
    if (result == QDialog::Accepted)
    {
      this->setYAxisScale(dialog.lower(), dialog.upper());
    }
  }
}

QList<SignalHandler*> PlotWidget::signalHandlers()
{
  QList<SignalHandler*> handlers;
  for (int i = 0; i < mSignalListWidget->count(); ++i)
  {
    handlers.append(mSignals[mSignalListWidget->item(i)]);
  }

  return handlers;
}

void PlotWidget::onShowSignalContextMenu(const QPoint& pos)
{

  if (mSignals.empty())
  {
    return;
  }

  QPoint globalPos = mSignalListWidget->mapToGlobal(pos);
  // for QAbstractScrollArea and derived classes you would use:
  // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos); 

  QMenu myMenu;
  if (mSignalListWidget->selectedItems().size() == 1)
    {
    myMenu.addAction("Change color");
    myMenu.addSeparator();
    }

  myMenu.addAction("Remove signal");

  QAction* selectedItem = myMenu.exec(globalPos);
  if (!selectedItem)
  {
    return;
  }

  QString selectedAction = selectedItem->text();

  if (selectedAction == "Change color")
  {
    QListWidgetItem* signalItem = mSignalListWidget->currentItem();
    SignalHandler* signalHandler = signalItem->data(Qt::UserRole).value<SignalHandler*>();
    QColor initialColor = signalHandler->signalDescription()->mColor;
    QColor newColor = QColorDialog::getColor(initialColor, this, "Choose signal color");

    if (newColor.isValid())
    {
      signalHandler->signalDescription()->mColor = newColor;
      QPixmap pixmap(24, 24);
      pixmap.fill(newColor);
      signalItem->setIcon(QIcon(pixmap));
      d_plot->setSignalColor(signalHandler->signalData(), newColor);
    }

  }
  else if (selectedAction == "Remove signal")
  {

    QList<QListWidgetItem*> selectedItems = mSignalListWidget->selectedItems();
    foreach (QListWidgetItem*	selectedItem, selectedItems)
    {
      SignalHandler* signalHandler = this->signalForItem(selectedItem);

      mSubscribers->removeSignalHandler(signalHandler);

      d_plot->removeSignal(signalHandler->signalData());
      mSignals.remove(selectedItem);
      this->replot();

      delete selectedItem;
      delete signalHandler;
    }
  }

}

double PlotWidget::timeWindow() const
{
  return this->d_plot->timeWindow();
}

void PlotWidget::resetYAxisMaxScale()
{
  bool update = false;
  QRectF tmpRange;
  for (int ii=0; ii<mSignalListWidget->count(); ii++){
    QListWidgetItem* signalItem = mSignalListWidget->item(ii);
    SignalHandler* signalHandler = signalItem->data(Qt::UserRole).value<SignalHandler*>();
    if (!this->signalIsVisible(signalHandler)) continue;
    SignalData* signalData = signalHandler->signalData();
    tmpRange = signalData->computeBounds();
    if (tmpRange.topLeft().y() < yRange[0]){
      yRange[0] = tmpRange.topLeft().y();
      update = true;
    }
    if (tmpRange.bottomLeft().y() > yRange[1]){
      yRange[1] = tmpRange.bottomLeft().y();
      update = true;
    }      
  }
  
  if (update){
    onResetYAxisScale();
  }
}

void PlotWidget::setSignalLabelText()
{
  for (int ii=0; ii<mSignalListWidget->count(); ii++){

    QListWidgetItem* signalItem = mSignalListWidget->item(ii);
    SignalHandler* signalHandler = signalItem->data(Qt::UserRole).value<SignalHandler*>();
    if (!signalHandler)
      continue;
    
    SignalData* signalData = signalHandler->signalData();
    QString signalValue = "No data";
    QString signalDescription = QString("%2 [%1]").arg(signalHandler->channel()).arg(signalHandler->description().split(".").back());

    // QString tlabel = signalItem->text().section(" ",1,0);
    signalItem->setText(signalDescription);
  }
}

void PlotWidget::updateSignalValueLabel()
{
  for (int ii=0; ii<mSignalListWidget->count(); ii++){

    QListWidgetItem* signalItem = mSignalListWidget->item(ii);
    SignalHandler* signalHandler = signalItem->data(Qt::UserRole).value<SignalHandler*>();
    SignalData* signalData = signalHandler->signalData();
    QString signalValue = "No data";
    int numberOfValues = signalData->size();
    if (numberOfValues)
    {
      signalValue = QString::number(signalData->value(numberOfValues-1).y(), 'g', 6);
    }

    QString test = signalHandler->signalDescription()->mFieldName;
    test.replace("]","[");
    QString signalDescription = test.section("[",1,1);

    // QString tlabel = signalItem->text().section(" ",1,0);
    signalItem->setText(signalDescription + QString(" ") + signalValue);
  }
}

void PlotWidget::updateSignalInfoLabel()
{
  mSignalInfoLabel->setText(QString());

  QListWidgetItem* selectedItem = mSignalListWidget->currentItem();
  if (!selectedItem)
  {
    return;
  }

  SignalHandler* signalHandler = selectedItem->data(Qt::UserRole).value<SignalHandler*>();
  SignalData* signalData = signalHandler->signalData();


  QString signalValue = "No data";
  int numberOfValues = signalData->size();
  if (numberOfValues)
  {
    signalValue = QString::number(signalData->value(numberOfValues-1).y(), 'g', 6);
  }

  QString signalInfoText = QString("Freq:  %1  Val: %2").arg(QString::number(signalData->messageFrequency(), 'f', 1)).arg(signalValue);

  mSignalInfoLabel->setText(signalInfoText);

}

void PlotWidget::setEndTime(double endTime)
{
  d_plot->setEndTime(endTime);
}

void PlotWidget::setXAxisScale(double x0, double x1)
{
  d_plot->setAxisScale(QwtPlot::xBottom, x0, x1);
}

void PlotWidget::replot()
{
  d_plot->replot();
}

void PlotWidget::onResetYAxisScale()
{
  QRectF area;
  foreach (SignalHandler* signalHandler, mSignals.values())
  {
    if (this->signalIsVisible(signalHandler))
    {
      QRectF signalBounds = signalHandler->signalData()->computeBounds();
      if (!signalBounds.isValid()){
        double margin = std::abs(signalBounds.top()*0.1);
        signalBounds.setTop(signalBounds.top() - margin);
        signalBounds.setBottom(signalBounds.bottom() + margin);
      }

      if (!area.isValid())
      {
        area = signalBounds;
      }
      else
      {
        area = area.united(signalBounds);
      }
    }
  }

  if (!area.isValid())
  {
    area = QRectF(-1, -1, 2, 2);
  }
  else
  {
    double margin = std::abs(area.height()*0.1);
    area.setTop(area.top() - margin);
    area.setBottom(area.bottom() + margin);
  }

  this->setYAxisScale(area.top(), area.bottom());
}

double PlotWidget::getExtent()
{
  return d_plot->getExtent();
}

void PlotWidget::setExtent(double maxExtent)
{
  d_plot->setExtent(maxExtent);
}

void PlotWidget::setYAxisScale(double lower, double upper)
{
  d_plot->setAxisScale(QwtPlot::yLeft, lower, upper);
}

void PlotWidget::onSignalListItemChanged(QListWidgetItem* item)
{
  SignalHandler* signalHandler = this->signalForItem(item);
  Qt::CheckState checkState = item->checkState();

  if (!item->isSelected())
  {
    mSignalListWidget->clearSelection();
    item->setSelected(true);
  }

  QList<QListWidgetItem*> selectedItems = mSignalListWidget->selectedItems();
  foreach (QListWidgetItem*	selectedItem, selectedItems)
  {
    SignalHandler* signalHandler = this->signalForItem(selectedItem);
    selectedItem->setCheckState(checkState);
    d_plot->setSignalVisible(signalHandler->signalData(), checkState == Qt::Checked);
  }
}

QListWidgetItem* PlotWidget::itemForSignal(SignalHandler* signalHandler)
{
  return mSignals.key(signalHandler);
}

SignalHandler* PlotWidget::signalForItem(QListWidgetItem* item)
{
  return mSignals.value(item);
}

bool PlotWidget::signalIsVisible(SignalHandler* signalHandler)
{
  if (!signalHandler)
  {
    return false;
  }

  return (this->itemForSignal(signalHandler)->checkState() == Qt::Checked);
}

void PlotWidget::setSignalVisibility(SignalHandler* signalHandler, bool visible)
{
  QListWidgetItem* item = this->itemForSignal(signalHandler);
  if (item)
  {
    item->setCheckState(visible ? Qt::Checked : Qt::Unchecked);
  }
}

void PlotWidget::start()
{
  d_plot->start();
}

void PlotWidget::stop()
{
  d_plot->stop();
}

void PlotWidget::clearHistory()
{
  foreach (SignalHandler* handler, this->signalHandlers())
  {
    handler->signalData()->clear();
  }
  this->yRange[0] = std::numeric_limits<double>::max();
  this->yRange[1] = -std::numeric_limits<double>::max();
  d_plot->replot();
}

void PlotWidget::setBackgroundColor(QString color)
{
  d_plot->setBackgroundColor(color);
}

void PlotWidget::setPointSize(double pointSize)
{
  d_plot->setPointSize(pointSize);
}

void PlotWidget::setCurveStyle(QwtPlotCurve::CurveStyle style)
{
  d_plot->setCurveStyle(style);
}

void PlotWidget::setAlignMode(QString mode)
{
  if (mode != "center" && mode != "right")
  {
    printf("unsupported align mode: %s\n", qPrintable(mode));
    return;
  }

  d_plot->setAlignMode(mode == "center" ? Plot::CENTER : Plot::RIGHT);
}

void PlotWidget::addSignal(const QMap<QString, QVariant>& signalSettings)
{
  SignalDescription desc;
  desc.mChannel = signalSettings.value("channel").toString();
  desc.mMessageType = signalSettings.value("messageType").toString();
  desc.mFieldName = signalSettings.value("fieldName").toString();
  desc.mArrayKeys = signalSettings.value("arrayKeys").toStringList();

  QList<QVariant> color = signalSettings.value("color").toList();
  if (color.size() == 3)
  {
    desc.mColor = QColor::fromRgb(color[0].toInt(), color[1].toInt(), color[2].toInt());
  }

  bool visible = signalSettings.value("visible", true).toBool();

  SignalHandler* signalHandler = SignalHandlerFactory::instance().createHandler(&desc);

  if (signalHandler)
  {
    //printf("adding signal: %s\n", qPrintable(signalHandler->description()));
  }
  else
  {
    printf("failed to create signal from signal settings.\n");
  }

  this->addSignal(signalHandler);
  this->setSignalVisibility(signalHandler, visible);
}

Q_DECLARE_METATYPE(SignalHandler*);

void PlotWidget::addSignal(SignalHandler* signalHandler)
{
  if (!signalHandler)
  {
    return;
  }

  QColor color = signalHandler->signalDescription()->mColor;
  if (!color.isValid())
  {
    int signalColorIndex = mSignals.size() % mColors.size();
    color = mColors[signalColorIndex];
    signalHandler->signalDescription()->mColor = color;
  }

  QString signalDescription = QString("%2 [%1]").arg(signalHandler->channel()).arg(signalHandler->description().split(".").back());
  QListWidgetItem* signalItem = new QListWidgetItem(signalDescription);
  signalItem->setToolTip(signalDescription);
  mSignalListWidget->addItem(signalItem);
  mSignals[signalItem] = signalHandler;

  mSubscribers->addSignalHandler(signalHandler);

  d_plot->addSignal(signalHandler->signalData(), color);


  QPixmap pixmap(24, 24);
  pixmap.fill(color);
  signalItem->setIcon(QIcon(pixmap));

  signalItem->setData(Qt::UserRole, qVariantFromValue(signalHandler));
  signalItem->setData(Qt::CheckStateRole, Qt::Checked);
}

void PlotWidget::setTimeWindow(double timeWindow)
{
  d_plot->setTimeWindow(timeWindow);
}

void PlotWidget::loadSettings(const QMap<QString, QVariant>& plotSettings)
{
  QList<QVariant> signalList = plotSettings.value("signals").toList();
  foreach (const QVariant& signalVariant, signalList)
  {
    this->addSignal(signalVariant.toMap());
  }

  double timeWindow = plotSettings.value("timeWindow", QVariant(10.0)).toDouble();
  double ymin = plotSettings.value("ymin", QVariant(-10.0)).toDouble();
  double ymax = plotSettings.value("ymax", QVariant(10.0)).toDouble();
  d_plot->setAxisScale(QwtPlot::yLeft, ymin, ymax);
  this->setTimeWindow(timeWindow);

  if (plotSettings.value("curveStyle", "dots") == "lines")
  {
    d_plot->setCurveStyle(QwtPlotCurve::Lines);
  }
}

QMap<QString, QVariant> PlotWidget::saveSettings()
{
  QMap<QString, QVariant> settings;

  settings["ymin"] = d_plot->axisInterval(QwtPlot::yLeft).minValue();
  settings["ymax"] = d_plot->axisInterval(QwtPlot::yLeft).maxValue();
  settings["timeWindow"] = d_plot->timeWindow();

  QList<QVariant> signalSettings;
  foreach (SignalHandler* signalHandler, this->signalHandlers())
  {
    signalSettings.append(this->saveSignalSettings(signalHandler)); 
  }

  settings["signals"] = signalSettings;
  return settings;
}


QMap<QString, QVariant> PlotWidget::saveSignalSettings(SignalHandler* signalHandler)
{
  QMap<QString, QVariant> settings;

  SignalDescription* signalDescription = signalHandler->signalDescription();

  settings["channel"] = signalDescription->mChannel;
  settings["messageType"] = signalDescription->mMessageType;
  settings["fieldName"] = signalDescription->mFieldName;
  settings["arrayKeys"] = QVariant(signalDescription->mArrayKeys);
  settings["visible"] = QVariant(this->itemForSignal(signalHandler)->checkState() == Qt::Checked);

  QList<QVariant> color;
  color << signalDescription->mColor.red() << signalDescription->mColor.green() << signalDescription->mColor.blue();
  settings["color"] = color;

  return settings;
}

