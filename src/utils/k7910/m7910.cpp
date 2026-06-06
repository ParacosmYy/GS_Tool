#include "k7910/m7910.h"
QVector<double> m7910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
