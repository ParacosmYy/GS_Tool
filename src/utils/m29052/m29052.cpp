#include "m29052/m29052.h"
QVector<double> m29052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
