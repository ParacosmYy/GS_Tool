#include "f9065/m9065.h"
QVector<double> m9065::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
