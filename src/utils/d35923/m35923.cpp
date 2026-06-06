#include "d35923/m35923.h"
QVector<double> m35923::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
