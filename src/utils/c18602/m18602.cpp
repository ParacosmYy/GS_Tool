#include "c18602/m18602.h"
QVector<double> m18602::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
