#include "a18420/m18420.h"
QVector<double> m18420::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
