#include "f8065/m8065.h"
QVector<double> m8065::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
