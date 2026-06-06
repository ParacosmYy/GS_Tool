#include "d25923/m25923.h"
QVector<double> m25923::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
