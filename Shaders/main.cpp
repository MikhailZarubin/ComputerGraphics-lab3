#include <QtWidgets/QApplication>
#include "Shaders.h"

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	ShaderWidget w;
	w.show();
	return a.exec();
}