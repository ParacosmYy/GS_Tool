#include "m30052/m30052.h"
QVector<double> m30052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
