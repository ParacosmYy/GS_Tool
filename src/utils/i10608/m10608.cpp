#include "i10608/m10608.h"
QVector<double> m10608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
