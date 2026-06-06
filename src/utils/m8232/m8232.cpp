#include "m8232/m8232.h"
QVector<double> m8232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
