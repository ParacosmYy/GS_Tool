#include "n8033/m8033.h"
QVector<double> m8033::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
