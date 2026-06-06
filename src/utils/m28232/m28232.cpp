#include "m28232/m28232.h"
QVector<double> m28232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
