#include "g25706/m25706.h"
QVector<double> m25706::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
