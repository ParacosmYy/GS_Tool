#include "c7822/m7822.h"
QVector<double> m7822::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
