#include "a8460/m8460.h"
QVector<double> m8460::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
