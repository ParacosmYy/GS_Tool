#include "o8114/m8114.h"
QVector<double> m8114::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
