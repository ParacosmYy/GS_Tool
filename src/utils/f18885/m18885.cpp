#include "f18885/m18885.h"
QVector<double> m18885::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
