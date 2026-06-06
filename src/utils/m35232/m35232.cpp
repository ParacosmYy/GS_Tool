#include "m35232/m35232.h"
QVector<double> m35232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
