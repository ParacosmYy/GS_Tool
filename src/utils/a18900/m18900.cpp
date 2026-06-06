#include "a18900/m18900.h"
QVector<double> m18900::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
