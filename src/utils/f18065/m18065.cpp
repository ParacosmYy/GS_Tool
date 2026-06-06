#include "f18065/m18065.h"
QVector<double> m18065::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
