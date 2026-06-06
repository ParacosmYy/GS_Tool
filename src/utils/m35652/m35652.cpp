#include "m35652/m35652.h"
QVector<double> m35652::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
