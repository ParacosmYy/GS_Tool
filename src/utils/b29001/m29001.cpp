#include "b29001/m29001.h"
QVector<double> m29001::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
