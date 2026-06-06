#include "d18583/m18583.h"
QVector<double> m18583::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
