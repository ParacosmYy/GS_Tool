#include "m15052/m15052.h"
QVector<double> m15052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
