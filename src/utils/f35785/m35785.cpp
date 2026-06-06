#include "f35785/m35785.h"
QVector<double> m35785::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
