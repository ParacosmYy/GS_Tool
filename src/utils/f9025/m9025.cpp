#include "f9025/m9025.h"
QVector<double> m9025::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
