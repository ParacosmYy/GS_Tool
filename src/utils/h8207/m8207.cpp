#include "h8207/m8207.h"
QVector<double> m8207::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
