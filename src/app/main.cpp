#include <QApplication>
#include <KAboutData>
#include <KLocalizedString>

int main(int argc, char* argv[])
{
  // Create the application
  QApplication app(argc, argv);

  // Make sure we have translations available
  KLocalizedString::setApplicationDomain("bangarang");

  // Initialize about data
  KAboutData aboutData("editor", i18n("Bangarang"),
                       QStringLiteral(BANGARANG_VERSION),
                       i18n("A media player for your KDE desktop"),
                       KAboutLicense::GPL_V3,
                       i18n("Copyright (C) 2014 The Bangarang Authors")
  );

  aboutData.setOrganizationDomain("kde.org"); // for dbus

  // Add information about authors
  aboutData.addAuthor(i18n("Andrew Lake"), i18n("Creator"), "jamboarder@gmail.com");
  aboutData.addCredit(i18n("Stefan Burnicki"), i18n("Contributor"));
  aboutData.addCredit(i18n("Elias Probst"), i18n("Contributor"));
  aboutData.setBugAddress("https://bugs.kde.org/enter_bug.cgi?product=bangarang&format=guided");
  aboutData.setCustomAuthorText(
	i18n("Defects may be reported at https://bugs.kde.org/enter_bug.cgi?product=bangarang&format=guided"),
	i18n("Defects may be reported at <a href='https://bugs.kde.org/enter_bug.cgi?product=bangarang&format=guided'>KDE Bugtracker</a>"));
  aboutData.setHomepage("https://projects.kde.org/projects/playground/multimedia/bangarang");

  KAboutData::setApplicationData(aboutData);

  // set application stuff from aboutData
  app.setApplicationName(aboutData.componentName());
  app.setApplicationDisplayName(aboutData.displayName());
  app.setOrganizationDomain(aboutData.organizationDomain());
  app.setApplicationVersion(aboutData.version());

  // here would be the place to set up the QCommandLineParser, I think

  // Start the actual program
  qDebug("Hello World! This is Bangarang.");
  // TODO: Create BangarangApplication object and get things done!

  // Wait until application closes and return status.
  return app.exec();
}
