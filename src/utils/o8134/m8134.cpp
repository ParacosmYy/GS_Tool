#include "o8134/m8134.h"
QVector<double> m8134::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
