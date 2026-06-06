#include "h35807/m35807.h"
QVector<double> m35807::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
