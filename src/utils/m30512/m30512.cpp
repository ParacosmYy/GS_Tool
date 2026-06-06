#include "m30512/m30512.h"
QVector<double> m30512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
