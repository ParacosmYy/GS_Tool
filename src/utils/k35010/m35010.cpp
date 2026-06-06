#include "k35010/m35010.h"
QVector<double> m35010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
