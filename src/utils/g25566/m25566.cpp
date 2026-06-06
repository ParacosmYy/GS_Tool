#include "g25566/m25566.h"
QVector<double> m25566::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
