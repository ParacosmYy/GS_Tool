#include "f37225/m37225.h"
QVector<double> m37225::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
