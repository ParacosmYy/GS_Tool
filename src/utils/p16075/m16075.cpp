#include "p16075/m16075.h"
QVector<double> m16075::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
