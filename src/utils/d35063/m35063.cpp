#include "d35063/m35063.h"
QVector<double> m35063::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
