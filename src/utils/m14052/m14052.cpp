#include "m14052/m14052.h"
QVector<double> m14052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
