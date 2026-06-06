#include "f18845/m18845.h"
QVector<double> m18845::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
