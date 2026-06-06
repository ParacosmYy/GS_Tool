#include "e28544/m28544.h"
QVector<double> m28544::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
