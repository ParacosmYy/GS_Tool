#include "f35225/m35225.h"
QVector<double> m35225::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
