#include "g18386/m18386.h"
QVector<double> m18386::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
