#include "i10828/m10828.h"
QVector<double> m10828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
