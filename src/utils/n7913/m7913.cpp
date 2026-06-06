#include "n7913/m7913.h"
QVector<double> m7913::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
