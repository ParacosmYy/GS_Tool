#include "m28992/m28992.h"
QVector<double> m28992::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
