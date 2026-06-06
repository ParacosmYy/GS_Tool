#include "f28725/m28725.h"
QVector<double> m28725::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
