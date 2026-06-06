#include "m9232/m9232.h"
QVector<double> m9232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
