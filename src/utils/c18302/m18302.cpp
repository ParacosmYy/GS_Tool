#include "c18302/m18302.h"
QVector<double> m18302::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
