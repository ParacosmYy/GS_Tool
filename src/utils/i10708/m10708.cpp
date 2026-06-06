#include "i10708/m10708.h"
QVector<double> m10708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
