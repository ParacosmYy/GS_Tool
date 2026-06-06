#include "m10052/m10052.h"
QVector<double> m10052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
