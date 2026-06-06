#include "f37525/m37525.h"
QVector<double> m37525::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
