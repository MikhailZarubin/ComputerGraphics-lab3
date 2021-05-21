#include "Shaders.h"

ShaderWidget::ShaderWidget(QWidget* parent) :
	QOpenGLWidget(parent)
{
	vertData = new GLfloat[12];
	vertData[0] = -1.0f;	vertData[1] = -1.0f;	vertData[2] = 0.0f;
	vertData[3] = 1.0f;	vertData[4] = -1.0f;	vertData[5] = 0.0f;
	vertData[6] = 1.0f;	vertData[7] = 1.0f;	vertData[8] = 0.0f;
	vertData[9] = -1.0f;	vertData[10] = 1.0f;	vertData[11] = 0.0f;

	sizeSpheres = 3;
	Spheres = new Sphere[3];
	Spheres[0] = { QVector3D(-1.0, -1.0, -2.0), 2, QVector3D(0.0, 1.0, 1.0), 1 };
	//Spheres[0] = { QVector3D(0, 0, 0), 2, QVector3D(1, 1, 1), 1 };
	Spheres[1] = { QVector3D(1.0, 1.0, -1.0), 2.5, QVector3D(1.0, 0.0, 0.0), 1 };
	Spheres[2] = { QVector3D(-3.0, -3.0, -3.0), 1.5, QVector3D(0.0, 1.0, 0.0), 1 };

	cam = { QVector3D(0.0, 0.0, -10), QVector3D(0.0, 0.0, 1.0), QVector3D(0.0, 1.0, 0.0), QVector3D(1.0, 0.0, 0.0) };

	//light_pos = QVector3D(-5, -10, -7);
	light_pos = QVector3D(0.0, 0.0, -10);
}

ShaderWidget::~ShaderWidget()
{
	delete[] Spheres;
	delete[] vertData;
}

void ShaderWidget::initializeGL()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	QOpenGLShader vertShader(QOpenGLShader::Vertex);
	vertShader.compileSourceFile("raytracing.vert");

	QOpenGLShader fragShader(QOpenGLShader::Fragment);
	fragShader.compileSourceFile("raytracing.frag");

	shaderProg.addShader(&vertShader);
	shaderProg.addShader(&fragShader);

	if (!shaderProg.link())
	{
		qWarning("Error with linking programm shader");
		return;
	}

	vert_data_location = shaderProg.attributeLocation("vertex");

	if (!shaderProg.bind())
	{
		qWarning("Error with binding programm shader");
		return;
	}

	shaderProg.setUniformValue("camera.position", cam.position);
	shaderProg.setUniformValue("camera.view", cam.view);
	shaderProg.setUniformValue("camera.up", cam.up);
	shaderProg.setUniformValue("camera.side", cam.side);
	shaderProg.setUniformValue("light_pos", light_pos);
	shaderProg.setUniformValue("scale", QVector2D(width(), height()));

	QOpenGLFunctions_4_3_Core* functions = QOpenGLContext::currentContext()->
		versionFunctions<QOpenGLFunctions_4_3_Core>();
	GLuint ssbo = 0;
	functions->glGenBuffers(1, &ssbo);
	functions->glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	functions->glBufferData(GL_SHADER_STORAGE_BUFFER, sizeSpheres * sizeof(Sphere), Spheres,
		GL_DYNAMIC_DRAW);
	// Now bind the buffer to the zeroth GL_SHADER_STORAGE_BUFFER binding point
	functions->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

	shaderProg.release();

	qDebug() << "Log: " << endl << shaderProg.log() << endl;
}

void ShaderWidget::resizeGL(int nWidth, int nHeight)
{
	glViewport(0, 0, nWidth, nHeight);

	if (!shaderProg.bind())
	{
		qWarning("Error with binding programm shader");
		return;
	}

	shaderProg.setUniformValue("scale", QVector2D(width(), height()));
	shaderProg.release();
}

void ShaderWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT);

	if (!shaderProg.bind())
	{
		qWarning("Error with binding programm shader");
		return;
	}

	shaderProg.enableAttributeArray(vert_data_location);
	shaderProg.setAttributeArray(vert_data_location, vertData, 3);

	glDrawArrays(GL_QUADS, 0, 4);

	shaderProg.disableAttributeArray(vert_data_location);

	shaderProg.release();
}

void ShaderWidget::keyPressEvent(QKeyEvent* event)
{
	if (event->nativeVirtualKey() == Qt::Key_W)
	{
		qDebug() << "Up key";
		if (!shaderProg.bind())
		{
			qWarning("Error with binding programm shader");
			return;
		}

		cam.position += cam.view * 0.5;

		shaderProg.setUniformValue("camera.position", cam.position);
		shaderProg.release();
	}
	else if (event->nativeVirtualKey() == Qt::Key_S)
	{
		qDebug() << "Down key";
		if (!shaderProg.bind())
		{
			qWarning("Error with binding programm shader");
			return;
		}

		//cam.position.setZ(cam.position.z() - 1.0);
		cam.position -= cam.view * 0.5;

		shaderProg.setUniformValue("camera.position", cam.position);
		shaderProg.release();
	}
	else if (event->nativeVirtualKey() == Qt::Key_A)
	{
		qDebug() << "Left key";
		if (!shaderProg.bind())
		{
			qWarning("Error with binding programm shader");
			return;
		}

		cam.position -= cam.side * 0.5;

		shaderProg.setUniformValue("camera.position", cam.position);
		shaderProg.release();
	}
	else if (event->nativeVirtualKey() == Qt::Key_D)
	{
		qDebug() << "Right key";
		if (!shaderProg.bind())
		{
			qWarning("Error with binding programm shader");
			return;
		}

		cam.position += cam.side * 0.5;

		shaderProg.setUniformValue("camera.position", cam.position);
		shaderProg.release();
	}
	else if (event->nativeVirtualKey() == Qt::Key_Z)
	{
		qDebug() << "Right key";
		if (!shaderProg.bind())
		{
			qWarning("Error with binding programm shader");
			return;
		}

		GLfloat angle = 0.1f;
		QMatrix4x4 matr = { cos(angle),   0.0f,    -sin(angle),      0.0f,
					  0.0f,         1.0f,     0.0f,            0.0f ,
					  sin(angle),   0.0f,    cos(angle),       0.0f,
					  0.0f,         0.0f,     0.0f,            1.0f
		};

		/*cam.view.setZ(cam.view.z() + 0.1);
		cam.view.setX(1.0 - cam.view.z());*/
		cam.view = matr * cam.view;
		cam.side = -QVector3D::crossProduct(cam.view, cam.up);

		shaderProg.setUniformValue("camera.view", cam.view);
		shaderProg.setUniformValue("camera.side", cam.side);
		shaderProg.release();
	}
	else if (event->nativeVirtualKey() == Qt::Key_X)
	{
		qDebug() << "Right key";
		if (!shaderProg.bind())
		{
			qWarning("Error with binding programm shader");
			return;
		}

		GLfloat angle = -0.1f;
		QMatrix4x4 matr = { cos(angle),   0.0f,    -sin(angle),      0.0f,
					  0.0f,         1.0f,     0.0f,            0.0f ,
					  sin(angle),   0.0f,    cos(angle),       0.0f,
					  0.0f,         0.0f,     0.0f,            1.0f
		};

		/*cam.view.setZ(cam.view.z() + 0.1);
		cam.view.setX(1.0 - cam.view.z());*/
		cam.view = matr * cam.view;
		cam.side = -QVector3D::crossProduct(cam.view, cam.up);

		shaderProg.setUniformValue("camera.view", cam.view);
		shaderProg.setUniformValue("camera.side", cam.side);
		shaderProg.release();
	}
	else if (event->nativeVirtualKey() == Qt::Key_I)
	{
		qDebug() << "Up key";
		if (!shaderProg.bind())
		{
			qWarning("Error with binding programm shader");
			return;
		}

		light_pos.setZ(light_pos.z() + 1.0);

		shaderProg.setUniformValue("light_pos", light_pos);
		shaderProg.release();
	}
	else if (event->nativeVirtualKey() == Qt::Key_K)
	{
		qDebug() << "Down key";
		if (!shaderProg.bind())
		{
			qWarning("Error with binding programm shader");
			return;
		}

		light_pos.setZ(light_pos.z() - 1.0);

		shaderProg.setUniformValue("light_pos", light_pos);
		shaderProg.release();
	}
	else if (event->nativeVirtualKey() == Qt::Key_J)
	{
		qDebug() << "Left key";
		if (!shaderProg.bind())
		{
			qWarning("Error with binding programm shader");
			return;
		}

		light_pos.setX(light_pos.x() - 1.0);

		shaderProg.setUniformValue("light_pos", light_pos);
		shaderProg.release();
	}
	else if (event->nativeVirtualKey() == Qt::Key_L)
	{
		qDebug() << "Right key";
		if (!shaderProg.bind())
		{
			qWarning("Error with binding programm shader");
			return;
		}

		light_pos.setX(light_pos.x() + 1.0);

		shaderProg.setUniformValue("light_pos", light_pos);
		shaderProg.release();
	}
	else if (event->nativeVirtualKey() == Qt::Key_U)
	{
		qDebug() << "Up key";
		if (!shaderProg.bind())
		{
			qWarning("Error with binding programm shader");
			return;
		}

		light_pos.setY(light_pos.y() + 1.0);

		shaderProg.setUniformValue("light_pos", light_pos);
		shaderProg.release();
	}
	else if (event->nativeVirtualKey() == Qt::Key_H)
	{
		qDebug() << "Down key";
		if (!shaderProg.bind())
		{
			qWarning("Error with binding programm shader");
			return;
		}

		light_pos.setY(light_pos.y() - 1.0);

		shaderProg.setUniformValue("light_pos", light_pos);
		shaderProg.release();
	}
	update();
}

void ShaderWidget::mouseMoveEvent(QMouseEvent* event)
{
	int dx = event->x() - m_lastPos.x();
	int dy = event->y() - m_lastPos.y();

	if (event->buttons() & Qt::LeftButton) {
		if (!shaderProg.bind())
		{
			qWarning("Error with binding programm shader");
			return;
		}

		GLfloat angle = -dx / 64.0f;
		QMatrix4x4 matr = { cos(angle),   0.0f,    -sin(angle),      0.0f,
					  0.0f,         1.0f,     0.0f,            0.0f ,
					  sin(angle),   0.0f,    cos(angle),       0.0f,
					  0.0f,         0.0f,     0.0f,            1.0f
		};

		/*cam.view.setZ(cam.view.z() + 0.1);
		cam.view.setX(1.0 - cam.view.z());*/
		cam.view = matr * cam.view;
		cam.side = -QVector3D::crossProduct(cam.view, cam.up);

		shaderProg.setUniformValue("camera.view", cam.view);
		shaderProg.setUniformValue("camera.side", cam.side);
		shaderProg.release();
		update();

	}
	m_lastPos = event->pos();
}

void ShaderWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton) {
		int dx = event->x() - m_lastPos.x();
		int dy = event->y() - m_lastPos.y();
		m_lastPos = event->pos();

	}
}
