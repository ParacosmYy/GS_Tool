#include "p16815/m16815.h"
QVector<double> m16815::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
