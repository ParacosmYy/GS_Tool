#include "f25845/m25845.h"
QVector<double> m25845::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
