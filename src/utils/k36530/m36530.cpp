#include "k36530/m36530.h"
QVector<double> m36530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
