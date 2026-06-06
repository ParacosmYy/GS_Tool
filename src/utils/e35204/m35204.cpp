#include "e35204/m35204.h"
QVector<double> m35204::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
