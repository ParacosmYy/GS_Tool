#include "g16706/m16706.h"
QVector<double> m16706::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
