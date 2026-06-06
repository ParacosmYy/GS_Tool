#include "f25685/m25685.h"
QVector<double> m25685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
