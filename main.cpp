#include <QApplication>
#include "CodeStatisticsWidget.h"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	CodeStatisticsWidget widget;
	widget.show();
	return app.exec();
}