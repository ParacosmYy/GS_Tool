#include "d8103/m8103.h"
QVector<double> m8103::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
