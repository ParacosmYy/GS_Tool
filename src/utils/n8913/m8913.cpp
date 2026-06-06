#include "n8913/m8913.h"
QVector<double> m8913::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
