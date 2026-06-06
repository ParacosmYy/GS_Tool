#include "e8784/m8784.h"
QVector<double> m8784::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
