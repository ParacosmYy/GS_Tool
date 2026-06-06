#include "i9768/m9768.h"
QVector<double> m9768::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
