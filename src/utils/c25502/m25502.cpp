#include "c25502/m25502.h"
QVector<double> m25502::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
