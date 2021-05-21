#pragma once
#include <QtCore>


struct Sphere
{
	QVector3D position;
	float radius;
	QVector3D color;
	int material_idx;
};

struct Camera
{
	QVector3D position;
	QVector3D view;
	QVector3D up;
	QVector3D side;
};