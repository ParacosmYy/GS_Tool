#include "i10488/m10488.h"
QVector<double> m10488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
