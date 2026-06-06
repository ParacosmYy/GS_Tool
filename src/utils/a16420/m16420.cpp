#include "a16420/m16420.h"
QVector<double> m16420::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
