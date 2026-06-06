#include "g18026/m18026.h"
QVector<double> m18026::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
