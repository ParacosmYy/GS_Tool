#include "k14250/m14250.h"
QVector<double> m14250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
