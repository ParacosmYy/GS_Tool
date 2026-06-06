#include "p35255/m35255.h"
QVector<double> m35255::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
