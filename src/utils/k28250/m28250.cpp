#include "k28250/m28250.h"
QVector<double> m28250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
