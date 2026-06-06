#include "m35532/m35532.h"
QVector<double> m35532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
