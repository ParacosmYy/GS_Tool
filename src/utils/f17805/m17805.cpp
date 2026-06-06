#include "f17805/m17805.h"
QVector<double> m17805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
