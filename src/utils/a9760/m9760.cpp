#include "a9760/m9760.h"
QVector<double> m9760::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
