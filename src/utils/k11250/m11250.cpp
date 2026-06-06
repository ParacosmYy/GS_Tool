#include "k11250/m11250.h"
QVector<double> m11250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
