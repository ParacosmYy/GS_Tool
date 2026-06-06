#include "k11910/m11910.h"
QVector<double> m11910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
