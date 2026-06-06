#include "f35805/m35805.h"
QVector<double> m35805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
