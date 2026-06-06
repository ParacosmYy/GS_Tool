#include "q8576/m8576.h"
QVector<double> m8576::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
