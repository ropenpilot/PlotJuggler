#include <QTextStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include "video_viewer.h"

PublisherVideo::PublisherVideo()
{
  commaLoadVideoFromEnvironment();
  _dialog = new VideoDialog(nullptr);
  connect(_dialog, &VideoDialog::closed, this, [this]()
          {
            setEnabled(false);
            emit closed();
          });
}

PublisherVideo::~PublisherVideo()
{
  delete _dialog;
}

void PublisherVideo::updateState(double current_time)
{
  if(_dialog->isHidden())
  {
    return;
  }
  QString ref_curve = _dialog->referenceCurve();
  if(ref_curve.isEmpty())
  {
    return;
  }
  auto it = _datamap->numeric.find(ref_curve.toStdString());
  if( it == _datamap->numeric.end() )
  {
    return;
  }
  const auto& data = it->second;
  auto position = data.getYfromX(current_time);
  if( position )
  {
    if( !_dialog->isPaused() )
    {
      _dialog->pause(true);
    }
    _dialog->seekByValue(position.value());
  }
}

void PublisherVideo::play(double current_time)
{
  updateState(current_time);
}

bool PublisherVideo::xmlSaveState(QDomDocument &doc, QDomElement &parent_element) const
{
  QDomElement config = doc.createElement("config");
  config.setAttribute("video_file", _dialog->ui->lineFilename->text());
  config.setAttribute("curve_name", _dialog->ui->lineEditReference->text());
  config.setAttribute("use_frame", _dialog->ui->radioButtonFrame->isChecked() ? "true" : "false");

  parent_element.appendChild(config);
  return true;
}

bool PublisherVideo::xmlLoadState(const QDomElement &parent_element)
{
  QDomElement config = parent_element.firstChildElement("config");
  if( config.isNull() )
  {
    return false;
  }
  _dialog->loadFile( config.attribute("video_file") );
  _dialog->ui->lineEditReference->setText( config.attribute("curve_name") );
  if( config.attribute("use_frame") == "true" )
  {
    _dialog->ui->radioButtonFrame->setChecked(true);
  }
  else{
    _dialog->ui->radioButtonTime->setChecked(true);
  }
  _xml_loaded = true;
  return true;
}

void PublisherVideo::setEnabled(bool enabled)
{
  const bool useCommaVideo = commaLoadVideoFromEnvironment();
  QSettings settings;
  auto ui = _dialog->ui;
  if(enabled)
  {
    if( !_xml_loaded  || useCommaVideo)
    {
      QString filename = settings.value("VideoDialog::video_file", "").toString();
      if(filename != ui->lineFilename->text() || useCommaVideo)
      {
        _dialog->loadFile(filename);
      }

      auto curve_name = settings.value("VideoDialog::curve_name", "").toString();
      ui->lineEditReference->setText(curve_name);
      _dialog->restoreGeometry(settings.value("VideoDialog::geometry").toByteArray());
    }
    _dialog->show();
  }
  else
  {
    settings.setValue("VideoDialog::video_file", ui->lineFilename->text());
    settings.setValue("VideoDialog::curve_name", ui->lineEditReference->text());
    settings.setValue("VideoDialog::geometry", _dialog->saveGeometry());
    _dialog->hide();
  }
}

bool PublisherVideo::commaLoadVideoFromEnvironment()
{
  QSettings settings;
  const char * videoPath = std::getenv("VIDEO_PATH");
  const char * referenceCurve = std::getenv("VIDEO_REFERENCE_CURVE");
  QString directory_path =
      settings.value("VideoDialog.loadDirectory", QDir::currentPath()).toString();

  if (!videoPath || !referenceCurve)
  {
    return false;
    //std::printf("shitshit1\n");
    //settings.setValue("VideoDialog::video_file", "");
    //settings.setValue("VideoDialog::curve_name", "");
    //settings.setValue("VideoDialog.video_file", "");
    //settings.setValue("VideoDialog.curve_name", "");
    //settings.setValue("VideoDialog.loadDirectory", directory_path);
    //settings.setValue("VideoDialog::loadDirectory", directory_path);
  }

  QString filename = videoPath;
  QString curve = referenceCurve;

  directory_path = QFileInfo(filename).absolutePath();
  std::printf("shitshityay2\n");
  std::printf("%s\t%s\n", videoPath, referenceCurve);
  settings.setValue("VideoDialog::loadDirectory", directory_path);
  settings.setValue("VideoDialog.loadDirectory", directory_path);
  settings.setValue("VideoDialog::video_file", filename);
  settings.setValue("VideoDialog::curve_name", curve);
  settings.setValue("VideoDialog.video_file", filename);
  settings.setValue("VideoDialog.curve_name", curve);
  return true;
}
