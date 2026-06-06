#include "d16063/m16063.h"
QVector<double> m16063::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
