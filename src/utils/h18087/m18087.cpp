#include "h18087/m18087.h"
QVector<double> m18087::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
