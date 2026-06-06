#include "k16250/m16250.h"
QVector<double> m16250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
