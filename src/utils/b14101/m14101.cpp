#include "b14101/m14101.h"
QVector<double> m14101::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
