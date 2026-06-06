#include "i28168/m28168.h"
QVector<double> m28168::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
