#include "p8555/m8555.h"
QVector<double> m8555::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
